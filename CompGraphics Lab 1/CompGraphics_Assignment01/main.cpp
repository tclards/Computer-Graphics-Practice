#include <Windows.h>			// Built in Library Files
#include <random>

#include "RasterSurface.h"		// Provided Library Files
#include "XTime.h"

#include "tiles_12.h"			// Image Data Files
#include "fire_02.h"

// Declare functions
UINT ConvertBGRAtoARGB(UINT valueToConvert);
UINT AlphaBlend(UINT pixel_01, UINT pixel_02);
void ClearBuffer(UINT color);
void ClearBufferLocation(int x1, int y1, int targetWidth, int targetHeight, UINT color);
int ConvertCoordinate_2Dto1D(int x1, int y1, int arrayWidth, int arrayHeight);
bool isARGBFullyTransparent(UINT color);
void DrawPixel(int x1, int y1, UINT color);
void BLIT_SingleTile(const unsigned int sourceArray[], int sourceWidth, int sourceHeight, UINT destinationArray[], int destWidth, int destHeight, UINT sourceRect[], int sourceRectWidth, int sourceRectHeight, int destX1, int destY1);
void BLIT_WholeWindow(const unsigned int sourceArray[], int sourceWidth, int sourceHeight, UINT destinationArray[], int destWidth, int destHeight, UINT sourceRect[], int sourceRectWidth, int sourceRectHeight, int destX1, int destY1);
void fireCellSwitch(UINT fireCell[], int count);

// Initialize Variables
int RS_Width = 500;						// Window Width
int RS_Height = 500;					// Window Height

const UINT RS_NumPixels = 250000;		// Total Pixels present in Window
UINT BackBuffer[RS_NumPixels];			// Array containing all pixel data

// Main Program Loop
int main(void)
{
	// Create window
	RS_Initialize("Tyler Clardy Lab#1", RS_Width, RS_Height);

	// Set Up Random Number Generator for Tree objects
	std::random_device rd;											// obtain random number from hardware
	std::mt19937 gen(rd());											// seed random number generator
	std::uniform_int_distribution<> distrX(0, RS_Width - 63);		// define X range
	std::uniform_int_distribution<> distrY(0, RS_Height - 78);		// define Y range

	// Generate Random Locations
	int randXLocations[10];
	int randYLocations[10];
	for (int i = 0; i < 10; i++)
	{
		randXLocations[i] = distrX(gen);
	}
	for (int i = 0; i < 10; i++)
	{
		randYLocations[i] = distrY(gen);
	}

	// Grass Tile Source Coordinates: 288,128 & 320,160
	UINT grassSourceRect[4];
	grassSourceRect[0] = 288;		// X1
	grassSourceRect[1] = 128;		// Y1
	grassSourceRect[2] = 320;		// X2
	grassSourceRect[3] = 160;		// Y2
	int grassRectWidth = 32;
	int grassRectHeight = 32;

	// Tree object Tile Source Coordinates: 319,15 & 384,96
	UINT treeSourceRect[4];
	treeSourceRect[0] = 320;		// X1
	treeSourceRect[1] = 16;			// Y1
	treeSourceRect[2] = 383;		// X2
	treeSourceRect[3] = 94;			// Y2
	int treeRectWidth = 63;
	int treeRectHeight = 78;

	// Create default/starting fireCell object. Coordinates: 0,0 & 128,128
	UINT fireCell[4];
	fireCell[0] = 0;				// X1
	fireCell[1] = 0;				// Y1
	fireCell[2] = 128;				// X2
	fireCell[3] = 128;				// Y2
	int fireCell_Width = 128;
	int fireCell_Height = 128;

	// Set up Timer for Animation
	XTime timer;					// create timer object
	timer.Restart();

	// Create variables for use with timer
	double desiredFPS = 30.0;
	int count = 0;
	double threshold = 1.0 / desiredFPS;
	double elapsed = 0.0;

	// Loop until window shut down
	do
	{
		// signal timer
		timer.Signal();

		// BLIT Grass Tile over whole window as Background
		BLIT_WholeWindow(tiles_12_pixels, tiles_12_width, tiles_12_height, BackBuffer, RS_Width, RS_Height, grassSourceRect, grassRectWidth, grassRectHeight, 0, 0);

		// BLIT individual tree tiles over top of Grass Tile background (10 trees in random locations)
		for (int i = 0; i < 10; i++)
		{
			// find background grass color
			UINT clearColorCoords = ConvertCoordinate_2Dto1D(grassSourceRect[0], grassSourceRect[1], tiles_12_width, tiles_12_height);
			UINT colorForClear = ConvertBGRAtoARGB(tiles_12_pixels[clearColorCoords]);

			// Draw tree 
			BLIT_SingleTile(tiles_12_pixels, tiles_12_width, tiles_12_height, BackBuffer, RS_Width, RS_Height, treeSourceRect, treeRectWidth, treeRectHeight, randXLocations[i], randYLocations[i]);
		}
		
		// check elapsed time
		elapsed += timer.Delta();

		// Compare elapsed delta time with threshold
		if (elapsed > threshold)
		{
			// increment count
			count += 1;

			// clamp count within amount of tiles for fire animation
			if (count >= 64)
			{
				count = 0;
			}

			// reset timer
			elapsed -= threshold;
		}

		// Update attributes of fireCell, based on count
		fireCellSwitch(fireCell, count);

		// Draw fireCell
		BLIT_SingleTile(fire_02_pixels, fire_02_width, fire_02_height, BackBuffer, RS_Width, RS_Height, fireCell, fireCell_Width, fireCell_Height, 200, 200);

	} while (RS_Update(BackBuffer, RS_NumPixels));

	// End program
	RS_Shutdown();

	// Exit main
	return 0;
}

