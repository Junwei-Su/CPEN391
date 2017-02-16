/*
 * initialize_RS232.c
 *
 *  Created on: Jan 21, 2017
 *      Author: Little_town
 */

/**************************************************************************
/* Subroutine to initialize the RS232 Port by writing some data
** to the internal registers.
** Call this function at the start of the program before you attempt
** to read or write to data via the RS232 port
**
** Refer to 6850 data sheet for details of registers and
***************************************************************************/


#define RS232_Control_wifi (*(volatile unsigned char *)(0x84000220))
#define RS232_Status_wifi (*(volatile unsigned char *)(0x84000220))
#define RS232_TxData_wifi (*(volatile unsigned char *)(0x84000222))
#define RS232_RxData_wifi (*(volatile unsigned char *)(0x84000222))
#define RS232_Baud_wifi (*(volatile unsigned char *)(0x84000224))


void Init_RS232WIFI(void)
{
	RS232_Control_wifi = 0x15;

	 // program baud rate generator to use 115k baud
	RS232_Baud_wifi = 0x01;

}
int putcharRS232WIFI(int c)
{
 // poll Tx bit in 6850 status register. Wait for it to become '1'
	while(!(0x02&RS232_Status_wifi)){};
	//if Tx bit = 1, break out the while loop
 // write 'c' to the 6850 TxData register to output the character
	//int = 8 bits
	RS232_TxData_wifi = c;
 return c ; // return c
}
int getcharRS232WIFI( void )
{
 // poll Rx bit in 6850 status register. Wait for it to become '1'
	while(!(0x01&RS232_Status_wifi)){};
	//when Rx bit becomes 1, break out the while loop
 // read received character from 6850 RxData register.
	return (int) RS232_RxData_wifi;
}
// the following function polls the 6850 to determine if any character
// has been received. It doesn't wait for one, or read it, it simply tests
// to see if one is available to read
int RS232TestForReceivedDataWIFI(void)
{
 // Test Rx bit in 6850 serial comms chip status register
 // if RX bit is set, return TRUE, otherwise return FALSE
	return 0x01&RS232_Status_wifi;
}


