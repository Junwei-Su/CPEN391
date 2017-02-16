#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RS232_Control_gps (*(volatile unsigned char *)(0x84000210))
#define RS232_Status_gps (*(volatile unsigned char *)(0x84000210))
#define RS232_TxData_gps (*(volatile unsigned char *)(0x84000212))
#define RS232_RxData_gps (*(volatile unsigned char *)(0x84000212))
#define RS232_Baud_gps (*(volatile unsigned char *)(0x84000214))


typedef struct { char* lat; char* lng; } Coord ;

//initialize
void initRS232GPS(void) {
	//8 data bits, 1 stop bit
	RS232_Control_gps = 0x15;

	//9600 baud
	RS232_Baud_gps = 0x05;
}

//write to tx
char getCharRS232GPS(void) {
	while (!(RS232_Status_gps & 0x01)) {
	};
	return RS232_RxData_gps;
}

//read from rx
void putCharRS232GPS(int c) {
	while (!(0x02 & RS232_Status_gps)) {
	};
	RS232_TxData_gps = c;
}

Coord getGPSdata() {
	int size = 500;
	Coord c;
	while (1) {
		char data[size];
		char character;
		int index = 0;
		const char s[2] = ",";
		//get data string
		do {
			character = getCharRS232GPS();
			printf("%c", character);
			data[index] = character;
			index++;
		} while (character != '\n');

		char *token = strtok(data, s);

		if (!strcmp(token, "$GPGGA")) {
			printf("message_id: %s \n", token);

			token = strtok(NULL, s);
			double UTC_time = atof(token);
			printf("UTC_time: %f \n", UTC_time);

			token = strtok(NULL, s);
			char* latitude = token;
			printf("latitude: %s \n", latitude);

			token = strtok(NULL, s);
			token = strtok(NULL, s);
			char* longitude = token;
			printf("longitude: %s \n", longitude);

			c.lat = latitude;
			c.lng = longitude;
			return c;
		}
	}
	return c;
}


char* concat(const char *s1, const char *s2, const char *s3, const char *s4){
    char *result = malloc(strlen(s1)+strlen(s2)+strlen(s3)+strlen(s4)+1);
    strcpy(result, s1);
    strcat(result, s2);
    strcat(result, s3);
    strcat(result, s4);
    return result;
}
