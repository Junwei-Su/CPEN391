/*
 * RS232.h
 *
 *  Created on: Feb 2, 2017
 *      Author: Simon Jinaphant
 */

#ifndef RS232_H_
#define RS232_H_


void rs232_init(void);
void rs232_send_string(char command[]);
int rs232_send_char(int char_data);
void rs232_print_response();
int rs232_get_char(void);
int rs232_check_if_received_data(void);

#endif /* RS232_H_ */
