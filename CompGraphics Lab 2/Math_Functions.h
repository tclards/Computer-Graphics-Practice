#include <corecrt_math.h>
#include "Defines.h"

#pragma region Function Declarations

// Math
float lerp(float a, float b, float f);
UINT ConvertBGRAtoARGB(UINT valueToConvert);
UINT AlphaBlend(UINT pixel_01, UINT pixel_02);
int ConvertCoordinate_2Dto1D(int x1, int y1, int arrayWidth, int arrayHeight);
Screen_XY ConvertCoordinate_3Dto2D(Vertex V);
float ConvertDegreeToRadian(float degreeValue);

// Matrix Math
Matrix4x4 BuildDefaultMatrixIdentity();
Matrix4x4 BuildTranslationMatrix(float x, float y, float z);
Matrix4x4 BuildRotationMatrixOnAxisX(float angleRad);
Matrix4x4 BuildRotationMatrixOnAxisY(float angleRad);
Matrix4x4 BuildRotationMatrixOnAxisZ(float angleRad);
Matrix4x4 MultiplyMatrix4x4byMatrix4x4(Matrix4x4& m1, Matrix4x4& m2);
Vertex MultiplyVectorbyMatrix4x4(Matrix4x4& m, Vertex& i);
Matrix4x4 MatrixQuickInverse(Matrix4x4& m);
Matrix4x4 BuildMatrixProjection(float fov, float aspectRatio, float nearPlane, float farPlane);

#pragma endregion

// Math Functions
#pragma region Math Functions

// Floating Point Linear Interpolation Function
float lerp(float a, float b, float f)
{
	return (a * (1.0f - f)) + (b * f);
}

// Converts unsigned int color value from BGRA to ARGB
UINT ConvertBGRAtoARGB(UINT valueToConvert)
{
	// Extract BGRA values
	UINT alpha = valueToConvert & 0x000000ff;
	UINT red = valueToConvert & 0x0000ff00;
	UINT green = valueToConvert & 0x00ff0000;
	UINT blue = valueToConvert & 0xff000000;

	// place BGRA values in the correct spots for ARGB
	alpha = (alpha << 24);
	red = (red << 8);
	green = (green >> 8);
	blue = (blue >> 24);

	// simplify extracted pieces and create new color
	UINT ARGB = 0x00000000;
	ARGB = ARGB | blue;
	ARGB = ARGB | green;
	ARGB = ARGB | red;
	ARGB = ARGB | alpha;

	return ARGB;
}

// Blend ARGB colors using Lerp, based on alpha value
UINT AlphaBlend(UINT pixel_01, UINT pixel_02)
{
	// get mask values
	UINT alphaMask = 0xff000000;
	UINT redMask = 0x00ff0000;
	UINT blueMask = 0x000000ff;
	UINT redBlueMask = 0x00ff00ff;
	UINT greenMask = 0x0000ff00;
	UINT alphaGreenMask = alphaMask | greenMask;
	UINT oneAlpha = 0x01000000;

	// blend values:
	// alpha
	UINT alpha = (pixel_02 & alphaMask) >> 24;

	// newAlpha
	UINT newAlpha = 255 - alpha;

	// redBlue
	UINT redBlue = (newAlpha * (pixel_01 & redBlueMask)) + (alpha * (pixel_02 & redBlueMask));
	redBlue = redBlue >> 8;

	// alphaGreen
	UINT alphaGreen = (newAlpha * ((pixel_01 & alphaGreenMask) >> 8)) + (alpha * (oneAlpha | ((pixel_02 & greenMask) >> 8)));

	// redBlue_final
	UINT rb_final = redBlue & redBlueMask;

	// alphaGreen_final
	UINT ab_final = alphaGreen & alphaGreenMask;

	UINT finalColor = rb_final | ab_final;

	return finalColor;
}

// Convert any 2D (X,Y) coordinate into a 1D machine-usable coordinate
int ConvertCoordinate_2Dto1D(int x1, int y1, int arrayWidth, int arrayHeight)
{
	int coordinate_1D;

	// Convert x,y to 1D
	int tempY = arrayWidth * y1;
	coordinate_1D = tempY + x1;

	return coordinate_1D;
}

