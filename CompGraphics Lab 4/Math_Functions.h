#include <corecrt_math.h>
#include <algorithm>
#include <random>
#include "Defines.h"

#pragma region Function Declarations

// Math
UINT CombineColors(UINT color1, UINT color2);
UINT ModulateColors(float lightRatio, UINT color1, UINT color2);
float Saturate(float clampMe);
float FindSmallestFloat(float a, float b, float c);
float FindLargestFloat(float a, float b, float c);
float lerp(float a, float b, float f);
float berpZ(BaryocentricPoint aby, Triangle tri);
float berpX(BaryocentricPoint aby, Triangle tri);
float berpY(BaryocentricPoint aby, Triangle tri);
float berpU(BaryocentricPoint aby, Triangle tri);
float berpV(BaryocentricPoint aby, Triangle tri);
UINT ConvertBGRAtoARGB(UINT valueToConvert);
UINT AlphaBlend(UINT pixel_01, UINT pixel_02);
int ConvertCoordinate_2Dto1D(int x1, int y1, int arrayWidth, int arrayHeight);
Screen_XY ConvertCoordinate_3Dto2D(Vertex V);
float ConvertDegreeToRadian(float degreeValue);
float AreaOfATriangle(Triangle tri);
bool PointIsInsideTriangle(BaryocentricPoint bya, Triangle tri);
BaryocentricPoint BaryocentricCoords(Vertex p, Triangle tri, float alphaRatio, float betaRatio);

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

// Vector Math Declarations
Vertex VertexAddition(Vertex& v1, Vertex& v2);
Vertex VertexSubtraction(Vertex& v1, Vertex& v2);
Vertex VertexMultiplication(Vertex& v1, float k);
Vertex VertexDivision(Vertex& v1, float k);
float VertexDotProduct(Vertex& v1, Vertex& v2);
float VertexLength(Vertex& v);
Vertex VertexNormalize(Vertex& v);
Vertex VertexCrossProduct(Vertex& v1, Vertex& v2);

#pragma endregion

// Math Functions
#pragma region Math Functions

// Combines two colors additively
UINT CombineColors(UINT color1, UINT color2)
{
	return color1 + color2;
}

// Combines two colors multiplicatively
UINT ModulateColors(float lightRatio, UINT lightColor, UINT surfaceColor)
{
	// Get Ratios
	float redRatio = (lightRatio * ((lightColor >> 16) & 0xff)) / 255;
	UINT rValue = ((surfaceColor >> 16) & 0xff);
	float greenRatio = (lightRatio * ((lightColor >> 8) & 0xff)) / 255;
	UINT gValue = ((surfaceColor >> 8) & 0xff);
	float blueRatio = (lightRatio * (lightColor & 0xff)) / 255;
	UINT bValue = (surfaceColor & 0xff);

	// Get color
	UINT finalRed = redRatio * rValue;
	UINT finalGreen = greenRatio * gValue;
	UINT finalBlue = blueRatio * bValue;

	// Build and Return modulated color
	UINT color = finalRed & finalGreen;
	color = color & finalBlue;

	return color;
}

// Clamps a value between 0 and 1
float Saturate(float clampMe)
{
	if (clampMe < 0)
	{
		clampMe = 0;
	}
	else if (clampMe > 1)
	{
		clampMe = 1;
	}

	return clampMe;
}

// Finds and returns the smallest of 3 float values
float FindSmallestFloat(float a, float b, float c)
{
	if (a < b && a < c)
	{
		return a;
	}
	else if (b < a && b < c)
	{
		return b;
	}
	else if (c < a && c < b)
	{
		return c;
	}
	else
	{
		return 0.0f;
	}
}

// Finds and returns the largest of 3 float values
float FindLargestFloat(float a, float b, float c)
{
	if (a > b && a > c)
	{
		return a;
	}
	else if (b > a && b > c)
	{
		return b;
	}
	else if (c > a && c > b)
	{
		return c;
	}
	else
	{
		return 0.0f;
	}
}

// Floating Point Linear Interpolation Function
float lerp(float a, float b, float f)
{
	return (a * (1.0f - f)) + (b * f);
}

