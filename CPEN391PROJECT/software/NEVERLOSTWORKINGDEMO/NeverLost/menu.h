


//menu constants
#define BOXW 150;
#define BOXH 150;

void menuBox(int x1, int y1) {

	int boxW = BOXW;
	int boxH = BOXH;

	DrawSolidRectangle(x1, y1, x1+boxW, y1, x1, y1+boxH, x1+boxW, y1+boxH, GRAY);
	DrawRectangle(x1, y1, x1+boxW, y1, x1, y1+boxH, x1+boxW, y1+boxH, LIGHT_SLATE_GRAY);

}


void symbolGPS(int xCentre, int yCentre, int rad){

	int colour = STEEL_BLUE;

	drawcircle(xCentre, yCentre, rad, colour);
	drawSolidcircle(xCentre, yCentre, rad/2, colour);

	DrawVline(xCentre, yCentre + rad - 10, xCentre, yCentre + rad + 10, colour);
	DrawVline(xCentre, yCentre - rad - 10, xCentre, yCentre - rad + 10, colour);
	DrawHline(xCentre + rad - 10, yCentre, xCentre + rad + 10, yCentre, colour);
	DrawHline(xCentre - rad - 10, yCentre, xCentre - rad + 10, yCentre, colour);


}

void symbolHELP(int x1, int y1) {

	int size = 75;

	int i;
	for(i = 0; i<7; i++){
	DrawVline(x1+i, y1, x1, y1+size, RED);
	DrawVline(x1-i, y1, x1, y1+size, RED);
	}

	drawSolidcircle(x1, y1+size+20, 7, RED);

}

void cross(int xc, int yc) {

	int size = 100;


	int xLeft = xc - 50;
	int yTop = yc - 50;

	int i;
	for(i = 0; i<15; i++){
		DrawVline(xc+i, yTop, xc+i, yTop+size, LIME);
		DrawVline(xc-i, yTop, xc-i, yTop+size, LIME);
	}

	yc -= 17;

	for(i = 0; i<30; i++){
		DrawHline(xLeft, yc+i, xLeft+size, yc+i, LIME);
		DrawHline(xLeft, yc-1, xLeft+size, yc-i, LIME);
	}
}


void menu(){

	clearScreen(LIGHT_CYAN);

	write(325, 50, "NeverLost", 16, SLATE_GRAY, LIGHT_CYAN);

	DrawSolidRectangle(0, 120, 800, 120, 0, 440, 800, 440, MEDIUM_SPRING_GREEN);
	DrawRectangle(0, 120, 800, 120, 0, 440, 800, 440, BLUE);


	menuBox(100,205);
	writeSmall(135,375,"DANGER",16, SLATE_GRAY, MEDIUM_SPRING_GREEN);
	symbolHELP(175, 230);
	menuBox(325,205);
	writeSmall(385,375,"GPS",16, SLATE_GRAY, MEDIUM_SPRING_GREEN);
	symbolGPS(400,280,50);
	menuBox(550,205);
	writeSmall(585,375,"HEALTH",16, SLATE_GRAY, MEDIUM_SPRING_GREEN);
	cross(625, 280);
}


void exitB(){

	drawSolidcircle(760, 20, 20, RED);
	write(755, 13, "X", 16, BLACK, RED);
}
