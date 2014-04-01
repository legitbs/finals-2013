#ifndef __FINODE__
#define __FINODE__
#include <iostream>
#include <vector>
#include <string>
#include <stdint.h>

#include "common.hpp"

// modes
#define IFDIR 0x20000
#define IFREG 0x10000
#define ISUID 0x01000
#define IRUSR 0x00200
#define IWUSR 0x00100
#define IRGRP 0x00020
#define IWGRP 0x00010
#define IROTH 0x00002
#define IWOTH 0x00001

class finode
{
    private:

    std::string name;   // file name
    uint32_t fsize;     // file size
    char *data;         // data
    uint32_t inode;     // file inode
    finode * pinode;    // parent inode
    uint32_t mode;      // permissions
    uint32_t owner;     // owner
    std::vector<finode*> children;  // children inodes

    public:
    finode( std::string name, uint32_t mode, uint32_t owner);
    virtual void setname( std::string name ) { this->name = name; }
    virtual std::string getname( void ) { return this->name; }
    virtual void setsize( uint32_t fsize ) { this->fsize = fsize; }
    virtual uint32_t getfsize( void ) { return this->fsize; }
    virtual uint32_t getmode( void ) { return this->mode; }
    virtual void setmode( uint32_t mode ) { this->mode = mode; }
    virtual uint32_t addchild( finode *child );
    virtual uint32_t remove_child_by_name( std::string name );
    virtual uint32_t get_num_children( void ) { return this->children.size(); }
    virtual finode *get_child_by_name( std::string name );
    virtual std::string list_children( void );
    virtual std::string getfullpath( void );
    virtual uint32_t getinode( void ) { return this->inode; }
    virtual void setpinode( finode *pinode ) { this->pinode = pinode; }
    virtual finode *getpinode( void ) { return this->pinode; }
    virtual uint32_t getowner( void ) { return this->owner; }
    virtual void setowner( uint32_t owner ) { this->owner = owner; }
    virtual std::string gen_mode_string( uint32_t mode );
    virtual char *getdata( void ) { return this->data; };
    virtual int appenddata( char *data, uint32_t len );
};

#endif
