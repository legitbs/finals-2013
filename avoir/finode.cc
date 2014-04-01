#include "finode.hpp"

finode *current_user;
uint32_t inode_g = 0;

int finode::appenddata( char *data, uint32_t len )
{
    char*temp = NULL;

    if ( data == NULL )
    {
        return ERR_INVALID_PARAM;
    }

    if ( this->data == NULL )
    {
        this->data = (char*)malloc(len+1);

        if ( this->data == NULL)
        {
            return ERR_NO_MEMORY;
        }
        memset( this->data, 0x00, len+1);
        memcpy( this->data, data, len);
        this->fsize = len;
        return ERR_SUCCESS;
    }

    temp = (char*)malloc( this->fsize + len );

    if ( temp == NULL )
    {
        return ERR_NO_MEMORY;
    }

    memcpy( temp, this->data, this->fsize );
    memcpy( temp+this->fsize, data, len);

    delete[] this->data;
    this->data = temp;
    this->fsize += len;

    return ERR_SUCCESS;
}

std::string finode::gen_mode_string( uint32_t mode )
{
    std::string m = "";

    if ( mode & IFDIR )
    {
        m += "d";
    } else
    {
        m += "-";
    }

    if ( mode & IRUSR )
    {
        m += "r";
    } else
    {
        m += "-";
    }

    if ( mode & IWUSR )
    {
        m += "w";
    } else
    {
        m += "-";
    }

    if ( mode & IRGRP )
    {
        m += "r";
    } else
    {
        m += "-";
    }

    if ( mode & IWGRP )
    {
        m += "w";
    } else
    {
        m += "-";
    }
    if ( mode & IROTH )
    {
        m += "r";
    } else
    {
        m += "-";
    }

    if ( mode & IWOTH )
    {
        m += "w";
    } else
    {
        m += "-";
    }

    return m;
}

finode *finode::get_child_by_name( std::string name )
{
    finode *child = NULL;

    std::vector<finode*>::iterator it;

    for ( it = this->children.begin(); it != this->children.end(); ++it)
    {
        if ( (*it)->getname() == name )
        {  
            child = (*it);
        }
    }

    return child; 
}

std::string finode::getfullpath( void )
{
    finode *c = this->getpinode();
    std::string dir = this->getname();

    while ( c != NULL )
    {
        if ( c->getinode() == 0 )
        {
            dir.insert( 0, c->getname());
        } else
        {
            dir.insert( 0, c->getname() + '/' );
        }
        c = c->getpinode();
    }

    return dir;
}

uint32_t finode::addchild( finode *child)
{
    if ( child == NULL )
    {
        return ERR_INVALID_PARAM;
    }

    if ( !(this->mode & IFDIR) )
    {
        return ERR_NO_PERMS;
    }

    // Set the parent inode
    child->setpinode( this );

    this->children.push_back( child );

    return ERR_SUCCESS;
}

std::string convertInt(int number)
{
   std::stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}

std::string finode::list_children( void )
{
    std::string ls = "";
    std::vector<finode*>::iterator it;

    if ( !(this->getmode() & IFDIR) )
    {
        return ls;
    }

    for (it = this->children.begin(); it < this->children.end(); ++it )
    {
        ls += this->gen_mode_string( (*it)->getmode() );
        ls += "\t";
        ls += convertInt( (*it)->fsize );
        ls += "\t";
        ls += convertInt((*it)->owner);
        ls += "\t";
        ls += (*it)->getname() + "\n";
    }

    return ls;
}

uint32_t finode::remove_child_by_name( std::string name )
{
    uint32_t found = 0;

    std::vector<finode*>::iterator it;

    for ( it = this->children.begin(); it < this->children.end(); ++it)
    {
        if ( (*it)->getname() == name )
        {
            this->children.erase(it);
            found = 1;
            break;
        }
    }

    if ( found )
    {
        return ERR_SUCCESS;
    } else
    {
        return ERR_FILE_NOT_FOUND;
    }
}

finode::finode( std::string name, uint32_t mode, uint32_t owner )
{
    this->name = name;
    this->fsize = 0;
    this->data = NULL;
    this->inode = inode_g++;
    this->mode = mode;
    this->owner = owner;
    this->pinode = NULL;
    this->children.clear();
}

