#include "RasterSurface.h"
#include "Shaders.h"

#pragma region Function Declarations

void DrawFilledTriangle(Triangle drawMe);
void DrawTriangleSplit(Triangle& splitTriToDraw);
int ClipTriangleIn3DViewSpace(Triangle& clipMe, Triangle& storageTri);
int Clip3DTriangle(Triangle& clipMe);
void DrawTriangleWireframe(Triangle drawMe);
void DrawLine3D(const Vertex& start, const Vertex& end);
void DrawPixel(int x1, int y1, UINT color);
void DrawLine2D(int xStart, int yStart, int xEnd, int yEnd, Pixel pixel);
void ClipLineIn3DViewSpace(Vertex& lineStart, Vertex& lineEnd);
void ClipLineIn2DNDCSpace(Vertex& lineStart, Vertex& lineEnd);
int Clip3DLine(Vertex& lineStart, Vertex& lineEnd);
void ClearBuffer(UINT color); 
void ClearDepthBuffer(float f);
void ClearBufferLocation(int x1, int y1, int targetWidth, int targetHeight, UINT color);
bool isARGBFullyTransparent(UINT color);
void BLIT_SingleTile(const unsigned int toBeCopiedSourceArray[], int toBeCopiedSourceWidth, int toBeCopiedSourceHeight, UINT toBeCopiedTargetRect[], int toBeCopiedTargetWidth, int toBeCopiedTargetHeight, int screenSpaceDrawAtX1, int screenSpaceDrawAtY1);
void BLIT_WholeWindow(const unsigned int toBeCopiedSourceArray[], int toBeCopiedSourceWidth, int toBeCopiedSourceHeight, UINT toBeCopiedTargetRect[], int toBeCopiedTargetWidth, int toBeCopiedTargetHeight, int screenSpaceDrawAtX1, int screenSpaceDrawAtY1);
void DrawStarField(Vertex starField[]);

void DrawGrid();
void DrawCubeWireframe();
void DrawEmptyCube();
void DrawTexturedCube();
void DrawColoredCube();
void CreateStoneHenge();
void DrawStoneHenge();

#pragma endregion