// Converts NDC 3D coordinates to pixelSpace 2D coordinates
Screen_XY ConvertCoordinate_3Dto2D(Vertex V)
{
	// Get Values to be Converted
	float xTemp = V.xyzw[0];
	float yTemp = -V.xyzw[1];
	float zTemp = V.xyzw[2];
	float wTemp = V.xyzw[3];

	// Get Screen Data
	float displayX = (float)screenSpaceWidth;
	float displayY = (float)screenSpaceHeight;

	// Convert X Value
	float xFinal = ((xTemp + 1.0f) * 0.5f) * displayX;

	// Convert Y Value
	float yFinal = ((yTemp + 1.0f) * 0.5f) * displayY;

	// Carry over Z Value
	float zFinal = zTemp;

	// Carry over W Value
	float wFinal = wTemp;

	// Return Vertex converted
	Vertex vConverted;
	vConverted.xyzw[0] = xFinal;
	vConverted.xyzw[1] = yFinal;
	vConverted.xyzw[2] = zFinal;
	vConverted.xyzw[3] = wFinal;

	Screen_XY screen;
	screen.convertedVertex = vConverted;

	return screen;
}

// Converts Degree values to Radian values
float ConvertDegreeToRadian(float degreeValue)
{
	float radianValue = 1.0f / tanf(degreeValue * 0.5f / 180.0f * 3.14159f);

	return radianValue;
}

#pragma endregion

// Matrix Math Functions
#pragma region Matrix Math Functions

// Can be called to build new Matrix4x4 object
// Syntax: Matrix4x4 newMatrix; newMatrix = BuildDefaultMatrixIdentity();
Matrix4x4 BuildDefaultMatrixIdentity()
{
	Matrix4x4 matrixDefault;

	matrixDefault.v[0][0] = 1.0f;
	matrixDefault.v[1][1] = 1.0f;
	matrixDefault.v[2][2] = 1.0f;
	matrixDefault.v[3][3] = 1.0f;

	return matrixDefault;
}

// Can be called to build new Translation Matrix
// Syntax: Matrix4x4 newMatrixTrans; newMatrixTrans = BuildTranslationMatrix();
Matrix4x4 BuildTranslationMatrix(float x, float y, float z)
{
	Matrix4x4 matrixTranslation;

	matrixTranslation.v[0][0] = 1.0f;
	matrixTranslation.v[1][1] = 1.0f;
	matrixTranslation.v[2][2] = 1.0f;
	matrixTranslation.v[3][3] = 1.0f;
	matrixTranslation.v[3][0] = x;
	matrixTranslation.v[3][1] = y;
	matrixTranslation.v[3][2] = z;

	return matrixTranslation;
}

// Can be called to build new Rotational Matrix using X Axis
// Takes in the angle in RADIANS - convert first if using Degrees
// Syntax: Matrix4x4 newMatrixRotX; newMatrixRotX = BuildTranslationMatrix(desiredAngle);
Matrix4x4 BuildRotationMatrixOnAxisX(float angle)
{
	Matrix4x4 matrixRotationX;

	matrixRotationX.v[0][0] = 1.0f;
	matrixRotationX.v[1][1] = cosf(angle);
	matrixRotationX.v[1][2] = sinf(angle);
	matrixRotationX.v[2][1] = -sinf(angle);
	matrixRotationX.v[2][2] = cosf(angle);
	matrixRotationX.v[3][3] = 1.0f;

	return matrixRotationX;
}

// Can be called to build new Rotational Matrix using Y Axis
// Takes in the angle in RADIANS - convert first if using Degrees
// Syntax: Matrix4x4 newMatrixRotY; newMatrixRotY = BuildTranslationMatrix(desiredAngle);
Matrix4x4 BuildRotationMatrixOnAxisY(float angle)
{
	Matrix4x4 matrixRotationY;

	matrixRotationY.v[0][0] = cosf(angle);
	matrixRotationY.v[0][2] = sinf(angle);
	matrixRotationY.v[2][0] = -sinf(angle);
	matrixRotationY.v[1][1] = 1.0f;
	matrixRotationY.v[2][2] = cosf(angle);
	matrixRotationY.v[3][3] = 1.0f;

	return matrixRotationY;
}

// Can be called to build new Rotational Matrix using Z Axis
// Takes in the angle in RADIANS - convert first if using Degrees
// Syntax: Matrix4x4 newMatrixRotZ; newMatrixRotZ = BuildTranslationMatrix(desiredAngle);
Matrix4x4 BuildRotationMatrixOnAxisZ(float angle)
{
	Matrix4x4 matrixRotationZ;

	matrixRotationZ.v[0][0] = cosf(angle);
	matrixRotationZ.v[0][1] = sinf(angle);
	matrixRotationZ.v[1][0] = -sinf(angle);
	matrixRotationZ.v[1][1] = cosf(angle);
	matrixRotationZ.v[2][2] = 1.0f;
	matrixRotationZ.v[3][3] = 1.0f;

	return matrixRotationZ;
}

