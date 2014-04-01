#ifndef __AVOIR__
#define __AVOIR__
#include "common.hpp"

void getr6(void);
user *get_user_by_name( std::string );
int command_loop(void);
int login_loop(void);
int exec_cmdline(std::string);
void cpptr( char*, user*);
void cpy(char*,char*,int);
#endif
