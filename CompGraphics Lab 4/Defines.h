#include "XTime.h"
#include <vector>

// Create Screen Space Data (Adjustable)
// NOTE: If you change the width or height, always update the screenSpacePixelCount & the zBuffer size
// They should always equal width * height
int screenSpaceWidth = 700;					// Window Width
int screenSpaceHeight = 500;				// Window Height
const UINT screenSpacePixelCount = 350000;	// Total pixel count. Should always be equal to screenSpaceWidth * screenSpaceHeight
UINT screenSpace[screenSpacePixelCount];	// Screen space back buffer 

// Create Depth Buffer
float zBuffer[350000];

// Struct objects
#pragma region Structs

// Struct containing Color Data for an individual 2D point
struct Pixel
{
	UINT color = 0x00000000;

	float u = 0.0f;
	float v = 0.0f;
};

// Struct containing Color Data, (X, Y, Z, W) Coordinates, and UV Coordinates for a point.
// 0 = x, 
// 1 = y, 
// 2 = z, 
// 3 = w
struct Vertex
{
	// X, Y, Z 3D Coordinates
	float xyzw[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	// Color Data for current point
	Pixel color;

	// UV Texture Coordinates
	float uv[2] = { 0.0f, 0.0f };

	float nrm[3] = { 0.0f, 0.0f, 0.0f };
};

// Struct containing a Vertex with ScreenSpace data
// 0 = x, 
// 1 = y, 
// 2 = z, 
// 3 = w
struct Screen_XY
{
	Vertex convertedVertex;
};

// Struct containing 2 Vertices making up a line
// Syntax: Line line; Line.A = Vertex startPoint; line.B = Vertex endPoint
struct Line
{
	Vertex A, B;
};

// Struct containing (alpha, beta, gamma) Coordinates for a point relative to a Triangle
// 0 = alpha
// 1 = beta
// 2 = gamma
struct BaryocentricPoint
{
	float bya[3] = { 0.0f, 0.0f, 0.0f };
};

// Struct containing Color Data and 3 Vertices defining a Triangle's 3 points
// 0 = A
// 1 = B
// 2 = C
struct Triangle
{
	// 3 points of Triangle
	Vertex points[3];

	// Color
	Pixel color;

