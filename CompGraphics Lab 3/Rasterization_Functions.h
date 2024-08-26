#include "RasterSurface.h"
#include "Shaders.h"

#pragma region Function Declarations

void DrawFilledTriangle(Triangle drawMe);
void DrawTriangleWireframe(Triangle drawMe);
void DrawLine3D(const Vertex& start, const Vertex& end);
void DrawPixel(int x1, int y1, UINT color);
void DrawLine2D(int xStart, int yStart, int xEnd, int yEnd, Pixel pixel);
void ClearBuffer(UINT color); 
void ClearDepthBuffer(float f);
void ClearBufferLocation(int x1, int y1, int targetWidth, int targetHeight, UINT color);
bool isARGBFullyTransparent(UINT color);
void BLIT_SingleTile(const unsigned int toBeCopiedSourceArray[], int toBeCopiedSourceWidth, int toBeCopiedSourceHeight, UINT toBeCopiedTargetRect[], int toBeCopiedTargetWidth, int toBeCopiedTargetHeight, int screenSpaceDrawAtX1, int screenSpaceDrawAtY1);
void BLIT_WholeWindow(const unsigned int toBeCopiedSourceArray[], int toBeCopiedSourceWidth, int toBeCopiedSourceHeight, UINT toBeCopiedTargetRect[], int toBeCopiedTargetWidth, int toBeCopiedTargetHeight, int screenSpaceDrawAtX1, int screenSpaceDrawAtY1);

void DrawGrid();
void DrawCubeWireframe();
void DrawEmptyCube();
void DrawFilledCube();


#pragma endregion

// Draws and fills a Triangle Object
void DrawFilledTriangle(Triangle drawMe)
{
	// Copy input data and send through shaders
	Vertex T0Temp = drawMe.points[0];
	Vertex T1Temp = drawMe.points[1];
	Vertex T2Temp = drawMe.points[2];

	// Use Vertex Shader to modify incoming copy
	if (VertexShader)
	{
		VertexShader(T0Temp);
		VertexShader(T1Temp);
		VertexShader(T2Temp);
	}

	// Original plotting variables adapted to use new cartesian data
	Screen_XY screenT0 = ConvertCoordinate_3Dto2D(T0Temp);
	Screen_XY screenT1 = ConvertCoordinate_3Dto2D(T1Temp);
	Screen_XY screenT2 = ConvertCoordinate_3Dto2D(T2Temp);

	// Create new Triangle to Draw
	Triangle tri;
	tri.points[0].xyzw[0] = screenT0.convertedVertex.xyzw[0];
	tri.points[0].xyzw[1] = screenT0.convertedVertex.xyzw[1];
	tri.points[0].xyzw[2] = screenT0.convertedVertex.xyzw[2];
	tri.points[0].uv[0] = drawMe.points[0].uv[0];
	tri.points[0].uv[1] = drawMe.points[0].uv[1];
	tri.points[1].xyzw[0] = screenT1.convertedVertex.xyzw[0];
	tri.points[1].xyzw[1] = screenT1.convertedVertex.xyzw[1];
	tri.points[1].xyzw[2] = screenT1.convertedVertex.xyzw[2];
	tri.points[1].uv[0] = drawMe.points[1].uv[0];
	tri.points[1].uv[1] = drawMe.points[1].uv[1];
	tri.points[2].xyzw[0] = screenT2.convertedVertex.xyzw[0];
	tri.points[2].xyzw[1] = screenT2.convertedVertex.xyzw[1];
	tri.points[2].xyzw[2] = screenT2.convertedVertex.xyzw[2];
	tri.points[2].uv[0] = drawMe.points[2].uv[0];
	tri.points[2].uv[1] = drawMe.points[2].uv[1];
	tri.color = drawMe.color;

	// Get data to calculate Iterators
	float X1 = tri.points[0].xyzw[0];
	float X2 = tri.points[1].xyzw[0];
	float X3 = tri.points[2].xyzw[0];
	float Y1 = tri.points[0].xyzw[1];
	float Y2 = tri.points[1].xyzw[1];
	float Y3 = tri.points[2].xyzw[1];

	// Create Iterator Values
	float xStart = FindSmallestFloat(X1, X2, X3);
	float yStart = FindSmallestFloat(Y1, Y2, Y3);
	float endX = FindLargestFloat(X1, X2, X3);
	float endY = FindLargestFloat(Y1, Y2, Y3);

	// Iterate over all pixels within our Triangle (or close to it) in screenspace
	for (int xIterator = xStart; xIterator <= endX; xIterator++)
	{
		for (int yIterator = yStart; yIterator <= endY; yIterator++)
		{
			Vertex currPoint;
			currPoint.xyzw[0] = xIterator;
			currPoint.xyzw[1] = yIterator;

			// Calculate Baryocentric Coordinates for the point in question relative to the given Triangle
			BaryocentricPoint bya = BaryocentricCoords(currPoint, tri);

			if (PointIsInsideTriangle(bya, tri) == true)
			{
				// Get 1D coordinates we're attempting to draw at
				int index = ConvertCoordinate_2Dto1D(xIterator, yIterator, screenSpaceWidth, screenSpaceHeight);

				// Interpolate Z value for this point inside the Triangle
				float z = -berpZ(bya, tri);

				// Interpolate UV values
				float u = berpU(bya, tri);
				float v = berpV(bya, tri);

				// Copy original pixel
				Pixel color;
				color.color = tri.color.color;

				// Pass interpolated UV to Copied Pixel
				color.u = u;
				color.v = v;

				// Modify copy - if using a texture pixel shader, it will automatically use the UV passed in
				if (PixelShader)
				{
					PixelShader(color);
				}

				// Compare Z value to Depth Buffer's currently stored Z for this pixel
				if (z < zBuffer[index])
				{
					// If Z is smaller (closer) than whatever was previously drawn on this pixel:
					// Overwrite old Z value
					zBuffer[index] = z;

					// Draw Pixel
					DrawPixel(currPoint.xyzw[0], currPoint.xyzw[1], color.color);
				}
			}
		}
	}
}