// Floating Point Baryocentric Interpolation Function for Z Values
float berpZ(BaryocentricPoint aby, Triangle tri)
{
	// Get Value for Point in question
	float alpha = aby.bya[0];
	float beta = aby.bya[1];
	float gamma = aby.bya[2];

	// Get Values for Triangle we are working in
	Vertex A = tri.points[0];
	float Ax = A.xyzw[0];
	float Ay = A.xyzw[1];
	float Az = A.xyzw[2];
	Vertex B = tri.points[1];
	float Bx = B.xyzw[0];
	float By = B.xyzw[1];
	float Bz = B.xyzw[2];
	Vertex C = tri.points[2];
	float Cx = C.xyzw[0];
	float Cy = C.xyzw[1];
	float Cz = C.xyzw[2];

	// Interpolate Z value of Point
	float interpolatedZ = (Az * alpha) + (Bz * beta) + (Cz * gamma);

	// Return interpolated value
	return interpolatedZ;
}

// Floating Point Baryocentric Interpolation Function for X Values
float berpX(BaryocentricPoint aby, Triangle tri)
{
	// Get Value for Point in question
	float alpha = aby.bya[0];
	float beta = aby.bya[1];
	float gamma = aby.bya[2];

	// Get Values for Triangle we are working in
	Vertex A = tri.points[0];
	float Ax = A.xyzw[0];
	Vertex B = tri.points[1];
	float Bx = B.xyzw[0];
	Vertex C = tri.points[2];
	float Cx = C.xyzw[0];

	// Interpolate X value of Point
	float interpolatedX = (alpha * Ax) + (beta * Bx) + (gamma * Cx);

	// Return interpolated value
	return interpolatedX;
}

// Floating Point Baryocentric Interpolation Function for Y Values
float berpY(BaryocentricPoint aby, Triangle tri)
{
	// Get Value for Point in question
	float alpha = aby.bya[0];
	float beta = aby.bya[1];
	float gamma = aby.bya[2];

	// Get Values for Triangle we are working in
	Vertex A = tri.points[0];
	float Ay = A.xyzw[1];
	Vertex B = tri.points[1];
	float By = B.xyzw[1];
	Vertex C = tri.points[2];
	float Cy = C.xyzw[1];

	// Interpolate Y value of Point
	float interpolatedY = (alpha * Ay) + (beta * By) + (gamma * Cy);

	// Return interpolated value
	return interpolatedY;
}

// Floating Point Baryocentric Interpolation Function for U Values
float berpU(BaryocentricPoint aby, Triangle tri)
{
	// Get Value for Point in question
	float alpha = aby.bya[0];
	float beta = aby.bya[1];
	float gamma = aby.bya[2];

	// Get Values for Triangle we are working in
	Vertex A = tri.points[0];
	float Au = A.uv[0];
	Vertex B = tri.points[1];
	float Bu = B.uv[0];
	Vertex C = tri.points[2];
	float Cu = C.uv[0];

	// Interpolate U value of Point
	float interpolatedU = (alpha * Au) + (beta * Bu) + (gamma * Cu);

	// Return interpolated value
	return interpolatedU;
}