	// Face Normal
	Vertex faceNormal;
};

// 4x4 Matrix
struct Matrix4x4
{
	float v[4][4] = { 0 };
};

#pragma endregion

// Test Objects
#pragma region Test Objects

// Initialize constant drawing data (Test Line)
const Vertex uneditableOriginalLine[2] =
{
	// Defines fixed diagonal line crossing the origin
	{0.5f, -0.5f }, {-0.5f, 0.5f }
};

// Grid Test Object (Note: Only use one of these):
Vertex backLeft = { -0.5f, 0.0f, 0.5f, 1, 0xFFFFFFFF };			// Test Grid - Flat
Vertex backRight = { 0.5f, 0.0f, 0.5f, 1, 0xFFFFFFFF };
Vertex frontLeft = { -0.5f, -0.0f, -0.5f, 1, 0xFFFFFFFF };
Vertex frontRight = { 0.5f, -0.0f, -0.5f, 1, 0xFFFFFFFF };

//Vertex backLeft = { -0.5f, 0.5f, 0.0f, 1, 0xFFFFFFFF };				// Test Grid - Upright
//Vertex backRight = { 0.5f, 0.5f, 0.0f, 1, 0xFFFFFFFF };
//Vertex frontLeft = { -0.5f, -0.5f, -0.0f, 1, 0xFFFFFFFF };
//Vertex frontRight = { 0.5f, -0.5f, -0.0f, 1, 0xFFFFFFFF };

Triangle testTriangle
{
	testTriangle.points[0] = { -0.3f, 0.2f, -0.3f, 1 },
	testTriangle.points[1] = { 0.2f, 0.2f, -0.3f, 1 },
	testTriangle.points[2] = { -0.3f, -0.3f, -0.3f, 1 },

	testTriangle.color.color = 0xFFFF0000
};

#pragma endregion

// Usable Objects
#pragma region My Object Library

// Grid w/ Lines
// Horizontal Grid Lines
Vertex gridHorizontalLine_0[2] =
{
	{ -0.5f, 0.0f, -0.5f, 1 }, { 0.5f, 0.0f, -0.5f, 1 }
};
Vertex gridHorizontalLine_1[2] =
{
	{ -0.5f, 0.0f, -0.4f, 1 }, { 0.5f, 0.0f, -0.4f, 1 }
};
Vertex gridHorizontalLine_2[2] =
{
	{ -0.5f, 0.0f, -0.3f, 1 }, { 0.5f, 0.0f, -0.3f, 1 }
};
Vertex gridHorizontalLine_3[2] =
{
	{ -0.5f, 0.0f, -0.2f, 1 }, { 0.5f, 0.0f, -0.2f, 1 }
};
Vertex gridHorizontalLine_4[2] =
{
	{ -0.5f, 0.0f, -0.1f, 1 }, { 0.5f, 0.0f, -0.1f, 1 }
};
Vertex gridHorizontalLine_5[2] =
{
	{ -0.5f, 0.0f, 0.0f, 1 }, { 0.5f, 0.0f, 0.0f, 1 }
};
Vertex gridHorizontalLine_6[2] =
{
	{ -0.5f, 0.0f, 0.1f, 1 }, { 0.5f, 0.0f, 0.1f, 1 }
};
Vertex gridHorizontalLine_7[2] =
{
	{ -0.5f, 0.0f, 0.2f, 1 }, { 0.5f, 0.0f, 0.2f, 1 }
};
Vertex gridHorizontalLine_8[2] =
{
	{ -0.5f, 0.0f, 0.3f, 1 }, { 0.5f, 0.0f, 0.3f, 1 }
};
Vertex gridHorizontalLine_9[2] =
{
	{ -0.5f, 0.0f, 0.4f, 1 }, { 0.5f, 0.0f, 0.4f, 1 }
};
Vertex gridHorizontalLine_10[2] =
{
	{ -0.5f, 0.0f, 0.5f, 1 }, { 0.5f, 0.0f, 0.5f, 1 }

};

// Vertical Grid Lines
Vertex gridVertical_0[2] =
{
	{ -0.5f, 0.0f, -0.5f, 1 }, { -0.5f, 0.0f, 0.5f, 1 }
};
Vertex gridVertical_1[2] =
{
	{ -0.4f, 0.0f, -0.5f, 1 }, { -0.4f, 0.0f, 0.5f, 1 }
};
Vertex gridVertical_2[2] =
{
	{ -0.3f, 0.0f, -0.5f, 1 }, { -0.3f, 0.0f, 0.5f, 1 }
};
Vertex gridVertical_3[2] =
{
	{ -0.2f, 0.0f, -0.5f, 1 }, { -0.2f, 0.0f, 0.5f, 1 }
};
Vertex gridVertical_4[2] =
{
	{ -0.1f, 0.0f, -0.5f, 1 }, { -0.1f, 0.0f, 0.5f, 1 }
};
Vertex gridVertical_5[2] =
{
	{ 0.0f, 0.0f, -0.5f, 1 }, { 0.0f, 0.0f, 0.5f, 1 }
};
Vertex gridVertical_6[2] =
{
	{ 0.1f, 0.0f, -0.5f, 1 }, { 0.1f, 0.0f, 0.5f, 1 }
};
Vertex gridVertical_7[2] =
{ 
	{ 0.2f, 0.0f, -0.5f, 1 }, { 0.2f, 0.0f, 0.5f, 1 }
};
Vertex gridVertical_8[2] =
{
	{ 0.3f, 0.0f, -0.5f, 1 }, { 0.3f, 0.0f, 0.5f, 1 }
};
Vertex gridVertical_9[2] =
{
	{ 0.4f, 0.0f, -0.5f, 1 }, { 0.4f, 0.0f, 0.5f, 1 }
};
Vertex gridVertical_10[2] =
{
	{ 0.5f, 0.0f, -0.5f, 1 }, { 0.5f, 0.0f, 0.5f, 1 }
};

// Cube w/ Lines
Vertex cubeBackSquare_0[2]
{
	{ -0.3f, 0.3f, -0.3f, 1 }, { 0.2f, 0.3f, -0.3f, 1 }
};
Vertex cubeBackSquare_1[2]
{
	{ -0.3f, -0.2f, -0.3f, 1 }, { 0.2f, -0.2f, -0.3f, 1 }
}; 
Vertex cubeBackSquare_2[2]
{
	{ -0.3f, 0.3f, -0.3f, 1 }, {-0.3f, -0.2f, -0.3f, 1 }
}; 
Vertex cubeBackSquare_3[2]
{
	{ 0.2f, 0.3f, -0.3f, 1 }, { 0.2f, -0.2f, -0.3f, 1 }
};

Vertex cubeFrontSquare_0[2]
{
	{ -0.3f, 0.3f, 0.2f, 1 }, { 0.2f, 0.3f, 0.2f, 1 }
};
Vertex cubeFrontSquare_1[2]
{
	{ -0.3f, -0.2f, 0.2f, 1 }, { 0.2f, -0.2f, 0.2f, 1 }
};
Vertex cubeFrontSquare_2[2]
{
	{ -0.3f, 0.3f, 0.2f, 1 }, {-0.3f, -0.2f, 0.2f, 1 }
};
Vertex cubeFrontSquare_3[2]
{
	{ 0.2f, 0.3f, 0.2f, 1 }, { 0.2f, -0.2f, 0.2f, 1 }
};

Vertex cubeTopLeftConnector[2]
{
	{ -0.3f, 0.3f, -0.3f, 1 }, { -0.3f, 0.3f, 0.2f, 1 }
};
Vertex cubeBottomLeftConnector[2]
{
	{ -0.3f, -0.2f, -0.3f, 1 }, { -0.3f, -0.2f, 0.2f, 1 }
};
Vertex cubeTopRightConnector[2]
{
	{ 0.2f, 0.3f, -0.3f, 1 }, { 0.2f, 0.3f, 0.2f, 1 }
};
Vertex cubeBottomRightConnector[2]
{
	{ 0.2f, -0.2f, 0.2f, 1 }, { 0.2f, -0.2f, -0.3f, 1 }
};

Vertex cubeDebugLine01[2]
{
	{ -0.3f, -0.2f, -0.3f, 1 }, { 0.2f, -0.2f, 0.2f, 1 }
};

Vertex cubeDebugLine02[2]
{
	{ -0.3f, -0.2f, 0.2f, 1 }, { 0.2f, -0.2f, -0.3f, 1 }
};

// Cube w/ Triangles
Triangle cubeBackTri_01
{
	cubeBackTri_01.points[0] = { -0.3f, 0.2f, -0.3f, 1 },		// uv 1,0: top right
	cubeBackTri_01.points[1] = { 0.2f, 0.2f, -0.3f, 1 },		// uv 0,0: top left 
	cubeBackTri_01.points[2] = { -0.3f, -0.3f, -0.3f, 1 },		// uv 1,1: bottom right 

	cubeBackTri_01.color.color = 0xFFFF0000

};
Triangle cubeBackTri_02
{
	cubeBackTri_02.points[0] = { 0.2f, 0.2f, -0.3f, 1 },		 // uv 0,0: top left 
	cubeBackTri_02.points[1] = { -0.3f, -0.3f, -0.3f, 1 },		 // uv 1,1: bottom right 
	cubeBackTri_02.points[2] = { 0.2f, -0.3f, -0.3f, 1 },		 // uv 0,1: bottom left

	cubeBackTri_02.color.color = 0xFFFF0000
};

Triangle cubeFrontTri_01
{
	cubeFrontTri_01.points[0] = { -0.3f, 0.2f, 0.2f, 1 },		// uv 0,0: top left
	cubeFrontTri_01.points[1] = { 0.2f, 0.2f, 0.2f, 1 },		// uv 1,0: top right
	cubeFrontTri_01.points[2] = { -0.3f, -0.3f, 0.2f, 1 },		// uv 0,1: bottom left

	cubeFrontTri_01.color.color = 0xFFFFFF00
};
Triangle cubeFrontTri_02
{
	cubeFrontTri_02.points[0] = { 0.2f, 0.2f, 0.2f, 1 },		// uv 1,0: top right
	cubeFrontTri_02.points[1] = { -0.3f, -0.3f, 0.2f, 1 },		// uv 0,1: bottom left
	cubeFrontTri_02.points[2] = { 0.2f, -0.3f, 0.2f, 1 },		// uv 1,1: bottom right

	cubeFrontTri_02.color.color = 0xFFFFFF00
};

Triangle cubeLeftTri_01
{
	cubeLeftTri_01.points[0] = { -0.3f, 0.2f, 0.2f, 1 },		// uv 1,0: top right
	cubeLeftTri_01.points[1] = { -0.3f, -0.3f, 0.2f, 1 },		// uv 1,1: bottom right
	cubeLeftTri_01.points[2] = { -0.3f, -0.3f, -0.3f, 1 },		// uv 0,1: bottom left

	cubeLeftTri_01.color.color = 0xFF0000FF
};
Triangle cubeLeftTri_02
{
	cubeLeftTri_02.points[0] = { -0.3f, -0.3f, -0.3f, 1 },		// uv 0,1: bottom left
	cubeLeftTri_02.points[1] = { -0.3f, 0.2f, -0.3f, 1 },		// uv 0,0: top left
	cubeLeftTri_02.points[2] = { -0.3f, 0.2f, 0.2f, 1 },		// uv 1,0: top right

	cubeLeftTri_02.color.color = 0xFF0000FF
};

Triangle cubeRightTri_01
{
	cubeRightTri_01.points[0] = { 0.2f, 0.2f, 0.2f, 1 },		 // uv 0,0: top left
	cubeRightTri_01.points[1] = { 0.2f, -0.3f, 0.2f, 1 },		 // uv 0,1: bottom left
	cubeRightTri_01.points[2] = { 0.2f, -0.3f, -0.3f, 1 },		 // uv 1,1: bottom right

	cubeRightTri_01.color.color = 0xFF00FF00
};
Triangle cubeRightTri_02
{
	cubeRightTri_02.points[0] = { 0.2f, -0.3f, -0.3f, 1 },		 // uv 1,1: bottom right
	cubeRightTri_02.points[1] = { 0.2f, 0.2f, -0.3f, 1 },		 // uv 1,0: top right
	cubeRightTri_02.points[2] = { 0.2f, 0.2f, 0.2f, 1 },		 // uv 0,0: top left

	cubeRightTri_02.color.color = 0xFF00FF00
};

Triangle cubeBottomTri_01
{
	cubeBottomTri_01.points[0] = { -0.3f, -0.3f, 0.2f, 1 },		// uv 0,1: bottom left
	cubeBottomTri_01.points[1] = { -0.3f, -0.3f, -0.3f, 1 },	// uv 0,0: top left
	cubeBottomTri_01.points[2] = { 0.2f, -0.3f, 0.2f, 1 },		// uv 1,0: top right

	cubeBottomTri_01.color.color = 0xFFFF00FF
};
Triangle cubeBottomTri_02
{
	cubeBottomTri_02.points[0] = { 0.2f, -0.3f, 0.2f, 1 },		// uv 1,0: top right
	cubeBottomTri_02.points[1] = { 0.2f, -0.3f, -0.3f, 1 },		// uv 1,1: bottom right
	cubeBottomTri_02.points[2] = { -0.3f, -0.3f, -0.3f, 1 },	// uv 0,0: top left

	cubeBottomTri_02.color.color = 0xFFFF00FF
};

Triangle cubeTopTri_01
{
	cubeTopTri_01.points[0] = { -0.3f, 0.2f, 0.2f, 1 },			// uv 0,1: bottom left
	cubeTopTri_01.points[1] = { -0.3f, 0.2f, -0.3f, 1 },		// uv 0,0: top left
	cubeTopTri_01.points[2] = { 0.2f, 0.2f, -0.3f, 1 },			// uv 1,0: top right

	cubeTopTri_01.color.color = 0xFFFFFFFF
};
Triangle cubeTopTri_02
{
	cubeTopTri_02.points[0] = { -0.3f, 0.2f, 0.2f, 1 },			// uv 0,1: bottom left
	cubeTopTri_02.points[1] = { 0.2f, 0.2f, 0.2f, 1 },			// uv 1,1
	cubeTopTri_02.points[2] = { 0.2f, 0.2f, -0.3f, 1 },			// uv 1,0: top right

	cubeTopTri_02.color.color = 0xFFFFFFFF
};


#pragma endregion

// Create Stone Henge Triangle Array
Triangle stoneHenge[844];