// Multiply Matrix4x4 by Matrix4x4 and return new Matrix4x4
Matrix4x4 MultiplyMatrix4x4byMatrix4x4(Matrix4x4& m1, Matrix4x4& m2)
{
	Matrix4x4 matrixMult;

	for (int c = 0; c < 4; c++)
	{
		for (int r = 0; r < 4; r++)
		{
			matrixMult.v[r][c] = m1.v[r][0] * m2.v[0][c] + m1.v[r][1] * m2.v[1][c] + m1.v[r][2] * m2.v[2][c] + m1.v[r][3] * m2.v[3][c];

		}
	}
		
	return matrixMult;
}

// Multiply Vector by Matrix4x4 and return new Vector
Vertex MultiplyVectorbyMatrix4x4(Matrix4x4& m, Vertex& i)
{
	Vertex v;

	// Calculate X
	v.xyzw[0] = i.xyzw[0] * m.v[0][0] + i.xyzw[1] * m.v[1][0] + i.xyzw[2] * m.v[2][0] + i.xyzw[3] * m.v[3][0];
							  						  						  						 
	// Calculate Y			  						  						  						 
	v.xyzw[1] = i.xyzw[0] * m.v[0][1] + i.xyzw[1] * m.v[1][1] + i.xyzw[2] * m.v[2][1] + i.xyzw[3] * m.v[3][1];
					
	// Calculate Z	
	v.xyzw[2] = i.xyzw[0] * m.v[0][2] + i.xyzw[1] * m.v[1][2] + i.xyzw[2] * m.v[2][2] + i.xyzw[3] * m.v[3][2];
					
	// Calculate W	
	v.xyzw[3] = i.xyzw[0] * m.v[0][3] + i.xyzw[1] * m.v[1][3] + i.xyzw[2] * m.v[2][3] + i.xyzw[3] * m.v[3][3];
	
	return v;
}

// Quickly calculate the inverse for Rotation Matrices and/or Translation Matrices
Matrix4x4 MatrixQuickInverse(Matrix4x4& m)
{
	Matrix4x4 matrixInverse;

	matrixInverse.v[0][0] = m.v[0][0]; 
	matrixInverse.v[0][1] = m.v[1][0]; 
	matrixInverse.v[0][2] = m.v[2][0]; 
	matrixInverse.v[0][3] = 0.0f;
				  
	matrixInverse.v[1][0] = m.v[0][1]; 
	matrixInverse.v[1][1] = m.v[1][1]; 
	matrixInverse.v[1][2] = m.v[2][1]; 
	matrixInverse.v[1][3] = 0.0f;
				  
	matrixInverse.v[2][0] = m.v[0][2]; 
	matrixInverse.v[2][1] = m.v[1][2]; 
	matrixInverse.v[2][2] = m.v[2][2]; 
	matrixInverse.v[2][3] = 0.0f;
				  
	matrixInverse.v[3][0] = -(m.v[3][0] * matrixInverse.v[0][0] + m.v[3][1] * matrixInverse.v[1][0] + m.v[3][2] * matrixInverse.v[2][0]);
	matrixInverse.v[3][1] = -(m.v[3][0] * matrixInverse.v[0][1] + m.v[3][1] * matrixInverse.v[1][1] + m.v[3][2] * matrixInverse.v[2][1]);
	matrixInverse.v[3][2] = -(m.v[3][0] * matrixInverse.v[0][2] + m.v[3][1] * matrixInverse.v[1][2] + m.v[3][2] * matrixInverse.v[2][2]);
	matrixInverse.v[3][3] = 1.0f;

	return matrixInverse;
}

//Can be called to build new Projection Matrix
// Syntax: Matrix4x4 newMatrixPerspective; newMatrixPerspective = BuildMatrixProjection(fov, aspectRatio, nearPlane, farPlane);
Matrix4x4 BuildMatrixProjection(float fov, float aspectRatio, float nearPlane, float farPlane)
{
	float fovRad = ConvertDegreeToRadian(fov);

	Matrix4x4 matrixProjected;
	matrixProjected.v[0][0] = aspectRatio * fovRad;
	matrixProjected.v[1][1] = fovRad;
	matrixProjected.v[2][2] = farPlane / (farPlane - nearPlane);
	matrixProjected.v[3][2] = (-farPlane * nearPlane) / (farPlane - nearPlane);
	matrixProjected.v[2][3] = 1.0f;
	matrixProjected.v[3][3] = 0.0f;

	return matrixProjected;
}

#pragma endregion