// Floating Point Baryocentric Interpolation Function for V Values
float berpV(BaryocentricPoint aby, Triangle tri)
{
	// Get Value for Point in question
	float alpha = aby.bya[0];
	float beta = aby.bya[1];
	float gamma = aby.bya[2];

	// Get Values for Triangle we are working in
	Vertex A = tri.points[0];
	float Av = A.uv[1];
	Vertex B = tri.points[1];
	float Bv = B.uv[1];
	Vertex C = tri.points[2];
	float Cv = C.uv[1];

	// Interpolate V value of Point
	float interpolatedV = (alpha * Av) + (beta * Bv) + (gamma * Cv);

	// Return interpolated value
	return interpolatedV;
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
	float yTemp = V.xyzw[1];
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

// Returns the Area of a Triangle
float AreaOfATriangle(Triangle tri)
{
	// Get Triangle Vertex Data
	Vertex X1 = tri.points[0];
	Vertex X2 = tri.points[1];
	Vertex X3 = tri.points[2];

	return abs((X1.xyzw[0] * (X2.xyzw[1] - X3.xyzw[1]) + X2.xyzw[0] * (X3.xyzw[1] - X1.xyzw[1]) + X3.xyzw[0] * (X1.xyzw[1] - X2.xyzw[1])) / 2.0);
}

// Determines if a given point is inside a given Triangle
// Returns True if the Point is inside the Triangle
bool PointIsInsideTriangle(BaryocentricPoint bya, Triangle tri)
{
	// If the coordinates are in range, return true
	// If the coordinates are not in range, return false
	if (bya.bya[0] >= 0 && bya.bya[0] <= 1 && bya.bya[1] >= 0 && bya.bya[1] <= 1 && bya.bya[2] >= 0 && bya.bya[2] <= 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Finds the Baryocentric Coordinates of a point relative to a given Triangle
BaryocentricPoint BaryocentricCoords(Vertex p, Triangle tri, float alphaRatio, float betaRatio)
{
	// Get Triangles Vertices
	Vertex p1 = tri.points[0];
	Vertex p2 = tri.points[1];
	Vertex p3 = tri.points[2];

	// Calculate bya Baryocentric Coordinates
	float alpha = ((p2.xyzw[1] - p3.xyzw[1]) * (p.xyzw[0] - p3.xyzw[0]) + (p3.xyzw[0] - p2.xyzw[0]) * (p.xyzw[1] - p3.xyzw[1])) * alphaRatio;
	
	float beta = ((p3.xyzw[1] - p1.xyzw[1]) * (p.xyzw[0] - p3.xyzw[0]) + (p1.xyzw[0] - p3.xyzw[0]) * (p.xyzw[1] - p3.xyzw[1])) * betaRatio;
	
	float gamma = 1.0f - alpha - beta;

	// Create a return Baryocentric Point object
	BaryocentricPoint bya;
	bya.bya[0] = alpha;
	bya.bya[1] = beta;
	bya.bya[2] = gamma;

	return bya;
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

// Vector Math Functions
#pragma region Vector Math

// Add a Vertex to a Vertex and return a new Vertex
Vertex VertexAddition(Vertex& v1, Vertex& v2)
{
	return { v1.xyzw[0] + v2.xyzw[0], v1.xyzw[1] + v2.xyzw[1], v1.xyzw[2] + v2.xyzw[2] };
}

// Subtract a Vertex from a Vertex and return a new Vertex
Vertex VertexSubtraction(Vertex& v1, Vertex& v2)
{
	return { v1.xyzw[0] - v2.xyzw[0], v1.xyzw[1] - v2.xyzw[1], v1.xyzw[2] - v2.xyzw[2] };
}

// Multiplies a Vertex and a Float and returns a new Vertex
Vertex VertexMultiplication(Vertex& v1, float f)
{
	return { v1.xyzw[0] * f, v1.xyzw[1] * f, v1.xyzw[2] * f };
}

// Divides a Vertex by a Float and returns a new Vertex
Vertex VertexDivision(Vertex& v1, float f)
{
	return { v1.xyzw[0] / f, v1.xyzw[1] / f, v1.xyzw[2] / f };
}

// Multiplies a Vertex by a Vertex to find a Dot Product and returns a new Vertex
float VertexDotProduct(Vertex& v1, Vertex& v2)
{
	return v1.xyzw[0] * v2.xyzw[0] + v1.xyzw[1] * v2.xyzw[1] + v1.xyzw[2] * v2.xyzw[2];
}

// Calculates length of a Vertex
float VertexLength(Vertex& v)
{
	return sqrtf(VertexDotProduct(v, v));
}

// Normalize a Vertex and return a new Vertex
Vertex VertexNormalize(Vertex& v)
{
	float l = VertexLength(v);
	return { v.xyzw[0] / l, v.xyzw[1] / l, v.xyzw[2] / l };
}

// Multiplies a Vertex by a Vertex to find a Cross Product and returns a new Vertex
Vertex VertexCrossProduct(Vertex& v1, Vertex& v2)
{
	Vertex v;

	v.xyzw[0] = v1.xyzw[1] * v2.xyzw[2] - v1.xyzw[2] * v2.xyzw[1];
	v.xyzw[1] = v1.xyzw[2] * v2.xyzw[0] - v1.xyzw[0] * v2.xyzw[2];
	v.xyzw[2] = v1.xyzw[0] * v2.xyzw[1] - v1.xyzw[1] * v2.xyzw[0];

	return v;
}

#pragma endregion