// Draws a triangle wireframe in 3D space
void DrawTriangleWireframe(Triangle drawMe)
{
	DrawLine3D(drawMe.points[0], drawMe.points[1]);
	DrawLine3D(drawMe.points[1], drawMe.points[2]);
	DrawLine3D(drawMe.points[2], drawMe.points[0]);
}

// Draws a line in 3D space
void DrawLine3D(const Vertex& start, const Vertex& end)
{
	// Copy input data and send through shaders
	Vertex copyStart = start;
	Vertex copyEnd = end;

	// Use Vertex Shader to modify incoming copy
	if (VertexShader)
	{
		VertexShader(copyStart);
		VertexShader(copyEnd);
	}

	// Original plotting variables adapted to use new cartesian data
	Screen_XY screenStart = ConvertCoordinate_3Dto2D(copyStart);
	Screen_XY screenEnd = ConvertCoordinate_3Dto2D(copyEnd);

	// Copy original color
	Pixel copyColor = start.color;

	// Modify copy
	if (PixelShader)
	{
		PixelShader(copyColor);
	}

	// Get 2D data 
	int xStart = (int)floor(screenStart.convertedVertex.xyzw[0] + 0.5f);
	int yStart = (int)floor(screenStart.convertedVertex.xyzw[1] + 0.5f);
	int xEnd = (int)floor(screenEnd.convertedVertex.xyzw[0] + 0.5f);
	int yEnd = (int)floor(screenEnd.convertedVertex.xyzw[1] + 0.5f);

	DrawLine2D(xStart, yStart, xEnd, yEnd, copyColor);
}

// Draws an individual pixel in 2D space
void DrawPixel(int x1, int y1, UINT color)
{
	// find target coordinate
	int targetCoordinate = ConvertCoordinate_2Dto1D(x1, y1, screenSpaceWidth, screenSpaceHeight);

	// Make sure target pixel is within the bounds of the screen array
	if (targetCoordinate < screenSpacePixelCount)
	{
		// get target coordinate color
		UINT targetColor = screenSpace[targetCoordinate];

		// lerp target color with incoming color
		UINT finalColor = AlphaBlend(targetColor, color);

		// add color to back buffer to be drawn
		screenSpace[targetCoordinate] = finalColor;
	}
}

