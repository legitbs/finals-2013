#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>

#include "nutil_list.h"

#include "RandomNumberGen.h"
#include "MersenneRNG.h"

#include "SMTPHelperFunctions.h"

#include "CIOConnection.h"
#include "SMTPClientInstance.h"

#include "Domain.h"
#include "MailPath.h"
#include "MailData.h"
#include "SmtpServerInstance.h"
#include "SmtpCommand.h"

#include "SpamFilter.h"
#include "MailEngine.h"

// Alarm timeouts
#define MAX_IDLE_SECS		60

//#define BUILD_WINDOWS_CODE	1
#define BUILD_LINUX_CODE

#define ERROR_500		500		// Syntax error, command unrecognized (This may include errors such as command line too long)
#define ERROR_501		501		// Syntax error in parameters or arguments
#define ERROR_502		502		// Command not implemented
#define ERROR_503		503		// Bad sequence of commands


#define MAX_FORWARD_PATH_LIST	100		// Maximum number of forward path users
#define MAX_MAIL_DATA_SIZE		32000	// Maximum message size


#define RELAY_MESSAGE_FOLDER		"mail_relay/"
#define INBOX_MESSAGE_FOLDER		"inboxes/"

#define MAILBOX_INBOX_NAME			"inbox.mbox"

//#define DEBUG_INPUT_HELPER	1

#endif // __COMMON_H__
