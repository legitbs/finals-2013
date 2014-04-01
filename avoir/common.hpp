#ifndef __COMMON__
#define __COMMON__

#include <iostream>
#include <vector>
#include <string>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <sstream>
#include <errno.h>

#include "base64.hpp"
#include "sharedfuncs.h"
#include "user.hpp"
#include "finode.hpp"
#include "initsys.hpp"
#include "md5.hpp"

#define ERR_SUCCESS 0
#define ERR_FILE_NOT_FOUND -1
#define ERR_UNKNOWN_COMMAND -2
#define ERR_NO_PERMS -3
#define ERR_INVALID_PARAM -4
#define ERR_NOT_A_DIRECTORY -5
#define ERR_EXISTS -6
#define ERR_DIR_NOT_EMPTY -7
#define ERR_USER_NOT_FOUND -8
#define ERR_NO_MEMORY -9
#define ERR_GEN -10
#define ERR_LO -11

#endif