// Draws and fills a Triangle Object
void DrawFilledTriangle(Triangle drawMe)
{	
	// Copy input data and send through shaders
	Vertex T0Temp = drawMe.points[0];
	Vertex T1Temp = drawMe.points[1];
	Vertex T2Temp = drawMe.points[2];

	// Use Vertex Shader to modify incoming copy
	VertexShader = VS_RasterizerWithoutPerspectiveDivide;
	if (VertexShader)
	{
		VertexShader(T0Temp);
		VertexShader(T1Temp);
		VertexShader(T2Temp);
	}

	// Near Plane Clipping
	// Perform Checks for if Line is completely hidden behind near plane
	if (T0Temp.xyzw[2] < 0 && T1Temp.xyzw[2] < 0 && T2Temp.xyzw[2] < 0)
	{
		// If its completely hidden, return and dont draw it
		return;
	}

	//// Back-Face Culling 1
	//Vertex A = VertexSubtraction(T0Temp, T1Temp);
	//Vertex B = VertexSubtraction(T0Temp, T2Temp);
	//Vertex triSurfaceNormal = VertexCrossProduct(A, B);
	//triSurfaceNormal = VertexNormalize(triSurfaceNormal);
	//if (triSurfaceNormal.xyzw[2] > 0)
	//{
	//	return;
	//}

	// Apply Perspective Divide
	VS_PerspectiveDivide(T0Temp);
	VS_PerspectiveDivide(T1Temp);
	VS_PerspectiveDivide(T2Temp);

	//// Back-Face Culling 2
	//Vertex A2 = VertexSubtraction(T0Temp, T1Temp);
	//Vertex B2 = VertexSubtraction(T0Temp, T2Temp);
	//Vertex triSurfaceNormal2 = VertexCrossProduct(A2, B2);
	//triSurfaceNormal2 = VertexNormalize(triSurfaceNormal2);
	//if (triSurfaceNormal2.xyzw[2] > 0)
	//{
	//	return;
	//}

	// Perform checks for if Triangle is completely off screen - break out of function if so, as triangle will not be drawn
	if (T0Temp.xyzw[1] > 1 && T1Temp.xyzw[1] > 1 && T2Temp.xyzw[1] > 1)	// +Y Check
	{
		return;
	}
	else if (T0Temp.xyzw[1] < -1 && T1Temp.xyzw[1] < -1 && T2Temp.xyzw[1] < -1) // -Y Check
	{
		return;
	}
	else if (T0Temp.xyzw[0] > 1 && T1Temp.xyzw[0] > 1 && T2Temp.xyzw[0] > 1)	// +X Check
	{
		return;
	}
	else if (T0Temp.xyzw[0] < -1 && T1Temp.xyzw[0] < -1 && T2Temp.xyzw[0] < -1)	// -X Check
	{
		return;
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

	//// Back-Face Culling 3
	//Vertex A3 = VertexSubtraction(T0Temp, T1Temp);
	//Vertex B3 = VertexSubtraction(T0Temp, T2Temp);
	//Vertex triSurfaceNormal3 = VertexCrossProduct(A3, B3);
	//triSurfaceNormal3 = VertexNormalize(triSurfaceNormal3);
	//if (triSurfaceNormal3.xyzw[2] > 0)
	//{
	//	return;
	//}

	// Calculate Face Normal
	/*Vertex normA = tri.points[0];
	Vertex normB = tri.points[1];
	Vertex normC = tri.points[2];
	Vertex AtoB = VertexSubtraction(normA, normB);
	Vertex AtoC = VertexSubtraction(normA, normC);
	Vertex normalTemp = VertexCrossProduct(AtoB, AtoC);
	Vertex faceNormal = VertexNormalize(normalTemp);
	tri.faceNormal = faceNormal;*/

	// Calculate Baryocentric relevant data
	float alphaRatio = 1.0f / ((Y2 - Y3) * (X1 - X3) + (X3 - X2) * (Y1 - Y3));
	float betaRatio = 1.0f / ((Y2 - Y3) * (X1 - X3) + (X3 - X2) * (Y1 - Y3));

	// Iterate over all pixels within our Triangle (or close to it) in screenspace
	for (int xIterator = xStart; xIterator <= endX; xIterator++)
	{
		for (int yIterator = yStart; yIterator <= endY; yIterator++)
		{
			Vertex currPoint;
			currPoint.xyzw[0] = xIterator;
			currPoint.xyzw[1] = yIterator;

			// Out of Bounds Checking
			if (currPoint.xyzw[0] <= 0 && currPoint.xyzw[0] >= screenSpaceWidth)
			{
				return;
			}
			else if (currPoint.xyzw[1] <= 0 && currPoint.xyzw[1] >= screenSpaceHeight)
			{
				return;
			}

			// Calculate Baryocentric Coordinates for the point in question relative to the given Triangle
			BaryocentricPoint bya = BaryocentricCoords(currPoint, tri, alphaRatio, betaRatio);

			if (PointIsInsideTriangle(bya, tri) == true)
			{
				// Get 1D coordinates we're attempting to draw at
				int index = ConvertCoordinate_2Dto1D(xIterator, yIterator, screenSpaceWidth, screenSpaceHeight);

				// Interpolate Z value for this point inside the Triangle
				float z = berpZ(bya, tri);

				// Interpolate UV values
				float u = berpU(bya, tri);
				float v = berpV(bya, tri);

				// Copy original pixel
				Pixel color;
				color = tri.color;

				// Pass interpolated UV to Copied Pixel
				color.u = u;
				color.v = v;

				// Modify copy - if using a texture pixel shader, it will automatically use the UV passed in
				if (PixelShader)
				{
					PixelShader(color);
				}

				if (index >= 0 && index < 350000)
				{
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
}

// Helper function for Clip3DTriangle()
// Called during ClipTriangleIn3DViewSpace() to draw any triangles split during clipping
void DrawTriangleSplit(Triangle& splitTriToDraw)
{
	// Apply Perspective Divide
	VS_PerspectiveDivide(splitTriToDraw.points[0]);
	VS_PerspectiveDivide(splitTriToDraw.points[1]);
	VS_PerspectiveDivide(splitTriToDraw.points[2]);

	// Perform checks for if Triangles are completely off screen - break out of function if so, as triangles will not be drawn
	if (splitTriToDraw.points[0].xyzw[1] > 1 && splitTriToDraw.points[1].xyzw[1] > 1 && splitTriToDraw.points[2].xyzw[1] > 1)	// +Y Check
	{
		return;
	}
	if (splitTriToDraw.points[0].xyzw[1] < -1 && splitTriToDraw.points[1].xyzw[1] < -1 && splitTriToDraw.points[2].xyzw[1] < -1) // -Y Check
	{
		return;
	}
	if (splitTriToDraw.points[0].xyzw[0] > 1 && splitTriToDraw.points[1].xyzw[0] > 1 && splitTriToDraw.points[2].xyzw[0] > 1)	// +X Check
	{
		return;
	}
	if (splitTriToDraw.points[0].xyzw[0] < -1 && splitTriToDraw.points[1].xyzw[0] < -1 && splitTriToDraw.points[2].xyzw[0] < -1)	// -X Check
	{
		return;
	}

	// Original plotting variables adapted to use new cartesian data
	Screen_XY screenT0 = ConvertCoordinate_3Dto2D(splitTriToDraw.points[0]);
	Screen_XY screenT1 = ConvertCoordinate_3Dto2D(splitTriToDraw.points[1]);
	Screen_XY screenT2 = ConvertCoordinate_3Dto2D(splitTriToDraw.points[2]);

	// Create new Triangle to Draw
	Triangle tri;
	tri.points[0].xyzw[0] = screenT0.convertedVertex.xyzw[0];
	tri.points[0].xyzw[1] = screenT0.convertedVertex.xyzw[1];
	tri.points[0].xyzw[2] = screenT0.convertedVertex.xyzw[2];
	tri.points[0].uv[0] = splitTriToDraw.points[0].uv[0];
	tri.points[0].uv[1] = splitTriToDraw.points[0].uv[1];
	tri.points[1].xyzw[0] = screenT1.convertedVertex.xyzw[0];
	tri.points[1].xyzw[1] = screenT1.convertedVertex.xyzw[1];
	tri.points[1].xyzw[2] = screenT1.convertedVertex.xyzw[2];
	tri.points[1].uv[0] = splitTriToDraw.points[1].uv[0];
	tri.points[1].uv[1] = splitTriToDraw.points[1].uv[1];
	tri.points[2].xyzw[0] = screenT2.convertedVertex.xyzw[0];
	tri.points[2].xyzw[1] = screenT2.convertedVertex.xyzw[1];
	tri.points[2].xyzw[2] = screenT2.convertedVertex.xyzw[2];
	tri.points[2].uv[0] = splitTriToDraw.points[2].uv[0];
	tri.points[2].uv[1] = splitTriToDraw.points[2].uv[1];

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
			BaryocentricPoint bya; /*= BaryocentricCoords(currPoint, tri);*/

			if (PointIsInsideTriangle(bya, tri) == true)
			{
				// Get 1D coordinates we're attempting to draw at
				int index = ConvertCoordinate_2Dto1D(xIterator, yIterator, screenSpaceWidth, screenSpaceHeight);

				// Interpolate Z value for this point inside the Triangle
				float z = berpZ(bya, tri);

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

				if (index >= 0 && index < 350000)
				{
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
}

// Helper Function for Clip3DTriangle()
// Clips a triangle while it is in 3D view space
// BEFORE Perspective Divide
// This function clips based on Z Values
// Returns 1 if line is completely in front of near plane
// Returns 2 if Triangle was clipped
// Returns 3 if triangle was clipped and split into two triangles
// Returns 4 if visible edge interpolation was needed - SKIP THIS COMPLETELY
int ClipTriangleIn3DViewSpace(Triangle& clipMe, Triangle& storageTri)
{
	// Perform Checks for if triangle is completely in front of near plane, if so break out of function as its fine to move ahead
	if (clipMe.points[0].xyzw[2] > 0 && clipMe.points[1].xyzw[2] > 0 && clipMe.points[2].xyzw[2] > 0)
	{
		return 1;
	}

	// Get Vertex data for easy access
	Vertex V0 = clipMe.points[0];
	Vertex V1 = clipMe.points[1];
	Vertex V2 = clipMe.points[2];

	// Storage variables
	float first_deltaZ, first_deltaPZ, second_deltaZ, second_deltaPZ, ratio;
	float newX, newY, newZ, newU, newV;

	// Scnario 1 (Visible Edge):
	// One Vertex is behind near plane
	if (V0.xyzw[2] < 0 && V1.xyzw[2] > 0 && V2.xyzw[2] > 0 || V0.xyzw[2] > 0 && V1.xyzw[2] < 0 && V2.xyzw[2] > 0 || V0.xyzw[2] > 0 && V1.xyzw[2] > 0 && V2.xyzw[2] < 0)
	{
		return 4;
	}
	// Scenario 2 (Hidden Edge):
	// Two Vertices are behind the near plane
	else if (V0.xyzw[2] < 0 && V1.xyzw[2] < 0 || V0.xyzw[2] < 0 && V2.xyzw[2] < 0 || V1.xyzw[2] < 0 && V2.xyzw[2] < 0)
	{
		// Determine which 2 Vertices need clipping
		if (V0.xyzw[2] < 0 && V1.xyzw[2] < 0)			// V0 and V1
		{
			// Clip first Vertex:
			// Calculate Deltas
			first_deltaZ = abs(V1.xyzw[2]);
			first_deltaPZ = V0.xyzw[2] + first_deltaZ;

			// Get ratio
			ratio = first_deltaZ / first_deltaPZ;

			// Lerp Values to find where line intersects near plane
			newX = lerp(V1.xyzw[0], V0.xyzw[0], ratio);
			newY = lerp(V1.xyzw[1], V0.xyzw[1], ratio);
			newZ = lerp(V1.xyzw[2], V0.xyzw[2], ratio);
			newU = lerp(V1.uv[0], V0.uv[0], ratio);
			newV = lerp(V1.uv[1], V0.uv[1], ratio);

			// Correct Vertex with Lerped Values
			clipMe.points[1].xyzw[0] = newX;
			clipMe.points[1].xyzw[1] = newY;
			clipMe.points[1].xyzw[2] = newZ;
			clipMe.points[1].uv[0] = newU;
			clipMe.points[1].uv[1] = newV;

			// Clip second Vertex:
			// Calculate Deltas
			second_deltaZ = abs(V0.xyzw[2]);
			second_deltaPZ = V1.xyzw[2] + second_deltaZ;

			// Get ratio
			ratio = second_deltaZ / second_deltaPZ;

			// Lerp Values to find where line intersects near plane
			newX = lerp(V0.xyzw[0], V1.xyzw[0], ratio);
			newY = lerp(V0.xyzw[1], V1.xyzw[1], ratio);
			newZ = lerp(V0.xyzw[2], V1.xyzw[2], ratio);
			newU = lerp(V0.uv[0], V1.uv[0], ratio);
			newV = lerp(V0.uv[1], V1.uv[1], ratio);

			// Correct Vertex with Lerped Values
			clipMe.points[0].xyzw[0] = newX;
			clipMe.points[0].xyzw[1] = newY;
			clipMe.points[0].xyzw[2] = newZ;
			clipMe.points[0].uv[0] = newU;
			clipMe.points[0].uv[1] = newV;
		}
		else if (V0.xyzw[2] < 0 && V2.xyzw[2] < 0)		// V0 and V2
		{
			// Clip first Vertex:
			// Calculate Deltas
			first_deltaZ = abs(V2.xyzw[2]);
			first_deltaPZ = V0.xyzw[2] + first_deltaZ;

			// Get ratio
			float ratio = first_deltaZ / first_deltaPZ;

			// Lerp Values to find where line intersects near plane
			newX = lerp(V2.xyzw[0], V0.xyzw[0], ratio);
			newY = lerp(V2.xyzw[1], V0.xyzw[1], ratio);
			newZ = lerp(V2.xyzw[2], V0.xyzw[2], ratio);
			newU = lerp(V2.uv[0], V0.uv[0], ratio);
			newV = lerp(V2.uv[1], V0.uv[1], ratio);

			// Correct Vertex with Lerped Values
			clipMe.points[2].xyzw[0] = newX;
			clipMe.points[2].xyzw[1] = newY;
			clipMe.points[2].xyzw[2] = newZ;
			clipMe.points[2].uv[0] = newU;
			clipMe.points[2].uv[1] = newV;

			// Clip second Vertex:
			// Calculate Deltas
			second_deltaZ = abs(V0.xyzw[2]);
			second_deltaPZ = V2.xyzw[2] + second_deltaZ;

			// Get ratio
			ratio = second_deltaZ / second_deltaPZ;

			// Lerp Values to find where line intersects near plane
			newX = lerp(V0.xyzw[0], V2.xyzw[0], ratio);
			newY = lerp(V0.xyzw[1], V2.xyzw[1], ratio);
			newZ = lerp(V0.xyzw[2], V2.xyzw[2], ratio);
			newU = lerp(V0.uv[0], V2.uv[0], ratio);
			newV = lerp(V0.uv[1], V2.uv[1], ratio);

			// Correct Vertex with Lerped Values
			clipMe.points[0].xyzw[0] = newX;
			clipMe.points[0].xyzw[1] = newY;
			clipMe.points[0].xyzw[2] = newZ;
			clipMe.points[0].uv[0] = newU;
			clipMe.points[0].uv[1] = newV;
		}
		else if (V1.xyzw[2] < 0 && V2.xyzw[2] < 0)		// V1 and V2
		{
			// Clip first Vertex:
			// Calculate Deltas
			first_deltaZ = abs(V2.xyzw[2]);
			first_deltaPZ = V1.xyzw[2] + first_deltaZ;

			// Get ratio
			ratio = first_deltaZ / first_deltaPZ;

			// Lerp Values to find where line intersects near plane
			newX = lerp(V2.xyzw[0], V1.xyzw[0], ratio);
			newY = lerp(V2.xyzw[1], V1.xyzw[1], ratio);
			newZ = lerp(V2.xyzw[2], V1.xyzw[2], ratio);
			newU = lerp(V2.uv[0], V1.uv[0], ratio);
			newV = lerp(V2.uv[1], V1.uv[1], ratio);

			// Correct Vertex with Lerped Values
			clipMe.points[2].xyzw[0] = newX;
			clipMe.points[2].xyzw[1] = newY;
			clipMe.points[2].xyzw[2] = newZ;
			clipMe.points[2].uv[0] = newU;
			clipMe.points[2].uv[1] = newV;

			// Clip second Vertex:
			// Calculate Deltas
			second_deltaZ = abs(V1.xyzw[2]);
			second_deltaPZ = V2.xyzw[2] + second_deltaZ;

			// Get ratio
			ratio = second_deltaZ / second_deltaPZ;

			// Lerp Values to find where line intersects near plane
			newX = lerp(V1.xyzw[0], V2.xyzw[0], ratio);
			newY = lerp(V1.xyzw[1], V2.xyzw[1], ratio);
			newZ = lerp(V1.xyzw[2], V2.xyzw[2], ratio);
			newU = lerp(V1.uv[0], V2.uv[0], ratio);
			newV = lerp(V1.uv[1], V2.uv[1], ratio);

			// Correct Vertex with Lerped Values
			clipMe.points[1].xyzw[0] = newX;
			clipMe.points[1].xyzw[1] = newY;
			clipMe.points[1].xyzw[2] = newZ;
			clipMe.points[1].uv[0] = newU;
			clipMe.points[1].uv[1] = newV;
		}
	}
}

// Clips 3D Triangle using the Near Plane
// Returns 1 - Triangle is hidden completely off screen
// Returns 2 - Triangle is hidden completely behind the near plane
// Returns 3 - Triangle was clipped due to being partially visible and not
// Returns 4 - Triangle was completely on screen and required no clipping
// Returns 5 - SKIP DRAWING THIS TRI
int Clip3DTriangle(Triangle& clipMe)
{
	// Perform Checks for if Line is completely hidden behind near plane
	if (clipMe.points[0].xyzw[2] < 0 && clipMe.points[1].xyzw[2] < 0 && clipMe.points[2].xyzw[2] < 0)
	{
		return 2;
	}

	// Clip Triangle in 3D View Space
	Triangle splitTri;
	int clip3D_Result = ClipTriangleIn3DViewSpace(clipMe, splitTri);
	if (clip3D_Result == 1) // no clipping needed
	{
		// Apply Perspective Divide
		VS_PerspectiveDivide(clipMe.points[0]);
		VS_PerspectiveDivide(clipMe.points[1]);
		VS_PerspectiveDivide(clipMe.points[2]);

		return 4;
	}
	if (clip3D_Result == 2)	// if triangle was clipped without splitting
	{
		// Apply Perspective Divide
		VS_PerspectiveDivide(clipMe.points[0]);
		VS_PerspectiveDivide(clipMe.points[1]);
		VS_PerspectiveDivide(clipMe.points[2]);

		// Perform checks for if Triangle is completely off screen - break out of function if so, as triangle will not be drawn
		if (clipMe.points[0].xyzw[1] > 1 && clipMe.points[1].xyzw[1] > 1 && clipMe.points[2].xyzw[1] > 1)	// +Y Check
		{
			return 1;
		}
		if (clipMe.points[0].xyzw[1] < -1 && clipMe.points[1].xyzw[1] < -1 && clipMe.points[2].xyzw[1] < -1) // -Y Check
		{
			return 1;
		}
		if (clipMe.points[0].xyzw[0] > 1 && clipMe.points[1].xyzw[0] > 1 && clipMe.points[2].xyzw[0] > 1)	// +X Check
		{
			return 1;
		}
		if (clipMe.points[0].xyzw[0] < -1 && clipMe.points[1].xyzw[0] < -1 && clipMe.points[2].xyzw[0] < -1)	// -X Check
		{
			return 1;
		}

		return 3;
	}
	//else if (clip3D_Result == 3)
	//{
	//	// split Triangle:
	//	DrawTriangleSplit(splitTri);

	//	// Original (clipped)Triangle:
	//	// Apply Perspective Divide
	//	VS_PerspectiveDivide(clipMe.points[0]);
	//	VS_PerspectiveDivide(clipMe.points[1]);
	//	VS_PerspectiveDivide(clipMe.points[2]);

	//	// Perform checks for if Triangles are completely off screen - break out of function if so, as triangles will not be drawn
	//	if (clipMe.points[0].xyzw[1] > 1 && clipMe.points[1].xyzw[1] > 1 && clipMe.points[2].xyzw[1] > 1)	// +Y Check
	//	{
	//		return 1;
	//	}
	//	if (clipMe.points[0].xyzw[1] < -1 && clipMe.points[1].xyzw[1] < -1 && clipMe.points[2].xyzw[1] < -1) // -Y Check
	//	{
	//		return 1;
	//	}
	//	if (clipMe.points[0].xyzw[0] > 1 && clipMe.points[1].xyzw[0] > 1 && clipMe.points[2].xyzw[0] > 1)	// +X Check
	//	{
	//		return 1;
	//	}
	//	if (clipMe.points[0].xyzw[0] < -1 && clipMe.points[1].xyzw[0] < -1 && clipMe.points[2].xyzw[0] < -1)	// -X Check
	//	{
	//		return 1;
	//	}

	//	return 3;
	//}
	else if (clip3D_Result == 4)
	{
		return 5;
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
	VertexShader = VS_RasterizerWithoutPerspectiveDivide;
	if (VertexShader)
	{
		VertexShader(copyStart);
		VertexShader(copyEnd);
	}

	// Near Plane Clipping
	int clipResult = Clip3DLine(copyStart, copyEnd);
	if (clipResult == 1)
	{
		return;
	}
	else if (clipResult == 2)
	{
		return;
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
	// Make sure Target is within screen space
	if (x1 >= 0 && x1 <= screenSpaceWidth && y1 >= 0 && y1 <= screenSpaceHeight)
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
			if (index <= 350000 && index > 0)
			{
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
			if (index <= 350000 && index > 0)
			{

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
}

// Helper Function for Clip3DLine()
// Clips a line while it is in 3D view space
// BEFORE Perspective Divide
// This function clips based on Z Values
void ClipLineIn3DViewSpace(Vertex& lineStart, Vertex& lineEnd)
{
	// If lineStart is behind near plane and lineEnd is not
	if (lineStart.xyzw[2] < 0 && lineEnd.xyzw[2] > 0)
	{
		// Calculate Deltas
		float deltaZ = abs(lineStart.xyzw[2]);
		float deltaPZ = lineEnd.xyzw[2] + deltaZ;

		// Get ratio
		float ratio = deltaZ / deltaPZ;

		// Lerp Values to find where line intersects near plane
		float newX = lerp(lineStart.xyzw[0], lineEnd.xyzw[0], ratio);
		float newY = lerp(lineStart.xyzw[1], lineEnd.xyzw[1], ratio);
		float newZ = lerp(lineStart.xyzw[2], lineEnd.xyzw[2], ratio);

		// Correct Vertex with Lerped Values
		lineStart.xyzw[0] = newX;
		lineStart.xyzw[1] = newY;
		lineStart.xyzw[2] = newZ;
	}
	// else if lineEnd is behind near plane and lineStart is not
	else if (lineEnd.xyzw[2] < 0 && lineStart.xyzw[2] > 0)
	{
		// Calculate Deltas
		float deltaZ = abs(lineEnd.xyzw[2]);
		float deltaPZ = lineStart.xyzw[2] + deltaZ;

		// Get ratio
		float ratio = deltaZ / deltaPZ;

		// Lerp Values to find where line intersects near plane
		float newX = lerp(lineEnd.xyzw[0], lineStart.xyzw[0], ratio);
		float newY = lerp(lineEnd.xyzw[1], lineStart.xyzw[1], ratio);
		float newZ = lerp(lineEnd.xyzw[2], lineStart.xyzw[2], ratio);

		// Correct Vertex with Lerped Values
		lineEnd.xyzw[0] = newX;
		lineEnd.xyzw[1] = newY;
		lineEnd.xyzw[2] = newZ;
	}
}

// Helper Function for Clip3DLine()
// Clips a line while it is in 2D NDC space
// AFTER Perspective Divide
void ClipLineIn2DNDCSpace(Vertex& lineStart, Vertex& lineEnd)
{
	// Positive X clipping
	// If lineStart is offscreen in the positive X, but lineEnd is not
	if (lineStart.xyzw[0] > 1 && lineEnd.xyzw[0] < 1)
	{
		// Calculate Deltas
		float deltaX = lineStart.xyzw[0] - 1;
		float deltaPX = lineStart.xyzw[0] - abs(lineEnd.xyzw[0]);

		// Get ratio
		float ratio = deltaX / deltaPX;

		// Lerp Values to find where line intersects near plane
		float newX = lerp(lineStart.xyzw[0], lineEnd.xyzw[0], ratio);
		float newY = lerp(lineStart.xyzw[1], lineEnd.xyzw[1], ratio);
		float newZ = lerp(lineStart.xyzw[2], lineEnd.xyzw[2], ratio);

		// Correct Vertex with Lerped Values
		lineStart.xyzw[0] = newX;
		lineStart.xyzw[1] = newY;
		lineStart.xyzw[2] = newZ;
	}
	// If lineEnd is offscreen in the positive X, but lineStart is not
	else if (lineStart.xyzw[0] > 1 && lineEnd.xyzw[0] < 1)
	{
		// Calculate Deltas
		float deltaX = lineEnd.xyzw[0] - 1;
		float deltaPX = lineEnd.xyzw[0] - abs(lineStart.xyzw[0]);

		// Get ratio
		float ratio = deltaX / deltaPX;

		// Lerp Values to find where line intersects near plane
		float newX = lerp(lineEnd.xyzw[0], lineStart.xyzw[0], ratio);
		float newY = lerp(lineEnd.xyzw[1], lineStart.xyzw[1], ratio);
		float newZ = lerp(lineEnd.xyzw[2], lineStart.xyzw[2], ratio);

		// Correct Vertex with Lerped Values
		lineEnd.xyzw[0] = newX;
		lineEnd.xyzw[1] = newY;
		lineEnd.xyzw[2] = newZ;
	}

	// Negative X clipping
	// If lineStart is offscreen in the negative X, but lineEnd is not
	if (lineStart.xyzw[0] < -1 && lineEnd.xyzw[0] > -1)
	{
		// Calculate Deltas
		float deltaX = abs(lineStart.xyzw[0] + 1);
		float deltaPX = abs(lineStart.xyzw[0]) - abs(lineEnd.xyzw[0]);

		// Get ratio
		float ratio = deltaX / deltaPX;

		// Lerp Values to find where line intersects near plane
		float newX = lerp(lineStart.xyzw[0], lineEnd.xyzw[0], ratio);
		float newY = lerp(lineStart.xyzw[1], lineEnd.xyzw[1], ratio);
		float newZ = lerp(lineStart.xyzw[2], lineEnd.xyzw[2], ratio);

		// Correct Vertex with Lerped Values
		lineStart.xyzw[0] = newX;
		lineStart.xyzw[1] = newY;
		lineStart.xyzw[2] = newZ;
	}
	// If lineEnd is offscreen in the negative X, but lineStart is not
	else if (lineStart.xyzw[0] < -1 && lineEnd.xyzw[0] > -1)
	{
		// Calculate Deltas
		float deltaX = abs(lineEnd.xyzw[0] + 1);
		float deltaPX = abs(lineEnd.xyzw[0]) - abs(lineStart.xyzw[0]);

		// Get ratio
		float ratio = deltaX / deltaPX;

		// Lerp Values to find where line intersects near plane
		float newX = lerp(lineEnd.xyzw[0], lineStart.xyzw[0], ratio);
		float newY = lerp(lineEnd.xyzw[1], lineStart.xyzw[1], ratio);
		float newZ = lerp(lineEnd.xyzw[2], lineStart.xyzw[2], ratio);

		// Correct Vertex with Lerped Values
		lineStart.xyzw[0] = newX;
		lineStart.xyzw[1] = newY;
		lineStart.xyzw[2] = newZ;
	}

	// Positive Y clipping
	// If lineStart is offscreen in the positive Y, but lineEnd is not
	if (lineStart.xyzw[1] > 1 && lineEnd.xyzw[1] < 1)
	{
		// Calculate Deltas
		float deltaY = lineStart.xyzw[1] - 1;
		float deltaPY = lineStart.xyzw[1] - abs(lineEnd.xyzw[1]);

		// Get ratio
		float ratio = deltaY / deltaPY;

		// Lerp Values to find where line intersects near plane
		float newX = lerp(lineStart.xyzw[0], lineEnd.xyzw[0], ratio);
		float newY = lerp(lineStart.xyzw[1], lineEnd.xyzw[1], ratio);
		float newZ = lerp(lineStart.xyzw[2], lineEnd.xyzw[2], ratio);

		// Correct Vertex with Lerped Values
		lineStart.xyzw[0] = newX;
		lineStart.xyzw[1] = newY;
		lineStart.xyzw[2] = newZ;
	}
	// If lineEnd is offscreen in the positive Y, but lineStart is not
	else if (lineStart.xyzw[1] > 1 && lineEnd.xyzw[1] < 1)
	{
		// Calculate Deltas
		float deltaY = lineEnd.xyzw[1] - 1;
		float deltaPY = lineEnd.xyzw[1] - abs(lineStart.xyzw[1]);

		// Get ratio
		float ratio = deltaY / deltaPY;

		// Lerp Values to find where line intersects near plane
		float newX = lerp(lineEnd.xyzw[0], lineStart.xyzw[0], ratio);
		float newY = lerp(lineEnd.xyzw[1], lineStart.xyzw[1], ratio);
		float newZ = lerp(lineEnd.xyzw[2], lineStart.xyzw[2], ratio);

		// Correct Vertex with Lerped Values
		lineEnd.xyzw[0] = newX;
		lineEnd.xyzw[1] = newY;
		lineEnd.xyzw[2] = newZ;
	}

	// Negative Y clipping
	// If lineStart is offscreen in the negative Y, but lineEnd is not
	if (lineStart.xyzw[1] < -1 && lineEnd.xyzw[1] > -1)
	{
		// Calculate Deltas
		float deltaY = abs(lineStart.xyzw[1] + 1);
		float deltaPY = abs(lineStart.xyzw[1]) - abs(lineEnd.xyzw[1]);

		// Get ratio
		float ratio = deltaY / deltaPY;

		// Lerp Values to find where line intersects near plane
		float newX = lerp(lineStart.xyzw[0], lineEnd.xyzw[0], ratio);
		float newY = lerp(lineStart.xyzw[1], lineEnd.xyzw[1], ratio);
		float newZ = lerp(lineStart.xyzw[2], lineEnd.xyzw[2], ratio);

		// Correct Vertex with Lerped Values
		lineStart.xyzw[0] = newX;
		lineStart.xyzw[1] = newY;
		lineStart.xyzw[2] = newZ;
	}
	// If lineEnd is offscreen in the negative Y, but lineStart is not
	else if (lineStart.xyzw[1] < -1 && lineEnd.xyzw[1] > -1)
	{
		// Calculate Deltas
		float deltaY = abs(lineEnd.xyzw[1] + 1);
		float deltaPY = abs(lineEnd.xyzw[1]) - abs(lineStart.xyzw[1]);

		// Get ratio
		float ratio = deltaY / deltaPY;

		// Lerp Values to find where line intersects near plane
		float newX = lerp(lineEnd.xyzw[0], lineStart.xyzw[0], ratio);
		float newY = lerp(lineEnd.xyzw[1], lineStart.xyzw[1], ratio);
		float newZ = lerp(lineEnd.xyzw[2], lineStart.xyzw[2], ratio);

		// Correct Vertex with Lerped Values
		lineStart.xyzw[0] = newX;
		lineStart.xyzw[1] = newY;
		lineStart.xyzw[2] = newZ;
	}
}

// Clips a 3D Line using the Near Plane 
// Returns 1 - Line is hidden completely off screen
// Returns 2 - Line is hidden completely behind the near plane
// Returns 3 - Line was clipped due to being partially visible and not
int Clip3DLine(Vertex& lineStart, Vertex& lineEnd)
{
	// Perform Checks for if Line is completely hidden behind near plane
	if (lineStart.xyzw[2] < 0 && lineEnd.xyzw[2] < 0)
	{
		return 2;
	}

	// Clip Line in 3D View Space
	ClipLineIn3DViewSpace(lineStart, lineEnd);

	// Apply Perspective Divide
	VS_PerspectiveDivide(lineStart);
	VS_PerspectiveDivide(lineEnd);

	// Perform checks for if line is completely off screen - break out of function if so, as line will not be drawn
	if (lineStart.xyzw[1] > 1 && lineEnd.xyzw[1] > 1)	// +Y Check
	{
		return 1;
	}
	if (lineStart.xyzw[1] < -1 && lineEnd.xyzw[1] < -1) // -Y Check
	{
		return 1;
	}
	if (lineStart.xyzw[0] > 1 && lineEnd.xyzw[0] > 1)	// +X Check
	{
		return 1;
	}
	if (lineStart.xyzw[1] < -1 && lineEnd.xyzw[1] < -1)	// -X Check
	{
		return 1;
	}

	// Clip Line in 2D NDC Space
	ClipLineIn2DNDCSpace(lineStart, lineEnd);

	return 3;
}

// Clear out everything in the screenSpace buffer, based on color passed in
void ClearBuffer(UINT color)
{
	ClearBufferLocation(0, 0, screenSpaceWidth, screenSpaceHeight, color);
}

// Clears everything in the depth buffer to the value passed in
void ClearDepthBuffer(float f)
{
	for (int i = 0; i < 350000; i++)
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

// Draws a star field
// Takes in an array of Vertices containing location info for the star field
void DrawStarField(Vertex starField[])
{
	PixelShader = PS_White;

	Vertex drawMe[3000];

	// Rasterize Vertices
	for (int i = 0; i < 3000; i++)
	{
		drawMe[i] = starField[i];

		// Convert Local Space to World Space
		Vertex world = MultiplyVectorbyMatrix4x4(SV_WorldMatrix, drawMe[i]);

		// Convert World Space to View Space
		Vertex view = MultiplyVectorbyMatrix4x4(SV_ViewMatrix, world);

		// Convert View Space to View Space prepped for Projection Space
		Vertex perspective = MultiplyVectorbyMatrix4x4(SV_PerspectiveMatrix, view);

		// Account for Perspective Divide, and Convert to Projection Space
		VS_PerspectiveDivide(perspective);

		drawMe[i] = perspective;
	}

	// Draw Vertices
	for (int i = 0; i < 3000; i++)
	{
		// Pixel Shaders
		Pixel copyColor = starField[i].color;
		if (PixelShader)
		{
			PixelShader(copyColor);
		}

		// Convert to 2D
		Screen_XY screenData = ConvertCoordinate_3Dto2D(drawMe[i]);

		// Get XY screen space data
		int x = (int)floor(screenData.convertedVertex.xyzw[0] + 0.5f);
		int y = (int)floor(screenData.convertedVertex.xyzw[1] + 0.5f);
		float z = screenData.convertedVertex.xyzw[2];

		// Get Index into screenSpace for the curr pixel
		int index = ConvertCoordinate_2Dto1D(x, y, screenSpaceWidth, screenSpaceHeight);

		// Check that coordinates are in bounds
		if (x >= 0 && y >= 0 && z >= 0 && index >= 0 && index < 350000)
		{
			// Compare Z value to Depth Buffer's currently stored Z for this pixel
			if (index >= 0 && index < 350000)
			{
				if (z < zBuffer[index])
				{
					// If Z is smaller (closer) than whatever was previously drawn on this pixel:
				// Overwrite old Z value
					zBuffer[index] = z;

					// Draw Pixel
					DrawPixel(x, y, copyColor.color);
				}
			}
		}
	}
}

// Load Mesh Data from StoneHenge.h into Triangle stoneHenge[]
void CreateStoneHenge()
{
	Vertex sh_Vertices[1457];

	// Loop Through all _OBJ_Vert_ and convert to Vertex
	for (int i = 0; i < 1457; i++)
	{
		// Load Values
		sh_Vertices[i].xyzw[0] = StoneHenge_data[i].pos[0] * 0.1f;		// X Value, scaled by 0.1
		sh_Vertices[i].xyzw[1] = StoneHenge_data[i].pos[1] * 0.1f;		// Y Value, scaled by 0.1
		sh_Vertices[i].xyzw[2] = StoneHenge_data[i].pos[2] * 0.1f;		// Z Value, scaled by 0.1
		sh_Vertices[i].xyzw[3] = 1.0f;									// W Value
		sh_Vertices[i].uv[0] = StoneHenge_data[i].uvw[0];				// U Value
		sh_Vertices[i].uv[1] = StoneHenge_data[i].uvw[1];				// V Value
		sh_Vertices[i].nrm[0] = StoneHenge_data[i].nrm[0];				// Normal Data
		sh_Vertices[i].nrm[1] = StoneHenge_data[i].nrm[1];
		sh_Vertices[i].nrm[2] = StoneHenge_data[i].nrm[2];
	}

	// Loop Through all Indices and build Triangles
	int triCount = 0;
	for (int i = 0; i < 2532; i += 3)
	{
		// Get Data from Indices Array
		int v1, v2, v3;
		v1 = StoneHenge_indicies[i];
		v2 = StoneHenge_indicies[i + 1];
		v3 = StoneHenge_indicies[i + 2];

		// Use Data to index into Converted Vertex array and build Triangles
		stoneHenge[triCount].points[0] = sh_Vertices[v1];
		stoneHenge[triCount].points[1] = sh_Vertices[v2];
		stoneHenge[triCount].points[2] = sh_Vertices[v3];

		// Increment Triangle Count
		triCount += 1;
	}
}

// Draws Stone Henge Object
void DrawStoneHenge()
{
	VertexShader = VS_Rasterizer;
	PixelShader = PS_StoneHengeTexture;

	for (int i = 0; i < 844; i += 1)
	{
		DrawFilledTriangle(stoneHenge[i]);
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
	PixelShader = PS_Red;
	DrawTriangleWireframe(cubeBottomTri_01);
	DrawTriangleWireframe(cubeBottomTri_02);

}

// Draws a Textured Cube Object out of triangles
void DrawTexturedCube()
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

// Draws a solidly colored Cube Object out of triangles
void DrawColoredCube()
{
	// Back Face
	PixelShader = PS_Red;						// Set Color data
	DrawFilledTriangle(cubeBackTri_01);			// Draw Triangles
	DrawFilledTriangle(cubeBackTri_02);

	// Front Face
	PixelShader = PS_Green;						// Set Color data
	DrawFilledTriangle(cubeFrontTri_01);		// Draw Triangles
	DrawFilledTriangle(cubeFrontTri_02);

	// Left Face
	PixelShader = PS_Blue;						// Set Color data
	DrawFilledTriangle(cubeLeftTri_01);			// Draw Triangles
	DrawFilledTriangle(cubeLeftTri_02);

	// Right Face
	PixelShader = PS_Yellow;					// Set Color data
	DrawFilledTriangle(cubeRightTri_01);		// Draw Triangles
	DrawFilledTriangle(cubeRightTri_02);

	// Top Face
	PixelShader = PS_Grey;						// Set Color data
	DrawFilledTriangle(cubeTopTri_01);			// Draw Triangles
	DrawFilledTriangle(cubeTopTri_02);

	// Bottom Face
	PixelShader = PS_Purple;					// Set Color data
	DrawFilledTriangle(cubeBottomTri_01);		// Draw Triangles
	DrawFilledTriangle(cubeBottomTri_02);
}

#pragma endregion
