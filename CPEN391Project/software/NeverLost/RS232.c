/*
 * RS232.c
 *
 *  Created on: Feb 14, 2017
 *      Author: Simon Jinaphant
 */

#define RS232_Control (*(volatile unsigned char *)(0x84000220))
#define RS232_Status (*(volatile unsigned char *)(0x84000220))
#define RS232_TxData (*(volatile unsigned char *)(0x84000222))
#define RS232_RxData (*(volatile unsigned char *)(0x84000222))
#define RS232_Baud (*(volatile unsigned char *)(0x84000224))

void rs232_init(void) {
	/**
	 * Reference to  MC6850
	 *
	 * Set up 6850 Control Register to utilize a divide by 16 clock,
	 * 16 clock => cr1= 0, cr0 =1
	 *
	 * Set RTS low, use 8 bits of data, no parity, 1 stop bit,
	 * => cr4 = 1, cr3 =0, cr2 = 1
	 *
	 * transmitter interrupt disabled
	 * => CR6=0, CR5=0;
	 *
	 */

	RS232_Control = 0x15;

	// Program baud rate generator to use 115k baud.
	RS232_Baud = 0x01;

}

/**
 * Send a string of data to the RS232 chip.
 */
void rs232_send_string(char command[]) {
	int i;

	for (i = 0; i < strlen(command); i++) {
		rs232_send_char(command[i]);
	}

	// Send the termination flags, without them the RS232 chip won't know when
	// the command ends and when the next command starts.
	rs232_send_char('\r');
	rs232_send_char('\n');
}

/**
 * Send a single character to the RS232 chip.
 */
int rs232_send_char(int char_data) {
	// Poll Tx bit in 6850 status register and await for it to become '1'
	while (!(0x02 & RS232_Status)) {
	};

	// Write the character to the 6850 TxData register.
	RS232_TxData = char_data;

	return char_data;
}

/*
 * Receive a string form the RS232 chip by polling until we reach a newline
 */
void rs232_print_response(){
	char response;
	while(response != '\n'){
		response = (char)rs232_get_char();
		printf("%c", response);
	}
	printf("\n");
}

/**
 * Receive a single char from the RS232 chip.
 */
int rs232_get_char(void) {
	// Poll RX bit in 6850 status register and await for it to become '1'
	while (!(0x01 & RS232_Status)) {
	};

	// Read the received character from 6850 RxData register.
	return (int) RS232_RxData;
}

/**
 * Polls the 6850 to determine if any character has been received.
 * It doesn't wait for one, or read it, it simply tests to see if one is available to read
 */
int rs232_check_if_received_data(void) {
	// Test Rx bit in the STATUS register
	// if RX bit is set, return TRUE, otherwise return FALSE
	return 0x01 & RS232_Status;
}


