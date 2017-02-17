#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RS232_Control_HR (*(volatile unsigned char *)(0x84000250))
#define RS232_Status_HR (*(volatile unsigned char *)(0x84000250))
#define RS232_TxData_HR (*(volatile unsigned char *)(0x84000252))
#define RS232_RxData_HR (*(volatile unsigned char *)(0x84000252))
#define RS232_Baud_HR (*(volatile unsigned char *)(0x84000254))

//initialize
void initRS232HR(void) {
    //8 data bits, 1 stop bit
    RS232_Control_HR = 0x15;

    //9600 baud
    RS232_Baud_HR = 0x05;
}

//read from rx
char getCharRS232HR(void) {
    while (!(RS232_Status_HR & 0x01)) {
    };
    return RS232_RxData_HR;
}

//write to tx
void putCharRS232HR(int c) {
    while (!(0x02 & RS232_Status_HR)) {
    };
    RS232_TxData_HR = c;
}

void waitChar(int i){
    int j;
    for (j = 0; j < i; j++)
        getCharRS232HR();
}

int getHRData(){
    int i;
    int j;
    do{
        i = getHR();
        j = getHR();
    }
    while(i != j);
}

int getHR(){
    char HR_MSB;
    char HR_LSB;
    while (1){
        char s = getCharRS232HR();
        if ((!(s&0xFE))&(s&0x01)){
            if (getCharRS232HR()&0x01){
                waitChar(1);
                HR_MSB = getCharRS232HR();
                waitChar(4);
                HR_LSB = getCharRS232HR();

                int HR = (0x7F&HR_LSB)+((0x03&HR_MSB)<<7);

                return HR;
            }
        }
    }
}
