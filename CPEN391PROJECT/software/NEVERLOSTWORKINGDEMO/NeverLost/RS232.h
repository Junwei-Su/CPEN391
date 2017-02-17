/*
 * initialize_RS232.c
 *
 *  Created on: Jan 21, 2017
 *      Author: Little_town
 */

/**************************************************************************
** Subroutine to initialize the RS232 Port by writing some data
** to the internal registers.
** Call this function at the start of the program before you attempt
** to read or write to data via the RS232 port
**
** Refer to 6850 data sheet for details of registers and
***************************************************************************/


#ifndef RS232_H_
#define RS232_H_

void rs232_init(void);
void rs232_send_string(char command[]);
int rs232_send_char(int char_data);
void rs232_print_response();
int rs232_get_char(void);
int rs232_check_if_received_data(void);


#endif /* RS232_H_ */
