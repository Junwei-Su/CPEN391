#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "GPS_plot.h"
#include "healthSensor.h"
#include "wifi.h"


//array that store heart rate data
int HR_MAX = 20;
int hr_index;
int HR_data[20];
int HR_plotdata[20];
int Average;
int SD;
int historical_average=70;
int historical_SD = 10;


void drawXaxis(){
	DrawHlineWeighted(280,240,750, 245,BLUE);
}

void drawYaxis(){
	DrawVlineWeighted(280,60,285, 400,GREEN);
}


void graph_hr(){
	drawLabel();
	drawXaxis();
	drawYaxis();
	int i;
	for(i=0; i<HR_MAX-1; i++){
		HR_plotdata[i] = 240 + (HR_data[i] - Average)*25;
	}

	for(i=0; i<HR_MAX-1; i++){
		Drawline(hr_calx(i+1),HR_plotdata[i+1],hr_calx(i),HR_plotdata[i], RED);
	}
}

void graph_hr_wait(){
	drawXaxis();
	drawYaxis();
	write(250,35, "PLEASE WAIT WE ARE MEASURING", 16, WHITE, BLACK);
	write(250,65, "YOUR HEART RATE", 16, WHITE, BLACK);
	write(50,50, "Average: ", 16, WHITE, BLACK);
	write(50,150, "Fluctuation: ", 16, WHITE, BLACK);
	write(675,265, "Mean", 16, WHITE, BLACK);
}

int hr_calx(int i){
	return i*30+280;
}

void fresh_hr(){

	hr_index=0;
	int i;
	for(i=0; i<HR_MAX; i++){
		HR_data[i] = 0;
	}

}


void init_hr(){

	hr_index=0;
	int i;
	for(i=0; i<HR_MAX; i++){
		HR_data[i] = getHRData();
	}

	computeAve();
	computeSD();
	update_HistAVE();

	if(abs(Average-historical_average)>= 2*historical_SD) {
		alert();
	}
}


void drawLabel(){

	char aver[20];
	char fluct[20];
	itoa(Average, aver);
	itoa(SD, fluct);
	write(450,25, "Heart Rate", 16, WHITE, BLACK);
	write(50,50, "Average: ", 16, WHITE, BLACK);
	write(80,90, aver, 16, WHITE, BLACK);
	write(50,150, "Fluctuation: ", 16, WHITE, BLACK);
	write(80,190, fluct, 16, WHITE, BLACK);
	write(675,265, "Mean", 16, WHITE, BLACK);
}

void computeAve(){
	int sum=0;
	int i;
	for(i=0; i<HR_MAX; i++){
		sum += HR_data[i];
	}

	Average = sum/HR_MAX;
}

void computeSD(){
	float sum=0;;
	int i;
	for(i=0; i<HR_MAX; i++){
		sum += pow((HR_data[i]-Average),2);
	}

	SD = floor(pow(sum,0.5));
}

void alert(){
	wifi_send_sms("Your dependent is showing abnormal health conditions!");
}

void update_HistAVE(){
	historical_average = (Average + historical_average)/2;
}

void update_HistSD(){
	historical_SD = (historical_SD + SD)/2;
}

void healthB(){

	drawSolidcircle(120, 350, 80, GREEN);
	write(95, 330, "GET", 16, WHITE, GREEN);
	write(65, 360, "READING", 16, WHITE, GREEN);


}

