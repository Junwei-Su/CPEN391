#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RS232_Control (*(volatile unsigned char *)(0x84000230))
#define RS232_Status (*(volatile unsigned char *)(0x84000230))
#define RS232_TxData (*(volatile unsigned char *)(0x84000232))
#define RS232_RxData (*(volatile unsigned char *)(0x84000232))
#define RS232_Baud (*(volatile unsigned char *)( 0x84000234))


/* a data type to hold a point/coord */
typedef struct { int x, y; } Point ;

//initialize
void initRS232Touch(void){
	//8 data bits, 1 stop bit
	RS232_Control = 0x15;

	//9600 baud
	RS232_Baud = 0x05;
}

//write to tx
char getCharRS232Touch( void ){
	while(!(RS232_Status&0x01)){};
	return RS232_RxData;
}

//read from rx
void putCharRS232Touch(int c){
	while(!(0x02&RS232_Status)){};
	RS232_TxData = c;
}

//initalize touchscreen
void initTouchScreen(void){
	putCharRS232Touch(0x55);
    putCharRS232Touch(0x01);
	putCharRS232Touch(0x12);
	int i;
	for (i = 0; i < 4; i++){
		char c = getCharRS232Touch();
	}
}

//wait for touch
void waitForTouch(){
	while(!screenTouched());
}

//wait for release
void waitForRelease(){
     while(!screenReleased());
}

//test if touched
int screenTouched( void ){
	char c = getCharRS232Touch();
	return (c&0x01)&(c>>7);
}

//test if released
int screenReleased( void ){
	char c = getCharRS232Touch();
	return (!(c&0x01))&(c>>7);
}

//forces point into set range
int check_bounds(int a, int b){
	if (a < 0)
		return 0;
	else if (a > b)
		return b;
	else
		return a;
}

//get the touched/release point and map it
Point getPoint(){

	Point p1;

	//get x and y coordinate from next 4 bytes
	int x_coord = 0x7F&getCharRS232Touch();
	x_coord = x_coord + ((0x1F&getCharRS232Touch())<<7);

	int y_coord = 0x7F&getCharRS232Touch();
	y_coord = y_coord + ((0x1F&getCharRS232Touch())<<7);

	printf("x_coord: %i\n", x_coord);
	printf("y_coord: %i\n\n", y_coord);

	//map touched coordinates to pixels on display
	//mapping x from 200-3800 to 0-800 range
	//mapping y from 450-3800 to 0-480 range

	int mapped_x_coord = check_bounds((x_coord-200)/4.5, 800);
	int mapped_y_coord = check_bounds((y_coord-450)/7, 480);

	printf("mapped_x_coord: %i\n", mapped_x_coord);
	printf("mapped_y_coord: %i\n\n", mapped_y_coord);

	p1.x = mapped_x_coord;
	p1.y = mapped_y_coord;

 return p1;
}

long getTime(){
	  time_t seconds;
	  seconds = time (NULL);
	  return seconds;
}

Point getPressTime(int time){
	long currentTime = getTime();
	while (getTime()-currentTime < time){
		char c = getCharRS232Touch();
		if((c&0x01)&(c>>7)){
			return getPoint();
		}
	}
	Point p;
	p.x = -1;
	p.y = -1;
	return p;
}

//waits for a touch screen press event and returns X,Y coord
Point getPress(void){
	waitForTouch();
	return getPoint();
}

//waits for a touch screen release event and returns X,Y coord
Point getRelease(void){
	waitForRelease();
	return getPoint();
}