// Draws a line in 2D space
void DrawLine2D(int xStart, int yStart, int xEnd, int yEnd, Pixel pixel)
{
	// Calculate deltas
	float xDiff = (float)(xEnd - xStart);
	float yDiff = (float)(yEnd - yStart);

	// If xDiff or yDiff are 0, set them very close to 0 instead to avoid divide by zero error
	if (xDiff == 0)
	{
		xDiff = 0.0001f;
	}
	else if (yDiff == 0)
	{
		yDiff = 0.0001f;
	}

	// Check if xDiff or yDiff is larger, and use one of two processes depending on answer
	if (fabs(xDiff) > fabs(yDiff)) 
	{
		float xMin, xMax;

		// set xMin to the lower x value given
		// and xMax to the higher value
		if (xStart < xEnd) 
		{
			xMin = (float)xStart;
			xMax = (float)xEnd;
		}
		else 
		{
			xMin = (float)xEnd;
			xMax = (float)xStart;
		}

		// draw line in terms of y slope
		float slope = yDiff / xDiff;
		for (float x = xMin; x <= xMax; x += 1.0f) 
		{
			float y = (float)yStart + ((x - (float)xStart) * slope);

			float z = lerp(x, y, slope);

			int index = ConvertCoordinate_2Dto1D(x, y, screenSpaceWidth, screenSpaceHeight);

			// Compare Z value to Depth Buffer's currently stored Z for this pixel
			if (z < zBuffer[index])
			{
				// If Z is smaller (closer) than whatever was previously drawn on this pixel:
				// Overwrite old Z value
				zBuffer[index] = z;

				// Draw Pixel
				DrawPixel((int)(floor(x + 0.5f)), (int)(floor(y + 0.5f)), pixel.color);
			}
		}
	}
	else 
	{
		float yMin, yMax;

		// set yMin to the lower y value given
		// and yMax to the higher value
		if (yStart < yEnd) 
		{
			yMin = (float)yStart;
			yMax = (float)yEnd;
		}
		else 
		{
			yMin = (float)yEnd;
			yMax = (float)yStart;
		}

		// draw line in terms of x slope
		float slope = xDiff / yDiff;
		for (float y = yMin; y <= yMax; y += 1.0f) 
		{
			float x = (float)xStart + ((y - (float)yStart) * slope);

			float z = lerp(x, y, slope);

			int index = ConvertCoordinate_2Dto1D(x, y, screenSpaceWidth, screenSpaceHeight);

			// Compare Z value to Depth Buffer's currently stored Z for this pixel
			if (z < zBuffer[index])
			{
				// If Z is smaller (closer) than whatever was previously drawn on this pixel:
				// Overwrite old Z value
				zBuffer[index] = z;

				// Draw Pixel
				DrawPixel((int)(floor(x + 0.5f)), (int)(floor(y + 0.5f)), pixel.color);
			}
		}
	}
}

// Clear out everything in the screenSpace buffer, based on color passed in
void ClearBuffer(UINT color)
{
	ClearBufferLocation(0, 0, screenSpaceWidth, screenSpaceHeight, color);
}

// Clears everything in the depth buffer to the value passed in
void ClearDepthBuffer(float f)
{
	for (int i = 0; i < 250000; i++)
	{
		zBuffer[i] = f;
	}
}

// Clear specific part of the screenSpace buffer, based on color, coordinates, and sizeData passed in
void ClearBufferLocation(int x1, int y1, int targetWidth, int targetHeight, UINT color)
{
	for (int x = 0; x < targetWidth; x++)
	{
		for (int y = 0; y < targetHeight; y++)
		{
			UINT targetCoord = ConvertCoordinate_2Dto1D(x1, y1, targetWidth, targetHeight);

			UINT colorToClearTo = color;

			DrawPixel(x1 + x, y1 + y, colorToClearTo);
		}
	}
}

