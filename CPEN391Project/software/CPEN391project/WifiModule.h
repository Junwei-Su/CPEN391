/*
 * WifiModule.h
 *
 *  Created on: Feb 2, 2017
 *      Author: Simon Jinaphant
 */

#ifndef WIFIMODULE_H_
#define WIFIMODULE_H_
#include <string.h>

// The message to send via SMS when Dependent is safe.
const char WIFI_MESSAGE_DEPENDENT_IS_SAFE[] =
		"The dependent has confirm their safety.";

void wifi_init();
void wifi_send_sms(char message[]);

#endif /* WIFIMODULE_H_ */
