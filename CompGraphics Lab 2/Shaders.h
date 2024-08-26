#include "Math_Functions.h"

#pragma region Function Declarations

// VS
void VS_World(Vertex& multiplyMe);
void VS_View(Vertex& multiplyMe);
void VS_Perspective(Vertex& multiplyMe);
void VS_PerspectiveDivide(Vertex& divideMe);
void VS_Rasterizer(Vertex& rasterizeMe);
void VS_MoveLeft(Vertex& moveMe);
void VS_MoveRight(Vertex& moveMe);
void VS_MoveUp(Vertex& moveMe);
void VS_MoveDown(Vertex& moveMe);
void VS_MoveForward(Vertex& moveMe);
void VS_MoveBack(Vertex& moveMe);

// PS
void PS_White(Pixel& makeWhite);
void PS_Blue(Pixel& makeBlue);
void PS_Green(Pixel& makeGreen);
void PS_Red(Pixel& makeRed);
void PS_Black(Pixel& makeBlack);
void PS_Yellow(Pixel& makeYellow);
void PS_Grey(Pixel& makeGrey);
void PS_Purple(Pixel& makePurple);

#pragma endregion

// The active vertex shader. Modifies an incoming vertex. Pre-Rasterization.
void (*VertexShader) ( Vertex& ) = 0;

// The active pixel shader. Modifies outgoing pixel. Pre-Rasterization.
void (*PixelShader) ( Pixel& ) = 0;

// All Shader Variables (Prefix with "SV_")
#pragma region Shader Variables

// World Matrix Variable to be Defined & Initialized in main
Matrix4x4 SV_WorldMatrix;

// VS_Move Variable (now defunct)
float moveAmount = 0.5f;

// Camera Variables (Adjustable)
float fov = 90;															// Field of View (In Degrees)
float nearPlane = 0.1f;													// Near Plane Value - Used for clipping
float farPlane = 10;													// Far Plane Value - Used for Perspective
float aspectRatio = (float)screenSpaceHeight / (float)screenSpaceWidth;	// Aspect Ratio 

float camDistanceFromOrigin = 1.0f;										// Distance Camera Spawns from origin
float camAngleDegrees = -18.0f;											// Angle the camera looks down from at spawn

// Camera Matrix used to find View Matrix
Matrix4x4 CameraMatrixTEMP01 = BuildDefaultMatrixIdentity();										// Initialize basic matrix
Matrix4x4 camRotate = BuildRotationMatrixOnAxisX(ConvertDegreeToRadian(camAngleDegrees));			// Build desired camera rotation matrix
Matrix4x4 CameraMatrixTEMP02 = MultiplyMatrix4x4byMatrix4x4(CameraMatrixTEMP01, camRotate);	// Apply rotation matrix to camera to point slightly down
Matrix4x4 camTranslation = BuildTranslationMatrix(0.0f, 0.0f, camDistanceFromOrigin);				// Build desired camera distance from origin translation matrix
Matrix4x4 SV_CameraMatrix = MultiplyMatrix4x4byMatrix4x4(CameraMatrixTEMP02, camTranslation);	// Apply translation matrix to camera - moving backwards and up along the rotated Z axis

// View Matrix
Matrix4x4 SV_ViewMatrix = MatrixQuickInverse(SV_CameraMatrix);

// Perspective Matrix
Matrix4x4 SV_PerspectiveMatrix = BuildMatrixProjection(fov, aspectRatio, nearPlane, farPlane);;

#pragma endregion

// Custom Vertex and Pixel Shaders (Prefix with "VS_" and "PS_")
// Can be swapped using above function pointers as needed
#pragma region Custom Vertex Shader Functions

// Applies current world matrix
void VS_World(Vertex& multiplyMe)
{
	multiplyMe = MultiplyVectorbyMatrix4x4(SV_WorldMatrix, multiplyMe);
}

// Applies View Matrix
void VS_View(Vertex& multiplyMe)
{
	multiplyMe = MultiplyVectorbyMatrix4x4(SV_ViewMatrix, multiplyMe);
}

// Applies Perspective Matrix
void VS_Perspective(Vertex& multiplyMe)
{
	multiplyMe = MultiplyVectorbyMatrix4x4(SV_PerspectiveMatrix, multiplyMe);
}

// Applies Perspective Divide
void VS_PerspectiveDivide(Vertex& divideMe)
{
	divideMe.xyzw[0] = divideMe.xyzw[0] / divideMe.xyzw[3];	// X
	divideMe.xyzw[1] = divideMe.xyzw[1] / divideMe.xyzw[3];	// Y
	divideMe.xyzw[2] = divideMe.xyzw[2] / divideMe.xyzw[3];	// Z
	divideMe.xyzw[3] = divideMe.xyzw[3] / divideMe.xyzw[3];	// W
}

// Rasterizes a Vertex
void VS_Rasterizer(Vertex& rasterizeMe)
{
	// Convert Local Space to World Space
	Vertex world = MultiplyVectorbyMatrix4x4(SV_WorldMatrix, rasterizeMe);
	
	// Convert World Space to View Space
	Vertex view = MultiplyVectorbyMatrix4x4(SV_ViewMatrix, world);
	
	// Convert View Space to View Space prepped for Projection Space
	Vertex perspective = MultiplyVectorbyMatrix4x4(SV_PerspectiveMatrix, view);

	// Account for Perspective Divide, and Convert to Projection Space
	VS_PerspectiveDivide(perspective);

	rasterizeMe = perspective;
}

// Moves an Object left on the x axis.
void VS_MoveLeft(Vertex& moveMe)
{
	moveMe.xyzw[0] -= moveAmount;
}

// Moves an Object right on the x axis.
void VS_MoveRight(Vertex& moveMe)
{
	moveMe.xyzw[0] += moveAmount;
}

// Moves an Object up on the y axis.
void VS_MoveUp(Vertex& moveMe)
{
	moveMe.xyzw[1] += moveAmount;
}

// Moves an Object down on the y axis.
void VS_MoveDown(Vertex& moveMe)
{
	moveMe.xyzw[2] -= moveAmount;
}

// Moves an Object Forward on the z axis.
void VS_MoveForward(Vertex& moveMe)
{
	moveMe.xyzw[3] += moveAmount;
}

// Moves an Object backwards on the z axis.
void VS_MoveBack(Vertex& moveMe)
{
	moveMe.xyzw[3] -= moveAmount;
}

#pragma endregion
#pragma region Custom Pixel Shader Functions

// Basic pixel shader returns the color white
void PS_White(Pixel& makeWhite)
{
	makeWhite.color = 0xFFFFFFFF;
}

// Basic pixel shader returns the color blue
void PS_Blue(Pixel& makeBlue)
{
	makeBlue.color = 0xFF0000FF;
}

// Basic pixel shader returns the color green
void PS_Green(Pixel& makeGreen)
{
	makeGreen.color = 0xFF00FF00;
}

// Basic pixel shader returns the color red
void PS_Red(Pixel& makeRed)
{
	makeRed.color = 0xFFFF0000;
}

// Basic pixel shader returns the color black
void PS_Black(Pixel& makeBlack)
{
	makeBlack.color = 0xFF000000;
}

// Basic pixel shader returns the color aqua
void PS_Yellow(Pixel& makeYellow)
{
	makeYellow.color = 0xFFFFFF00;
}

// Basic pixel shader returns the color grey
void PS_Grey(Pixel& makeGrey)
{
	makeGrey.color = 0xFF808080;
}

// Basic pixel shader returns the color purple
void PS_Purple(Pixel& makePurple)
{
	makePurple.color = 0xFFFF00FF;
}

#pragma endregion
