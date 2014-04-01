#include "user.hpp"

user::user( std::string name, std::string pass, std::string home, uint32_t uid )
{
    this->name = name;
    this->pass = pass;
    this->home = home;
    this->uid = uid;

    return;
}

uint32_t user::checkpass( std::string pass )
{
    uint32_t retval = 0;
    uint32_t len = pass.length();
    char *t = (char*)malloc( len + 1 );
    MD5 context;
    std::stringstream ss;
    char *u = NULL;
    std::string v;

    if ( t == NULL )
    {
        retval = ERR_NO_MEMORY;
        goto end;
    }

    memset( t, 0x00, len+1);
    memcpy( t, pass.c_str(), len);

    context.update( (unsigned char*)t, len );
    context.finalize();

    u = context.hex_digest();
    v = u;
    delete[] u;    

    if ( v == this->pass )
    {
        retval = ERR_SUCCESS;
        goto end; 
    } else
    {
        retval = ERR_NO_PERMS;
        goto end;
    }

    end:
    return retval;
}
