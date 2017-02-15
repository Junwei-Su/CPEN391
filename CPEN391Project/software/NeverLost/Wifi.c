/*
 * Wifi.c
 *
 *  Created on: Feb 14, 2017
 *      Author: Simon Jinaphant
 */

/**
 * Initialize the Wifi module.
 * Requires initialization of the RS232 chip beforehand!
 */
void wifi_init() {
	// Instruct RS232 chip to initializes the onboard .lua wifi script.
	rs232_send_string("dofile(\"send_text_message.lua\")");
}

/**
 * Send a SMS message to the Guardian.
 *
 * @param message - The message to send via SMS; make sure this is no longer than ~180 chars
 */
void wifi_send_sms(char message[]) {
	char lua_sms_command[256];

	sprintf(lua_sms_command,
			"send_sms(\"(778) 653-7744\",\"(604) 783-0303\",\"%s\")", message);

	rs232_send_string(lua_sms_command);
	rs232_print_response();
}
