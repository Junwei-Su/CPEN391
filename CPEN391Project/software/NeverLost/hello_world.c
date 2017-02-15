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
#include "RS232.h"
#include "Wifi.h"

int main()
{
	printf("Initializing...\n");
	rs232_init();
	wifi_init();

	printf("Sending message\n");
	wifi_send_sms("Hello Aaron.");

	printf("Done\n");
	return 0;
}

