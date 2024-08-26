#include "Rasterization_Functions.h"

// Task List: 
// Technical Debt & Bugfixes:
// Week 2 Technical Debt: cube rotating causes changes in xz axis. Moves in small circle while rotating. Needs bugfixing
// Week 2 Technical Debt: Make Vertex Shaders for ApplyRotationMatrix & ApplyTranslationMatrix. Replace calls to MatrixMath in main with calls in Draw functions

// TODO:
// Write CombineColors, ModulateColors, and Saturate functions 
// Implement Camera Movement (rotation around fixed center point both up/down and left/right)
// Lighting - Directional, Ambient, and point light w/ attenuation
// Create star field
// Model Loading, Model Drawing


int main(void)
{
	// Create window
	RS_Initialize("Tyler Clardy - Assignment 3", screenSpaceWidth, screenSpaceHeight);

	// Set up Timer for Animation
	XTime timer;					
	timer.Restart();

	// Create variables for use with animations & timer
	double desiredFPS = 30.0f;
	int count = 0;
	double threshold = 1.0f / desiredFPS;
	double elapsed = 0.0f;
	float cubeRot = 0.0f;

	// Camera Variables (Adjustable)
	float fov = 90;																						// Field of View (In Degrees)
	float aspectRatio = (float)screenSpaceHeight / (float)screenSpaceWidth;								// Aspect Ratio 
	float nearPlane = 0.1f;																				// Near Plane Value - Used for clipping
	float farPlane = 10;																				// Far Plane Value - Used for Perspective

	float camDistanceFromOrigin = 1.0f;																	// Distance Camera Spawns from origin
	float camAngleDegrees = 19.0f;																		// Angle the camera looks down from at spawn

	// Camera Matrix used to find View Matrix
	SV_CameraMatrix = BuildDefaultMatrixIdentity();														// Initialize basic matrix
	Matrix4x4 camRotate = BuildRotationMatrixOnAxisX(ConvertDegreeToRadian(camAngleDegrees));			// Build desired camera rotation matrix
	SV_CameraMatrix = MultiplyMatrix4x4byMatrix4x4(camRotate, SV_CameraMatrix);							// Apply rotation matrix to camera to point slightly down
	Matrix4x4 camTranslation = BuildTranslationMatrix(0.0f, 0.0f, camDistanceFromOrigin);				// Build desired camera distance from origin translation matrix
	SV_CameraMatrix = MultiplyMatrix4x4byMatrix4x4(camTranslation, SV_CameraMatrix);					// Apply translation matrix to camera - moving backwards and up along the rotated Z axis

	// View Matrix
	SV_ViewMatrix = MatrixQuickInverse(SV_CameraMatrix);

	// Perspective Matrix
	SV_PerspectiveMatrix = BuildMatrixProjection(fov, aspectRatio, nearPlane, farPlane);

	// Code to be executed every frame goes here:
	do
	{
		// Clear color buffer at the start of every frame
		UINT clearColor = 0xFF000000;
		ClearBuffer(clearColor);

		// Clear Depth Buffer at the start of every frame
		ClearDepthBuffer(FLT_MAX);

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

			cubeRot = 0.03f * count;

			// clamp count to account for long run times
			if (count == INT16_MAX)
			{
				count = 0;
			}

			// reset timer
			elapsed -= threshold;
		}

		// Draw:
		#pragma region Draw Test Code

		// Uncomment code blocks to test various drawing functions:
		// These object values can be tinkered with in Defines.h

		// 3D Line Test
		/*VertexShader = NULL;
		DrawLine3D(uneditableOriginalLine[0], uneditableOriginalLine[1]);*/

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
		/*VertexShader = NULL;
		DrawLine3D(backLeft, backRight);
		DrawLine3D(backLeft, frontLeft);
		DrawLine3D(backRight, frontRight);
		DrawLine3D(frontLeft, frontRight);*/

		// Test Triangle WireFrame
		/*PixelShader = PS_Red;
		Matrix4x4 testTriangleWorldMatrix = BuildDefaultMatrixIdentity();
		SV_WorldMatrix = testTriangleWorldMatrix;
		DrawTriangleWireframe(testTriangle);*/

		// Test Triangle Fill
		/*PixelShader = PS_Red;
		Matrix4x4 testTriangleWorldMatrix = BuildDefaultMatrixIdentity();
		SV_WorldMatrix = testTriangleWorldMatrix;
		DrawFilledTriangle(testTriangle);*/

		// Test Cube Wireframe - Line Version
		//PixelShader = PS_Green;																	// Set Pixel Shader to green
		//Matrix4x4 testcubeWorldMatrix = BuildDefaultMatrixIdentity();								// Build default World Matrix
		//Matrix4x4 testcubeTranslation = BuildTranslationMatrix(0.0f, 0.1f, 0.0f);					// Build Translation Matrix to move Cube up 
		//testcubeWorldMatrix = MultiplyMatrix4x4byMatrix4x4(testcubeWorldMatrix, testcubeTranslation);		// Apply Translation Matrix
		//Matrix4x4 testcubeRotation = BuildRotationMatrixOnAxisY(cubeRot);							// Build Rotation Matrix to rotate Cube with respect to time
		//testcubeWorldMatrix = MultiplyMatrix4x4byMatrix4x4(testcubeWorldMatrix, testcubeRotation);			// Apply Rotation Matrix
		//SV_WorldMatrix = testcubeWorldMatrix;
		//DrawCubeWireframe();

		// Test Cube Wireframe - Triangle Version
		//PixelShader = PS_Green;																	// Set Pixel Shader to green
		//Matrix4x4 testcubeWorldMatrix = BuildDefaultMatrixIdentity();								// Build default World Matrix
		//Matrix4x4 testcubeTranslation = BuildTranslationMatrix(0.0f, 0.2f, 0.0f);					// Build Translation Matrix to move Cube up 
		//testcubeWorldMatrix = MultiplyMatrix4x4byMatrix4x4(testcubeWorldMatrix, testcubeTranslation);		// Apply Translation Matrix
		//Matrix4x4 testcubeRotation = BuildRotationMatrixOnAxisY(cubeRot);							// Build Rotation Matrix to rotate Cube with respect to time
		//testcubeWorldMatrix = MultiplyMatrix4x4byMatrix4x4(testcubeWorldMatrix, testcubeRotation);			// Apply Rotation Matrix
		//SV_WorldMatrix = testcubeWorldMatrix;
		//DrawEmptyCube();

#pragma endregion
		#pragma region Draw Code

		// Draw Grid
		PixelShader = PS_White;																	// Set Pixel Shader to white
		Matrix4x4 gridWorldMatrix = BuildDefaultMatrixIdentity();								// Build default World Matrix
		SV_WorldMatrix = gridWorldMatrix;														// Make sure SV_WorldMatrix is set to our Grid's World Matrix 
		DrawGrid();																				// Draw Grid

		// Draw Cube
		PixelShader = PS_DragonTexture;																	// Set Pixel Shader to green
		Matrix4x4 cubeWorldMatrix = BuildDefaultMatrixIdentity();								// Build default World Matrix
		Matrix4x4 cubeTranslation = BuildTranslationMatrix(0.0f, 0.2f, 0.0f);					// Build Translation Matrix to move Cube up 
		cubeWorldMatrix = MultiplyMatrix4x4byMatrix4x4(cubeWorldMatrix, cubeTranslation);		// Apply Translation Matrix
		Matrix4x4 cubeRotation = BuildRotationMatrixOnAxisY(cubeRot);							// Build Rotation Matrix to rotate Cube with respect to time
		cubeWorldMatrix = MultiplyMatrix4x4byMatrix4x4(cubeWorldMatrix, cubeRotation);			// Apply Rotation Matrix
		SV_WorldMatrix = cubeWorldMatrix;														// Make sure SV_WorldMatrix is set to our Cubes World Matrix																	
		DrawFilledCube();																		// Draw Cube

#pragma endregion

	} while (RS_Update(screenSpace, screenSpacePixelCount));

	// End program
	RS_Shutdown();

	// end test
	return 0;
}