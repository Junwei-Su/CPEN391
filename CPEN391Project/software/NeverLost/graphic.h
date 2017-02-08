
#include <stdio.h>
#include "Colours.h"
#include "OutGraphicsCharFont1.h"
#include "OutGraphicsCharFont2.h"
#include "touchScreen.h"
#include "wifi.h"

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

void DrawHlineWeighted(int x1, int y1, int x2, int y2, int color){
	int y;
	for (y = y1; y <= y2; y++){
		DrawHline(x1, y, x2, y, color);
	}
}

void DrawVlineWeighted(int x1, int y1, int x2, int y2, int color){
	int x;
	for (x = x1; x <= x2; x++){
		DrawVline(x, y1, x, y2, color);
	}
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

void DrawSolidRectangle(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int color){
    int i;
    for(i=0; i<y3-y1; i++){
        DrawHline(x1,y1+i,x2,y2+i,color);
    }
}

void DrawRectangle(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int color){
    DrawHline(x1,y1,x2,y2,color);
    DrawHline(x3,y3,x4,y4,color);
    DrawVline(x1,y1,x3,y3,color);
    DrawVline(x2,y2,x4,y4,color);
}

void waitFor (unsigned int secs) {
    unsigned int retTime = time(0) + secs;
    while (time(0) < retTime);
}

void clearScreen(){
	DrawHlineWeighted(0, 0, 800, 480, BLACK);
}

void write(int x, int y, char *string, int spacing, int color, int backgroundcolor){
	int i;
	for (i = 0; i < strlen(string); i++){
		OutGraphicsCharFont2a(x+(i*spacing), y, color, backgroundcolor, string[i], 1);
	}
}

void drawBox1(int colour){
	int border = 10;
	int x_s = 150;
	int y_s = 150;
	int b_x1 = 167;
	int b_y1 = 100;
	int line_weight = 6;

	DrawRectangle(b_x1, b_y1, b_x1+x_s, b_y1, b_x1, b_y1+y_s, b_x1+x_s, b_y1+y_s, WHITE);
	DrawSolidRectangle(b_x1+border, b_y1+border, b_x1+x_s-border, b_y1+border, b_x1+border, b_y1+y_s-border, b_x1+x_s-border, b_y1+y_s-border, colour);

	DrawVlineWeighted(b_x1+((x_s-line_weight)/2), b_y1+border*2, b_x1+((x_s-line_weight)/2)+line_weight, y_s+b_y1-border*2, WHITE);
	DrawHlineWeighted(b_x1+border*2, b_y1+((y_s-line_weight)/2), x_s+b_x1-border*2, b_y1+((y_s-line_weight)/2)+line_weight, WHITE);

	write(b_x1+border*2, b_y1+y_s+border*3, "Respond", 16, WHITE, BLACK);
}

void drawBox2(int colour){
	int border = 10;
	int x_s = 150;
	int y_s = 150;
	int b_x2 = 483;
	int b_y2 = 100;
	int line_weight = 6;

	DrawRectangle(b_x2, b_y2, b_x2+x_s, b_y2, b_x2, b_y2+y_s, b_x2+x_s, b_y2+y_s, WHITE);
	DrawSolidRectangle(b_x2+border, b_y2+border, b_x2+x_s-border, b_y2+border, b_x2+border, b_y2+y_s-border, b_x2+x_s-border, b_y2+y_s-border, colour);

	DrawVlineWeighted(b_x2+((x_s-line_weight)/2), b_y2+border*2, b_x2+((x_s-line_weight)/2)+line_weight, y_s+b_y2-border*2, WHITE);
	DrawVlineWeighted(b_x2+((x_s-line_weight)/2)-border*3, b_y2+border*2, b_x2+((x_s-line_weight)/2)+line_weight-border*3, y_s+b_y2-border*2, WHITE);
	DrawVlineWeighted(b_x2+((x_s-line_weight)/2)+border*3, b_y2+border*2, b_x2+((x_s-line_weight)/2)+line_weight+border*3, y_s+b_y2-border*2, WHITE);

	write(b_x2+5.5*border, b_y2+y_s+border*3, "GPS", 16, WHITE, BLACK);
}

int touchBox(Point p, int size, int x, int y){
	if (p.x > x && p.x < x+size && p.y > y && p.y < y+size)
		return 1;
	else
		return 0;
}

void drawcircle(int x0, int y0, int radius, int colour)
{
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y)
    {
    	WriteAPixel(x0 + x, y0 + y, colour);
    	WriteAPixel(x0 + y, y0 + x, colour);
    	WriteAPixel(x0 - y, y0 + x, colour);
    	WriteAPixel(x0 - x, y0 + y, colour);
    	WriteAPixel(x0 - x, y0 - y, colour);
    	WriteAPixel(x0 - y, y0 - x, colour);
    	WriteAPixel(x0 + y, y0 - x, colour);
    	WriteAPixel(x0 + x, y0 - y, colour);

        if (err <= 0)
        {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0)
        {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}
