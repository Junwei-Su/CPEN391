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


#define RS232_Control (*(volatile unsigned char *)(0x84000220))
#define RS232_Status (*(volatile unsigned char *)(0x84000220))
#define RS232_TxData (*(volatile unsigned char *)(0x84000222))
#define RS232_RxData (*(volatile unsigned char *)(0x84000222))
#define RS232_Baud (*(volatile unsigned char *)(0x84000224))

/*
#define RS232_Control_wifi (*(volatile unsigned char *)(0x84000240))
#define RS232_Status_wifi (*(volatile unsigned char *)(0x84000240))
#define RS232_TxData_wifi (*(volatile unsigned char *)(0x84000242))
#define RS232_RxData_wifi (*(volatile unsigned char *)(0x84000242))
#define RS232_Baud_wifi (*(volatile unsigned char *)(0x84000244))
*/

void Init_RS232(void)
{
	//reference to  MC6850
 // set up 6850 Control Register to utilize a divide by 16 clock,

//16 clock => cr1= 0, cr0 =1

 // set RTS low, use 8 bits of data, no parity, 1 stop bit,
//=> cr4 = 1, cr3 =0, cr2 = 1

 // transmitter interrupt disabled
//=> CR6=0, CR5=0;

	//not sure if CR7 should be 1 or 0, let's try 1 first
	//CR= 10010110;
	RS232_Control = 0x15;

	 // program baud rate generator to use 115k baud
	RS232_Baud = 0x01;

}
int putcharRS232(int c)
{
 // poll Tx bit in 6850 status register. Wait for it to become '1'
	while(!(0x02&RS232_Status)){};
	//if Tx bit = 1, break out the while loop
 // write 'c' to the 6850 TxData register to output the character
	//int = 8 bits
	RS232_TxData = c;
 return c ; // return c
}
int getcharRS232( void )
{
 // poll Rx bit in 6850 status register. Wait for it to become '1'
	while(!(0x01&RS232_Status)){};
	//when Rx bit becomes 1, break out the while loop
 // read received character from 6850 RxData register.
	return (int) RS232_RxData;
}
// the following function polls the 6850 to determine if any character
// has been received. It doesn't wait for one, or read it, it simply tests
// to see if one is available to read
int RS232TestForReceivedData(void)
{
 // Test Rx bit in 6850 serial comms chip status register
 // if RX bit is set, return TRUE, otherwise return FALSE
	return 0x01&RS232_Status;
}

