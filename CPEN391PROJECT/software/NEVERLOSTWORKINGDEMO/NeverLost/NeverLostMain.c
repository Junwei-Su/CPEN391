/*
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
#include <stdlib.h>
#include <string.h>
#include "Colours.h"
#include "graphic.h"
#include "prompt.h"
#include "Bio_plot.h"
#include "menu.h"


int main()
{
	initRS232Touch();
	initTouchScreen();
	initRS232HR();
	initRS232GPS();
	rs232_init();
	wifi_init();
	//wifi_send_sms("test");

	clearScreen(LIGHT_CYAN);


	menu:

	menu();
	while(1){

		Point p = getPressTime(100);
		if (touchBox(p, 150, 100, 205)){
			goto prompt;
		}
		if (touchBox(p, 150, 325, 205)){
			goto gps;
		}
		if (touchBox(p, 150, 550, 205)){
			goto health;
		}
	}

	prompt:

	prompt();
	int Spass = 8523;
	int Dpass = 0000;
	char *pin = "";
	int spacer=1;
	int numLine = 170;

	while(1){
		Point p = getPressTime(100);
		if(spacer < 10){
			if (touchBox(p, 100, 400, 50)){
				write(50+10*spacer, numLine, "1", 16, GRAY, WHITE);
				spacer++;
				pin = concat(pin,"1");
				waitFor(1);

			}
			if (touchBox(p, 100, 500, 50)){
				write(50+10*spacer, numLine, "2", 16, GRAY, WHITE);
				spacer++;
				pin = concat(pin,"2");
				waitFor(1);
			}
			if (touchBox(p, 100, 600, 50)){
				write(50+10*spacer, numLine, "3", 16, GRAY, WHITE);
				spacer++;
				pin = concat(pin,"3");
				waitFor(1);
			}
			if (touchBox(p, 100, 400, 150)){
				write(50+10*spacer, numLine, "4", 16, GRAY, WHITE);
				spacer++;
				pin = concat(pin,"4");
				waitFor(1);
			}
			if (touchBox(p, 100, 500, 150)){
				write(50+10*spacer, numLine, "5", 16, GRAY, WHITE);
				spacer++;
				pin = concat(pin,"5");
				waitFor(1);
			}
			if (touchBox(p, 100, 600, 150)){
				write(50+10*spacer, numLine, "6", 16, GRAY, WHITE);
				spacer++;
				pin = concat(pin,"6");
				waitFor(1);
			}
			if (touchBox(p, 100, 400, 250)){
				write(50+10*spacer, numLine, "7", 16, GRAY, WHITE);
				spacer++;
				pin = concat(pin,"7");
				waitFor(1);
			}
			if (touchBox(p, 100, 500, 250)){
				write(50+10*spacer, numLine, "8", 16, GRAY, WHITE);
				spacer++;
				pin = concat(pin,"8");
				waitFor(1);
			}
			if (touchBox(p, 100, 600, 250)){
				write(50+10*spacer, numLine, "9", 16, GRAY, WHITE);
				spacer++;
				pin = concat(pin,"9");
				waitFor(1);
			}
			if (touchBox(p, 100, 500, 350)){
				write(50+10*spacer, numLine, "0", 16, GRAY, WHITE);
				spacer++;
				pin = concat(pin,"0");
				waitFor(1);
			}
		}
		//backspace
		if (touchBox(p, 100, 600, 350)){
			if(spacer > 1){
				spacer--;
				space(50+10*spacer, numLine);
				pin[strlen(pin)-1] = 0;
			}
			waitFor(1);
		}

		//ok
		if (touchBox(p, 100, 400, 350)){
			int ipin = atoi(pin);
			if (ipin == Spass){
				space2(150, 250);
				write(50, 250, "CORRECT", 16, GRAY, LIGHT_CYAN);
				waitFor(2);
				goto menu;
			}
			else if (ipin == Dpass){
				write(50, 250, "Help is on the way!", 16, GRAY, LIGHT_CYAN);
				wifi_send_sms("Your dependent needs your help!");
				waitFor(2);
				goto menu;
			}
			else{
				write(50, 250, "INCORRECT", 16, GRAY, LIGHT_CYAN);
			}
			waitFor(1);
		}

	}

	gps:
	gps();
	exitB();
	while(1){
		Point p = getPressTime(100);
		if(touchBox(p,100,790,5)){
			goto menu;
		}
		if(touchBox(p,150,50,300)){
			goto gps;
		}
	}

	health:
	clearScreen(BLACK);
	graph_hr_wait();
	init_hr();
	clearScreen(BLACK);
	graph_hr();
	exitB();
	healthB();
	while(1){
		Point p = getPressTime(100);
		if(touchBox(p,100,790,5)){
			goto menu;
		}
		if(touchBox(p,150,50,300)){
			goto health;
		}

	}

	return 0;
}