// Helper Fucntions
#pragma region Helper Functions
// Convert any BGRA value to an ARGB value
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

// Blend ARGB pixel colors using lerp
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

// Clear and Zero Out anything in the buffer
void ClearBuffer(UINT color)
{
	ClearBufferLocation(0, 0, RS_Width, RS_Height, color);
}

// Clear a specific section of the buffer
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

// Convert any 2D x,y coordinate in an array to a 1D coordinate
int ConvertCoordinate_2Dto1D(int x1, int y1, int arrayWidth, int arrayHeight)
{
	int coordinate_1D;

	// Convert x,y to 1D
	int tempY = arrayWidth * y1;
	coordinate_1D = tempY + x1;

	return coordinate_1D;
}

// check if converted color is fully transparent
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

// Takes in an x,y coordinate, converts to 1D coordinate, then assigns ARGB color at that location
void DrawPixel(int x1, int y1, UINT color)
{
	// find target coordinate
	int targetCoordinate = ConvertCoordinate_2Dto1D(x1, y1, RS_Width, RS_Height);

	// Make sure target pixel is within the bounds of the screen array
	if (targetCoordinate < RS_NumPixels)
	{
		// get target coordinate color
		UINT targetColor = BackBuffer[targetCoordinate];

		// lerp target color with incoming color
		UINT finalColor = AlphaBlend(targetColor, color);

		// add color to back buffer to be drawn
		BackBuffer[targetCoordinate] = finalColor;
	}
}

// BLIT function that takes in: 
// Pointer to Source Image Array, 
// Source Image Dimension (Width & Height), 
// Pointer to Destination Screen Array,
// Destination Screen Array Dimension (Width & Height),
// Source Image Sub-rectangle to be Copied (UINT array[4]: x1,y1 & x2,y2),
// Source Image Sub-rectangle dimensions (Width & Height)
// Destination Copy Location (X,Y Draw Coordinates).
void BLIT_SingleTile(const unsigned int sourceArray[], int sourceWidth, int sourceHeight, UINT destinationArray[], int destWidth, int destHeight, UINT sourceRect[], int sourceRectWidth, int sourceRectHeight, int destX1, int destY1)
{
	// Iterate over SourceRect
	for (int x = 0; x < sourceRectWidth; x++)
	{
		for (int y = 0; y < sourceRectHeight; y++)
		{
			// Calculate current coordinates within target rect
			UINT currCoords_1D = ConvertCoordinate_2Dto1D(sourceRect[0] + x, sourceRect[1] + y, sourceWidth, sourceHeight);

			// Copy target color
			UINT colorToCopyTemp = sourceArray[currCoords_1D];

			// Convert/Color Correct source color type to destination color type
			UINT colorToCopy = ConvertBGRAtoARGB(colorToCopyTemp);

			// Draw copied color to BackBuffer to be shown on screen
			DrawPixel(destX1 + x, destY1 + y, colorToCopy);
		}
	}
}

// Iterate over every tile in destination array and BLIT a source tile onto it
void BLIT_WholeWindow(const unsigned int sourceArray[], int sourceWidth, int sourceHeight, UINT destinationArray[], int destWidth, int destHeight, UINT sourceRect[], int sourceRectWidth, int sourceRectHeight, int destX1, int destY1)
{
	// Iterate over Destination screen by evenly divisible Tile Width and Tile Height
	for (int x = 0; x <= (destWidth / sourceRectWidth); x++)
	{
		for (int y = 0; y <= (destHeight / sourceRectHeight); y++)
		{
			// Draw Tile in correct rect location
			BLIT_SingleTile(sourceArray, sourceWidth, sourceHeight, destinationArray, destWidth, destHeight, sourceRect, sourceRectWidth, sourceRectHeight, destX1 + (x * 32), destY1 + (y * 32));
		}
	}
}

// update fireCell attributes
void fireCellSwitch(UINT fireCell[], int count)
{
	// declare temp variables
	int x1, y1, x2, y2;

	// Calculate Current Row
	int rowNum = count / 8;

	// Set Y Values
	y1 = 128 * rowNum;
	y2 = 128 * (rowNum + 1);

	// Calculate Current Column
	int columnNum = 0;

	if (count < 8)									// 0 - 7
	{
		columnNum = count;
	}
	else if (count >= 8 && count < 16)				// 8 - 15
	{
		int temp = count;

		temp = temp - 8;

		columnNum = temp;
	}
	else if (count >= 16 && count < 24)				// 16 - 23
	{
		int temp = count;

		temp = temp - 16;

		columnNum = temp;
	}
	else if (count >= 24 && count < 32)				// 24 - 31
	{
		int temp = count;

		temp = temp - 24;

		columnNum = temp;
	}
	else if (count >= 32 && count < 40)				// 32 - 39
	{
		int temp = count;

		temp = temp - 32;

		columnNum = temp;
	}
	else if (count >= 40 && count < 48)				// 40 - 47
	{
		int temp = count;

		temp = temp - 40;

		columnNum = temp;
	}
	else if (count >= 48 && count < 56)				// 48 - 55
	{
		int temp = count;

		temp = temp - 48;

		columnNum = temp;
	}
	else if (count >= 56 && count < 64)				// 56 - 63
	{
		int temp = count;

		temp = temp - 56;

		columnNum = temp;
	}

	// Set X Values
	x1 = 128 * columnNum;
	x2 = 128 * (columnNum + 1);
	
	// Apply fireCell Changes
	fireCell[0] = x1;				// X1
	fireCell[1] = y1;				// Y1

	fireCell[2] = x2;				// X2
	fireCell[3] = y2;				// Y2
}
#pragma endregion
