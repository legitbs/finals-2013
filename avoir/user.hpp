#ifndef __USER_HPP__
#define __USER_HPP__

#include "common.hpp"

class user
{
    private:
    std::string name;
    std::string pass;
    std::string home;
    uint32_t uid;

    public:
    user( std::string name, std::string pass, std::string home, uint32_t uid );
    virtual void setname( std::string name ) { this->name = name; }
    virtual void setpass( std::string pass ) { this->pass = pass; }
    virtual void setuid( uint32_t uid) { this->uid = uid; }
    virtual std::string getname( void ) { return this->name; }
    virtual std::string getpass( void ) { return this->pass; }
    virtual uint32_t getuid( void ) { return this->uid; }
    virtual uint32_t checkpass( std::string pass );
    virtual std::string gethome( void ) { return this->home; }
};

#endif
