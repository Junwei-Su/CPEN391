/*
#include <stdio.h>
#include <string.h>
#include "wifi_serial.h"

void putString(char stringp[]);

char initialize[] = "dofile(\"wifi.lua\")";

char theMessage[] = "Your person is safe";

void putString(char string[]){
	int i;

	for (i = 0; i < strlen(string); i++){
	putcharRS232(string[i]);
	}
	putcharRS232('\r');
	putcharRS232('\n');
}

void message(char string[]){
	char str[256];

	sprintf(str,"send_sms(\"(778) 653-7744\",\"(604) 783-0303\",\"%s\")",string);
	putString(str);
}

*/


#ifndef WIFI_H_
#define WIFI_H_

#include <stdio.h>
#include <string.h>
#include "RS232.h"


// The message to send via SMS when Dependent is safe.
char WIFI_MESSAGE_DEPENDENT_IS_SAFE[] =
		"The dependent has confirm their safety.";

void wifi_init();
void wifi_send_sms(char message[]);

#endif /* WIFI_H_ */
