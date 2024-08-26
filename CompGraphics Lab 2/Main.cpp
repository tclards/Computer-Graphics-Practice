#include "Rasterization_Functions.h"

// Note: Adjustable Variables for screen data, camera data, test object data, and object data exist in Shaders.h and Defines.h

// Todo: 
// Changing screenSpaceWidth and screenSpaceHeight in Defines.h breaks window. Something wrong with AspectRatio implementation? Needs bugfixing.
// count value is unclamped in main, as clamping at 360 degrees still left jarring reset for that frame of the rotating cube. This defies logic, as a 360 degree rotation should be the exact same frame as the 0 degree rotation. Needs bugfixing.

int main(void)
{
	// Create window
	RS_Initialize("Tyler Clardy - Assignment 2", screenSpaceWidth, screenSpaceHeight);

	// Set up Timer for Animation
	XTime timer;					// create timer object
	timer.Restart();

	// Create variables for use with timer
	double desiredFPS = 30.0f;
	int count = 0;
	double threshold = 1.0f / desiredFPS;
	double elapsed = 0.0f;

	// Code to be executed every frame goes here:
	do
	{
		UINT clearColor = 0xFF000000;
		ClearBuffer(clearColor);

		// signal timer
		timer.Signal();

		// Select Shaders, remember you can change these throughout runtime as needed:
		VertexShader = VS_Rasterizer;	// Set to object being manipulated's world matrix
		PixelShader = PS_White;			// Set to color desired
		
		// check elapsed time
		elapsed += timer.Delta();

		// Compare elapsed delta time with threshold
		if (elapsed > threshold)
		{
			// increment count
			count += 1;

			// clamp count to 360 degrees for one full turn
			/*if (count == 360)
			{
				count = 0;
			}*/

			// reset timer
			elapsed -= threshold;
		}

		// Draw:
		#pragma region Draw Test Code

		// Uncomment code blocks to test various drawing functions:
		// These values can be tinkered with in Defines.h

		// 3D Line Test
		//DrawLine3D(uneditableOriginalLine[0], uneditableOriginalLine[1]);

		// 2D Line Test
		/*Pixel testPixel;
		testPixel.color = 0xFFFFFFFF;
		DrawLine2D(200, 250, 300, 250, testPixel);
		Pixel testPixel2;
		testPixel2.color = 0xFF00FF00;
		DrawLine2D(0, 250, 200, 250, testPixel2);
		Pixel testPixel3;
		testPixel3.color = 0xFFFF0000;
		DrawLine2D(300, 250, 500, 250, testPixel3);*/

		// Grid Test (can select upright test or flat test in Defines.h)
		/*DrawLine3D(backLeft, backRight);
		DrawLine3D(backLeft, frontLeft);
		DrawLine3D(backRight, frontRight);
		DrawLine3D(frontLeft, frontRight);*/

#pragma endregion
		#pragma region Draw Code

		// Draw Grid
		PixelShader = PS_White;																	// Set Pixel Shader to white
		Matrix4x4 gridWorldMatrix = BuildDefaultMatrixIdentity();								// Build default World Matrix
		Matrix4x4 gridTransform = BuildTranslationMatrix(0.0f, 0.2f, 0.0f);						// Build Translation Matrix to adjust Grid Position slightly 
		gridWorldMatrix = MultiplyMatrix4x4byMatrix4x4(gridWorldMatrix, gridTransform);			// Apply Translation Matrix
		SV_WorldMatrix = gridWorldMatrix;														// Make sure SV_WorldMatrix is set to our Grid's World Matrix 
		DrawGrid();																				// Draw Grid

		// Draw Cube
		PixelShader = PS_Green;																	// Set Pixel Shader to green
		Matrix4x4 cubeWorldMatrix = BuildDefaultMatrixIdentity();								// Build default World Matrix
		Matrix4x4 cubeTranslation = BuildTranslationMatrix(0.0f, (0.5f / 2) + -0.3f, -0.1f);	// Build Translation Matrix to move Cube up (and slightly over)
		cubeWorldMatrix = MultiplyMatrix4x4byMatrix4x4(cubeWorldMatrix, cubeTranslation);		// Apply Translation Matrix
		Matrix4x4 cubeRotation = BuildRotationMatrixOnAxisY(0.03f * count);						// Build Rotation Matrix to rotate Cube with respect to time
		cubeWorldMatrix = MultiplyMatrix4x4byMatrix4x4(cubeRotation, cubeWorldMatrix);			// Apply Rotation Matrix
		SV_WorldMatrix = cubeWorldMatrix;														// Make sure SV_WorldMatrix is set to our Cubes World Matrix
		DrawCube();																				// Draw Cube

#pragma endregion

	} while (RS_Update(screenSpace, screenSpacePixelCount));

	// End program
	RS_Shutdown();

	// end test
	return 0;
}