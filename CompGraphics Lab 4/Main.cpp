#include "Rasterization_Functions.h"

// Task List: 
// Technical Debt & Bugfixes:
// Week 2 Technical Debt: cube rotating causes changes in xz axis. Moves in small circle while rotating. Needs bugfixing
// Week 2 Technical Debt: Make Vertex Shaders for ApplyRotationMatrix & ApplyTranslationMatrix. Replace calls to MatrixMath in main with calls in Draw functions
// Week 3 Technical Debt: Fix texture perspective problem
// Week 4 Technical Debt: Triangle Near Plane Clipping
// Week 4 Technical Debt: Bugfix Triangle Back Face Culling
// Week 4 Technical Debt: Some Triangles on ground don't load until camera is moved a little. Is fixed with a hardcoded "camera wiggle" in the first 2 frames of the draw loop.
// Week 4 Technical Debt: If camera moves and puts any kind of filled or textured Triangle off screen, program locks up (Doesnt crash). Needs bugfixing.
// Week 4 Technical Debt: Modulate Colors not working properly, so all implemented lighting (directional and ambient) has been commented out in vertex shaders & pixel shaders
// Week 4 Technical Debt: Point Light and Light Attenuation not implemented at all

// Controls: Left/Right & Up/Down Arrow Keys to look around. Left Shift/Right Shift to zoom in and out. Enter to load/unload Stone Henge model.
// Note: To look around star field, press enter to disable henge. Otherwise program locks up when henge goes off screen

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

	float camDistanceFromOrigin = 10.0f;																	// Distance Camera Spawns from origin
	float camAngleDegrees = -90.0f;																		// Angle the camera looks down from at spawn
	
	// Camera Movement Variables
	bool canZoomIn = true;
	bool canZoomOut = true;
	float CamUserAppliedTranslate = 0.0f;
	int zoomInLevel = 0;
	float camAngleRotateX = 0.0f;
	float camAngleRotateY = 0.0f;

	// Bool for loading stonehenge model
	bool loadHenge = true;
	bool firstLoop = true;
	bool secondLoop = false;

	// Camera Matrix used to find View Matrix
	SV_CameraMatrix = BuildDefaultMatrixIdentity();		
	Matrix4x4 camRotate = BuildRotationMatrixOnAxisX(ConvertDegreeToRadian(camAngleDegrees));			// Build desired camera rotation matrix
	Matrix4x4 camRotateZ = BuildRotationMatrixOnAxisZ(ConvertDegreeToRadian(35.0f));
	SV_CameraMatrix = MultiplyMatrix4x4byMatrix4x4(camRotateZ, SV_CameraMatrix);
	Matrix4x4 camTranslation = BuildTranslationMatrix(0.0f, -5.0f, -3.0f);								// Build desired camera distance from origin translation matrix
	SV_CameraMatrix = MultiplyMatrix4x4byMatrix4x4(camTranslation, SV_CameraMatrix);					// Apply translation matrix to camera - moving backwards and up along the rotated Z axis
	SV_CameraMatrix = MultiplyMatrix4x4byMatrix4x4(camRotate, SV_CameraMatrix);							// Apply rotation matrix to camera to point slightly down

	// View Matrix
	SV_ViewMatrix = MatrixQuickInverse(SV_CameraMatrix);

	// Perspective Matrix
	SV_PerspectiveMatrix = BuildMatrixProjection(fov, aspectRatio, nearPlane, farPlane);

	// Set Up Random Number Generator star field
	std::random_device rd;									// obtain random number from hardware
	std::mt19937 gen(rd());									// seed random number generator
	std::uniform_real_distribution<float> distr(-1, 1);		// define XYZ range
	
	// Generate Random star Locations
	Vertex starField[3000];
	for (int i = 0; i < 3000; i++)
	{
		// Generate Random X Value Scaled by 50 then normalized
		starField[i].xyzw[0] = (distr(gen) * 50.0f);

		// Generate Random Y Value Scaled by 50
		starField[i].xyzw[1] = (distr(gen) * 50.0f);

		// Generate Random Z Value Scaled by 50
		starField[i].xyzw[2] = (distr(gen) * 50.0f);

		// Set W Value
		starField[i].xyzw[3] = 1.0f;

		// Set Star Color
		starField[i].color.color = 0xFFFFFFFF;
	}

	// Load Model
	CreateStoneHenge();

	// Code to be executed every frame goes here:
	do
	{
		// hardcoded caveman fix for unrendered ground issue - uses first 2 drawing frames to hide problem by 'wiggling' camera
		float fixValue = 0.5f;
		if (secondLoop == true)	// camera reset
		{
			Matrix4x4 camFixOnSecondLoop1 = BuildRotationMatrixOnAxisY(-fixValue);
			Matrix4x4 camFixOnSecondLoop = BuildRotationMatrixOnAxisX(-fixValue);
			SV_CameraMatrix = MultiplyMatrix4x4byMatrix4x4(camFixOnSecondLoop, SV_CameraMatrix);
			SV_CameraMatrix = MultiplyMatrix4x4byMatrix4x4(camFixOnSecondLoop1, SV_CameraMatrix);
			secondLoop = false;
		}
		if (firstLoop == true)	// 'camera wiggle'
		{
			Matrix4x4 camFixOnFirstLoop1 = BuildRotationMatrixOnAxisY(fixValue);
			Matrix4x4 camFixOnFirstLoop = BuildRotationMatrixOnAxisX(fixValue);
			SV_CameraMatrix = MultiplyMatrix4x4byMatrix4x4(camFixOnFirstLoop, SV_CameraMatrix);
			SV_CameraMatrix = MultiplyMatrix4x4byMatrix4x4(camFixOnFirstLoop1, SV_CameraMatrix);
			firstLoop = false;
			secondLoop = true;
		}

		// signal timer
		timer.Signal();

		// Clear color buffer at the start of every frame
		UINT clearColor = 0xFF000000;
		ClearBuffer(clearColor);

		// Clear Depth Buffer at the start of every frame
		ClearDepthBuffer(FLT_MAX);

		// User Input - Camera Control
		#pragma region User Input

		// check elapsed time
		elapsed += timer.Delta();

		if (GetAsyncKeyState(VK_LEFT))		// Rotate Cam Left on the Y Axis
		{
			// Rotate Left
			camAngleRotateY += 0.01f * elapsed;
		}
		if (GetAsyncKeyState(VK_RIGHT))		// Roate Cam Right on the Y Axis
		{
			// Rotate Right
			camAngleRotateY -= 0.01f * elapsed;
		}
		if (GetAsyncKeyState(VK_UP))		// Rotate Cam Forward on the X Axis
		{
			// Rotate Up
			camAngleRotateX += 0.01f * elapsed;
		}
		if (GetAsyncKeyState(VK_DOWN))		// Rotate Cam Backward on the X Axis
		{
			// Rotate Down
			camAngleRotateX -= 0.01f * elapsed;
		}
		if (GetAsyncKeyState(VK_RSHIFT) && zoomInLevel > -65)	// Translate Cam Forward on the Z Axis
		{
			// Zoom in
			CamUserAppliedTranslate -= 0.01f * elapsed;
			zoomInLevel -= 1;
		}
		if (GetAsyncKeyState(VK_LSHIFT) && zoomInLevel < 10)	// Translate Cam Backward on the Z Axis
		{
			// Zoom out
			CamUserAppliedTranslate += 0.01f * elapsed;
			zoomInLevel += 1;
		}
		if (GetAsyncKeyState(VK_RETURN))
		{
			if (loadHenge == true)
			{
				loadHenge = false;
			}
			else if (loadHenge == false)
			{
				loadHenge = true;
			}
		}

		// Build Cam rotate and translate - then apply them
		Matrix4x4 camTranslationUser = BuildTranslationMatrix(0.0f, 0.0f, -CamUserAppliedTranslate);	// Build Translate Matrix
		Matrix4x4 CamUserAppliedRotX = BuildRotationMatrixOnAxisX(camAngleRotateX);						// Build Rotate Matrix 1
		Matrix4x4 CamUserAppliedRotY = BuildRotationMatrixOnAxisY(camAngleRotateY);						// Build Rotate Matrix 2
		SV_CameraMatrix = MultiplyMatrix4x4byMatrix4x4(CamUserAppliedRotX, SV_CameraMatrix);			// Apply Rotate 1
		SV_CameraMatrix = MultiplyMatrix4x4byMatrix4x4(CamUserAppliedRotY, SV_CameraMatrix);			// Apply Rotate 2
		SV_CameraMatrix = MultiplyMatrix4x4byMatrix4x4(camTranslationUser, SV_CameraMatrix);			// Apply Tranaslate

		// View Matrix
		SV_ViewMatrix = MatrixQuickInverse(SV_CameraMatrix);

		// Reset Rotation and Translation User Input Matrices after applying each frame
		CamUserAppliedTranslate = 0.0f;
		camAngleRotateX = 0.0f;
		camAngleRotateY = 0.0f;

#pragma endregion

		// Select Shaders, remember you can change these throughout runtime as needed:
		VertexShader = VS_Rasterizer;	// Set to object being manipulated's world matrix
		PixelShader = PS_White;			// Set to color desired
		
		// Set Rotation for Cube Objects
		cubeRot = 0.3f * elapsed;

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

		// Draw Grid
		//PixelShader = PS_White;																	// Set Pixel Shader to white
		//Matrix4x4 gridWorldMatrix = BuildDefaultMatrixIdentity();								// Build default World Matrix
		//SV_WorldMatrix = gridWorldMatrix;														// Make sure SV_WorldMatrix is set to our Grid's World Matrix 
		//DrawGrid();

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

		// Test Color Cube
		//PixelShader = PS_Green;																	// Set Pixel Shader to green
		//Matrix4x4 testcubeWorldMatrix = BuildDefaultMatrixIdentity();								// Build default World Matrix
		//Matrix4x4 testcubeTranslation = BuildTranslationMatrix(0.0f, 0.2f, 0.0f);					// Build Translation Matrix to move Cube up 
		//testcubeWorldMatrix = MultiplyMatrix4x4byMatrix4x4(testcubeWorldMatrix, testcubeTranslation);		// Apply Translation Matrix
		//Matrix4x4 testcubeRotation = BuildRotationMatrixOnAxisY(cubeRot);							// Build Rotation Matrix to rotate Cube with respect to time
		//testcubeWorldMatrix = MultiplyMatrix4x4byMatrix4x4(testcubeWorldMatrix, testcubeRotation);			// Apply Rotation Matrix
		//SV_WorldMatrix = testcubeWorldMatrix;
		//DrawColoredCube();

		// Draw Textured Cube
		//PixelShader = PS_DragonTexture;																	// Set Pixel Shader to green
		//Matrix4x4 cubeWorldMatrix = BuildDefaultMatrixIdentity();								// Build default World Matrix
		//Matrix4x4 cubeTranslation = BuildTranslationMatrix(0.0f, 0.2f, 0.0f);					// Build Translation Matrix to move Cube up 
		//cubeWorldMatrix = MultiplyMatrix4x4byMatrix4x4(cubeWorldMatrix, cubeTranslation);		// Apply Translation Matrix
		//Matrix4x4 cubeRotation = BuildRotationMatrixOnAxisY(cubeRot);							// Build Rotation Matrix to rotate Cube with respect to time
		//cubeWorldMatrix = MultiplyMatrix4x4byMatrix4x4(cubeWorldMatrix, cubeRotation);			// Apply Rotation Matrix
		//SV_WorldMatrix = cubeWorldMatrix;														// Make sure SV_WorldMatrix is set to our Cubes World Matrix																	
		//DrawTexturedCube();

#pragma endregion
		#pragma region Draw Code

		// Create Star Field
		Matrix4x4 starFieldWorldMatrix = BuildDefaultMatrixIdentity();
		SV_WorldMatrix = starFieldWorldMatrix;
		DrawStarField(starField);

		// Draw Models
		if (loadHenge == true)
		{
			Matrix4x4 stoneHengeWorldMatrix = BuildDefaultMatrixIdentity();
			SV_WorldMatrix = stoneHengeWorldMatrix;
			VertexShader = VS_Rasterizer;
			PixelShader = PS_StoneHengeTexture;
			DrawStoneHenge();
		}

#pragma endregion

	} while (RS_Update(screenSpace, screenSpacePixelCount));

	// End program
	RS_Shutdown();

	// end test
	return 0;
}