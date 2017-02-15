
#ifndef WIFI_H_
#define WIFI_H_

#include <stdio.h>
#include <string.h>
#include "RS232.h"


// The message to send via SMS when Dependent is safe.
char WIFI_MESSAGE_DEPENDENT_IS_SAFE[] =
		"The dependent has confirm their safety.";

void wifi_init();
void wifi_send_sms(char message[]);

#endif /* WIFI_H_ */
