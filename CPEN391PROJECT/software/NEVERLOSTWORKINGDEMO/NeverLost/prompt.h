#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void prompt(){

	clearScreen(LIGHT_CYAN);
	int i;
	for (i = 1; i < 5; i++){
		keyPad(400,100*i-50);
		keyPad(500,100*i-50);
		keyPad(600,100*i-50);

	}

	write(450, 100, "1", 16, WHITE, GRAY);
	write(550, 100, "2", 16, WHITE, GRAY);
	write(650, 100, "3", 16, WHITE, GRAY);

	write(450, 200, "4", 16, WHITE, GRAY);
	write(550, 200, "5", 16, WHITE, GRAY);
	write(650, 200, "6", 16, WHITE, GRAY);

	write(450, 300, "7", 16, WHITE, GRAY);
	write(550, 300, "8", 16, WHITE, GRAY);
	write(650, 300, "9", 16, WHITE, GRAY);

	write(450, 400, "OK", 16, GREEN, GRAY);
	write(550, 400, "0", 16, WHITE, GRAY);
	write(650, 400, "<-", 16, RED, GRAY);

	int x1 = 50;
	int y1 = 150;
	int boxW = 300;
	int boxH = 50;

	DrawSolidRectangle(x1, y1, x1+boxW, y1, x1, y1+boxH, x1+boxW, y1+boxH, WHITE);

	write(50, 10, "Are you Safe?", 16, GRAY, LIGHT_CYAN);
	write(50, 30, "Please enter your pin if you are!", 16, GRAY, LIGHT_CYAN);
}

void keyPad(int x1, int y1){

	int boxW = 100;
	int boxH = 100;

	DrawSolidRectangle(x1, y1, x1+boxW, y1, x1, y1+boxH, x1+boxW, y1+boxH, GRAY);
	DrawRectangle(x1, y1, x1+boxW, y1, x1, y1+boxH, x1+boxW, y1+boxH, LIGHT_SLATE_GRAY);

}

void space(int x1, int y1) {

	int boxW = 10;
	int boxH = 20;

	DrawSolidRectangle(x1, y1, x1+boxW, y1, x1, y1+boxH, x1+boxW, y1+boxH, WHITE);

}

void space2(int x1, int y1) {

	int boxW = 70;
	int boxH = 30;

	DrawSolidRectangle(x1, y1, x1+boxW, y1, x1, y1+boxH, x1+boxW, y1+boxH, LIGHT_CYAN);

}

void append(char*s, char c) {
     int len = strlen(s);
     s[len] = c;
     s[len+1] = '\0';
}

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    //in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

