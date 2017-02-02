/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include <stdio.h>
#include "Colours.h"
//#include "BresenhamsLineDrawingAlgorithm.h"

// #defined constants representing values we write to the graphics 'command' register to get
// it to do something. You will add more values as you add hardware to the graphics chip
// Note DrawHLine, DrawVLine and DrawLine at the moment do nothing - you will modify these

#define DrawHLine		1
#define DrawVLine		2
#define DrawLine		3
#define PutAPixel		0xA
#define GetAPixel		0xB
#define ProgramPaletteColour   	0x10

/*******************************************************************************************
** This macro pauses until the graphics chip status register indicates that it is idle
*******************************************************************************************/

#define WAIT_FOR_GRAPHICS		while((GraphicsStatusReg & 0x0001) != 0x0001);

//Predefined Colour Values
//Use the symbolic constants below as the values to write to the Colour Register
//When you ask the graphics chip to draw something. These values are also returned
//By the read pixel command

// the header file "Colours.h" contains symbolic names for all 256 colours e.g. RED
// while the source file ColourPaletteData.c contains the 24 bit RGB data
// that is pre-programmed into each of the 256 palettes

#define	BLACK		0
#define	WHITE		1
#define	RED			2
#define	LIME		3
#define	BLUE		4
#define	YELLOW		5
#define	CYAN		6
#define	MAGENTA		7


#define GraphicsCommandReg   		(*(volatile unsigned short int *)(0x84000000))
#define GraphicsStatusReg   		(*(volatile unsigned short int *)(0x84000000))
#define GraphicsX1Reg   		(*(volatile unsigned short int *)(0x84000002))
#define GraphicsY1Reg   		(*(volatile unsigned short int *)(0x84000004))
#define GraphicsX2Reg   		(*(volatile unsigned short int *)(0x84000006))
#define GraphicsY2Reg			(*(volatile unsigned short int *)(0x84000008))
#define GraphicsColourReg		(*(volatile unsigned short int *)(0x8400000E))
#define GraphicsBackGroundColourReg   		(*(volatile unsigned short int *)(0x84000010))

/**********************************************************************
* This function writes a single pixel to the x,y coords specified in the specified colour
* Note colour is a palette number (0-255) not a 24 bit RGB value
**********************************************************************/
void WriteAPixel (int x, int y, int Colour)
{
	WAIT_FOR_GRAPHICS;			// is graphics ready for new command

	GraphicsX1Reg = x;			// write coords to x1, y1
	GraphicsY1Reg = y;
	GraphicsColourReg = Colour;		// set pixel colour with a palette number
	GraphicsCommandReg = PutAPixel;		// give graphics a "write pixel" command
}

/*****************************************************************************************
* This function read a single pixel from x,y coords specified and returns its colour
* Note returned colour is a palette number (0-255) not a 24 bit RGB value
******************************************************************************************/
int ReadAPixel (int x, int y)
{
	WAIT_FOR_GRAPHICS;			// is graphics ready for new command

	GraphicsX1Reg = x;			// write coords to x1, y1
	GraphicsY1Reg = y;
	GraphicsCommandReg = GetAPixel;		// give graphics a "get pixel" command

	WAIT_FOR_GRAPHICS;			// is graphics done reading pixel
	return (int)(GraphicsColourReg) ;		// return the palette number (colour)
}

/****************************************************************************************************
** subroutine to program a hardware (graphics chip) palette number with an RGB value
** e.g. ProgramPalette(RED, 0x00FF0000) ;
****************************************************************************************************/

void ProgramPalette(int PaletteNumber, int RGB)
{
    WAIT_FOR_GRAPHICS;
    GraphicsColourReg = PaletteNumber;
    GraphicsX1Reg = RGB >> 16   ;          // program red value in ls.8 bit of X1 reg
    GraphicsY1Reg = RGB ;                	 // program green and blue into 16 bit of Y1 reg
    GraphicsCommandReg = ProgramPaletteColour;	// issue command
}

void DrawHline(int x1, int y1, int x2, int y2, int color)
{
    WAIT_FOR_GRAPHICS;

	GraphicsX1Reg = x1;			// write coords to x1, y1
	GraphicsY1Reg = y1;

	GraphicsX2Reg = x2;			// write coords to x1, y1
	GraphicsY2Reg = y2;

	GraphicsColourReg = color;		// set pixel colour with a palette number
	GraphicsCommandReg = DrawHLine;		// give graphics a "write pixel" command
}

void DrawVline(int x1, int y1, int x2, int y2, int color)
{
    WAIT_FOR_GRAPHICS;

	GraphicsX1Reg = x1;			// write coords to x1, y1
	GraphicsY1Reg = y1;

	GraphicsX2Reg = x2;			// write coords to x1, y1
	GraphicsY2Reg = y2;

	GraphicsColourReg = color;		// set pixel colour with a palette number
	GraphicsCommandReg = DrawVLine;		// give graphics a "write pixel" command
}

void Drawline(int x1, int y1, int x2, int y2, int color)
{
    WAIT_FOR_GRAPHICS;

	GraphicsX1Reg = x1;			// write coords to x1, y1
	GraphicsY1Reg = y1;

	GraphicsX2Reg = x2;			// write coords to x1, y1
	GraphicsY2Reg = y2;

	GraphicsColourReg = color;		// set pixel colour with a palette number
	GraphicsCommandReg = DrawLine;		// give graphics a "write pixel" command
}

int main()
{
	int i ;
//	printf("Hello from Nios II!\n");
//
	//draw a line across the screen in RED at y coord 100 and from x = 0 to 799
//	for(i = 0; i < 800; i ++)
//		WriteAPixel(i, 100, RED);
//
//	// read the pixels back and make sure we read 2 (RED) to prove it's working
//	for(i = 0; i < 800; i ++)
//		printf("Colour value (i.e. pallette number) = %d at [%d, 100]\n", ReadAPixel(i, 100),i);

	DrawHline(20,200,220,200, RED);
	DrawHline(20,220,220,220, GREEN);
	DrawVline(20,200,20,400, GREEN);
	Drawline(20,20,220,40, GREEN);
	printf("Colour value (i.e. pallette number) = %d at [%d, 100]\n", ReadAPixel(100, 20),20);
	printf("Colour value (i.e. pallette number) = %d at [%d, 100]\n", ReadAPixel(20, 20),20);

	return 0;
}