// Check if unsigned int ARGB value is fully transparent
bool isARGBFullyTransparent(UINT color)
{
	// extract value
	UINT alpha = color & 0xff000000;

	// simplify value
	alpha = (alpha >> 24);

	// check if alpha value is at max
	if (alpha == 0x0000000)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// BLIT an image to the screen from a source pixel array to the screenSpace buffer at a specific location
void BLIT_SingleTile(const unsigned int toBeCopiedSourceArray[], int toBeCopiedSourceWidth, int toBeCopiedSourceHeight, UINT toBeCopiedTargetRect[], int toBeCopiedTargetWidth, int toBeCopiedTargetHeight, int screenSpaceDrawAtX1, int screenSpaceDrawAtY1)
{
	// Iterate over SourceRect
	for (int x = 0; x < toBeCopiedTargetWidth; x++)
	{
		for (int y = 0; y < toBeCopiedTargetHeight; y++)
		{
			// Calculate current coordinates within target rect
			UINT currCoords_1D = ConvertCoordinate_2Dto1D(toBeCopiedTargetRect[0] + x, toBeCopiedTargetRect[1] + y, toBeCopiedSourceWidth, toBeCopiedSourceHeight);

			// Copy target color
			UINT colorToCopyTemp = toBeCopiedSourceArray[currCoords_1D];

			// Convert/Color Correct source color type to destination color type
			UINT colorToCopy = ConvertBGRAtoARGB(colorToCopyTemp);

			// Draw copied color to BackBuffer to be shown on screen
			DrawPixel(screenSpaceDrawAtX1 + x, screenSpaceDrawAtY1 + y, colorToCopy);
		}
	}
}

// BLIT an image from a source pixel array over the entirety of the screenSpace buffer
void BLIT_WholeWindow(const unsigned int toBeCopiedSourceArray[], int toBeCopiedSourceWidth, int toBeCopiedSourceHeight, UINT toBeCopiedTargetRect[], int toBeCopiedTargetWidth, int toBeCopiedTargetHeight, int screenSpaceDrawAtX1, int screenSpaceDrawAtY1)
{
	// Iterate over Destination screen by evenly divisible Tile Width and Tile Height
	for (int x = 0; x <= (screenSpaceWidth / toBeCopiedTargetHeight); x++)
	{
		for (int y = 0; y <= (screenSpaceHeight / toBeCopiedTargetHeight); y++)
		{
			// Draw Tile in correct rect location
			BLIT_SingleTile(toBeCopiedSourceArray, toBeCopiedSourceWidth, toBeCopiedSourceHeight, toBeCopiedTargetRect, toBeCopiedTargetWidth, toBeCopiedTargetHeight, screenSpaceDrawAtX1 + (x * 32), screenSpaceDrawAtY1 + (y * 32));
		}
	}
}

#pragma region Object Drawing Functions

// Draws the Grid Object
void DrawGrid()
{
	DrawLine3D(gridHorizontalLine_0[0], gridHorizontalLine_0[1]);
	DrawLine3D(gridHorizontalLine_1[0], gridHorizontalLine_1[1]);
	DrawLine3D(gridHorizontalLine_2[0], gridHorizontalLine_2[1]);
	DrawLine3D(gridHorizontalLine_3[0], gridHorizontalLine_3[1]);
	DrawLine3D(gridHorizontalLine_4[0], gridHorizontalLine_4[1]);
	DrawLine3D(gridHorizontalLine_5[0], gridHorizontalLine_5[1]);
	DrawLine3D(gridHorizontalLine_6[0], gridHorizontalLine_6[1]);
	DrawLine3D(gridHorizontalLine_7[0], gridHorizontalLine_7[1]);
	DrawLine3D(gridHorizontalLine_8[0], gridHorizontalLine_8[1]);
	DrawLine3D(gridHorizontalLine_9[0], gridHorizontalLine_9[1]);
	DrawLine3D(gridHorizontalLine_10[0], gridHorizontalLine_10[1]);
	DrawLine3D(gridVertical_0[0], gridVertical_0[1]);
	DrawLine3D(gridVertical_1[0], gridVertical_1[1]);
	DrawLine3D(gridVertical_2[0], gridVertical_2[1]);
	DrawLine3D(gridVertical_3[0], gridVertical_3[1]);
	DrawLine3D(gridVertical_4[0], gridVertical_4[1]);
	DrawLine3D(gridVertical_5[0], gridVertical_5[1]);
	DrawLine3D(gridVertical_6[0], gridVertical_6[1]);
	DrawLine3D(gridVertical_7[0], gridVertical_7[1]);
	DrawLine3D(gridVertical_8[0], gridVertical_8[1]);
	DrawLine3D(gridVertical_9[0], gridVertical_9[1]);
	DrawLine3D(gridVertical_10[0], gridVertical_10[1]);
}

// Draws a Cube Object out of Lines
void DrawCubeWireframe()
{
	DrawLine3D(cubeBackSquare_0[0], cubeBackSquare_0[1]);
	DrawLine3D(cubeBackSquare_1[0], cubeBackSquare_1[1]);
	DrawLine3D(cubeBackSquare_2[0], cubeBackSquare_2[1]);
	DrawLine3D(cubeBackSquare_3[0], cubeBackSquare_3[1]);

	DrawLine3D(cubeFrontSquare_0[0], cubeFrontSquare_0[1]);
	DrawLine3D(cubeFrontSquare_1[0], cubeFrontSquare_1[1]);
	DrawLine3D(cubeFrontSquare_2[0], cubeFrontSquare_2[1]);
	DrawLine3D(cubeFrontSquare_3[0], cubeFrontSquare_3[1]);

	DrawLine3D(cubeBottomLeftConnector[0], cubeBottomLeftConnector[1]);
	DrawLine3D(cubeBottomRightConnector[0], cubeBottomRightConnector[1]);
	DrawLine3D(cubeTopLeftConnector[0], cubeTopLeftConnector[1]);
	DrawLine3D(cubeTopRightConnector[0], cubeTopRightConnector[1]);

	// Bottom Cube Debug Lines
	/*PixelShader = PS_Red;
	DrawLine3D(cubeDebugLine01[0], cubeDebugLine01[1]);
	DrawLine3D(cubeDebugLine02[0], cubeDebugLine02[1]);*/
}

// Draws a Cube Object out of triangles
void DrawEmptyCube()
{
	// Back Face
	DrawTriangleWireframe(cubeBackTri_01);
	DrawTriangleWireframe(cubeBackTri_02);

	// Front Face
	DrawTriangleWireframe(cubeFrontTri_01);
	DrawTriangleWireframe(cubeFrontTri_02);

	// Left Face
	DrawTriangleWireframe(cubeLeftTri_01);
	DrawTriangleWireframe(cubeLeftTri_02);

	// Right Face
	DrawTriangleWireframe(cubeRightTri_01);
	DrawTriangleWireframe(cubeRightTri_02);

	// Top Face
	DrawTriangleWireframe(cubeTopTri_01);
	DrawTriangleWireframe(cubeTopTri_02);

	// Bottom Face
	DrawTriangleWireframe(cubeBottomTri_01);
	DrawTriangleWireframe(cubeBottomTri_02);

}

// Draws a filled and colored Cube Object out of triangles
void DrawFilledCube()
{
	// Back Face
	/*PixelShader = PS_Red;	*/					// Set Color data
	
	cubeBackTri_01.points[0].uv[0] = 1.0f;			// Set UV Data for Triangle 1
	cubeBackTri_01.points[0].uv[1] = 0.0f;
	cubeBackTri_01.points[1].uv[0] = 0.0f;
	cubeBackTri_01.points[1].uv[1] = 0.0f;
	cubeBackTri_01.points[2].uv[0] = 1.0f;
	cubeBackTri_01.points[2].uv[1] = 1.0f;
	cubeBackTri_02.points[0].uv[0] = 0.0f;			// Set UV Data for Triangle 2
	cubeBackTri_02.points[0].uv[1] = 0.0f;
	cubeBackTri_02.points[1].uv[0] = 1.0f;
	cubeBackTri_02.points[1].uv[1] = 1.0f;
	cubeBackTri_02.points[2].uv[0] = 0.0f;
	cubeBackTri_02.points[2].uv[1] = 1.0f;

	DrawFilledTriangle(cubeBackTri_01);			// Draw Triangles
	DrawFilledTriangle(cubeBackTri_02);

	// Front Face
	/*PixelShader = PS_Green;*/						// Set Color data

	cubeFrontTri_01.points[0].uv[0] = 0.0f;		// Set UV Data for Triangle 1
	cubeFrontTri_01.points[0].uv[1] = 0.0f;
	cubeFrontTri_01.points[1].uv[0] = 1.0f;
	cubeFrontTri_01.points[1].uv[1] = 0.0f;
	cubeFrontTri_01.points[2].uv[0] = 0.0f;
	cubeFrontTri_01.points[2].uv[1] = 1.0f;
	cubeFrontTri_02.points[0].uv[0] = 1.0f;		// Set UV Data for Triangle 2
	cubeFrontTri_02.points[0].uv[1] = 0.0f;
	cubeFrontTri_02.points[1].uv[0] = 0.0f;
	cubeFrontTri_02.points[1].uv[1] = 1.0f;
	cubeFrontTri_02.points[2].uv[0] = 1.0f;
	cubeFrontTri_02.points[2].uv[1] = 1.0f;

	DrawFilledTriangle(cubeFrontTri_01);		// Draw Triangles
	DrawFilledTriangle(cubeFrontTri_02);

	// Left Face
	/*PixelShader = PS_Blue;*/						// Set Color data

	cubeLeftTri_01.points[0].uv[0] = 1.0f;			// Set UV Data for Triangle 1
	cubeLeftTri_01.points[0].uv[1] = 0.0f;
	cubeLeftTri_01.points[1].uv[0] = 1.0f;
	cubeLeftTri_01.points[1].uv[1] = 1.0f;
	cubeLeftTri_01.points[2].uv[0] = 0.0f;
	cubeLeftTri_01.points[2].uv[1] = 1.0f;
	cubeLeftTri_02.points[0].uv[0] = 0.0f;			// Set UV Data for Triangle 2
	cubeLeftTri_02.points[0].uv[1] = 1.0f;
	cubeLeftTri_02.points[1].uv[0] = 0.0f;
	cubeLeftTri_02.points[1].uv[1] = 0.0f;
	cubeLeftTri_02.points[2].uv[0] = 1.0f;
	cubeLeftTri_02.points[2].uv[1] = 0.0f;

	DrawFilledTriangle(cubeLeftTri_01);			// Draw Triangles
	DrawFilledTriangle(cubeLeftTri_02);

	// Right Face
	/*PixelShader = PS_Yellow;*/					// Set Color data

	cubeRightTri_01.points[0].uv[0] = 0.0f;		// Set UV Data for Triangle 1
	cubeRightTri_01.points[0].uv[1] = 0.0f;
	cubeRightTri_01.points[1].uv[0] = 0.0f;
	cubeRightTri_01.points[1].uv[1] = 1.0f;
	cubeRightTri_01.points[2].uv[0] = 1.0f;
	cubeRightTri_01.points[2].uv[1] = 1.0f;
	cubeRightTri_02.points[0].uv[0] = 1.0f;		// Set UV Data for Triangle 2
	cubeRightTri_02.points[0].uv[1] = 1.0f;
	cubeRightTri_02.points[1].uv[0] = 1.0f;
	cubeRightTri_02.points[1].uv[1] = 0.0f;
	cubeRightTri_02.points[2].uv[0] = 0.0f;
	cubeRightTri_02.points[2].uv[1] = 0.0f;

	DrawFilledTriangle(cubeRightTri_01);		// Draw Triangles
	DrawFilledTriangle(cubeRightTri_02);

	// Top Face
	/*PixelShader = PS_Grey;*/						// Set Color data

	cubeTopTri_01.points[0].uv[0] = 0.0f;			// Set UV Data for Triangle 1
	cubeTopTri_01.points[0].uv[1] = 1.0f;
	cubeTopTri_01.points[1].uv[0] = 0.0f;
	cubeTopTri_01.points[1].uv[1] = 0.0f;
	cubeTopTri_01.points[2].uv[0] = 1.0f;
	cubeTopTri_01.points[2].uv[1] = 0.0f;
	cubeTopTri_02.points[0].uv[0] = 0.0f;			// Set UV Data for Triangle 2
	cubeTopTri_02.points[0].uv[1] = 1.0f;
	cubeTopTri_02.points[1].uv[0] = 1.0f;
	cubeTopTri_02.points[1].uv[1] = 1.0f;
	cubeTopTri_02.points[2].uv[0] = 1.0f;
	cubeTopTri_02.points[2].uv[1] = 0.0f;

	DrawFilledTriangle(cubeTopTri_01);			// Draw Triangles
	DrawFilledTriangle(cubeTopTri_02);

	// Bottom Face
	/*PixelShader = PS_Purple;*/					// Set Color data

	cubeBottomTri_01.points[0].uv[0] = 1.0f;			// Set UV Data for Triangle 1
	cubeBottomTri_01.points[0].uv[1] = 0.0f;
	cubeBottomTri_01.points[1].uv[0] = 1.0f;
	cubeBottomTri_01.points[1].uv[1] = 1.0f;
	cubeBottomTri_01.points[2].uv[0] = 0.0f;
	cubeBottomTri_01.points[2].uv[1] = 0.0f;
	cubeBottomTri_02.points[0].uv[0] = 0.0f;			// Set UV Data for Triangle 2
	cubeBottomTri_02.points[0].uv[1] = 1.0f;
	cubeBottomTri_02.points[1].uv[0] = 0.0f;
	cubeBottomTri_02.points[1].uv[1] = 0.0f;
	cubeBottomTri_02.points[2].uv[0] = 1.0f;
	cubeBottomTri_02.points[2].uv[1] = 0.0f;

	DrawFilledTriangle(cubeBottomTri_01);		// Draw Triangles
	DrawFilledTriangle(cubeBottomTri_02);
}

#pragma endregion
