#include "avoir.hpp"

finode *root;
std::vector<user*> users;
finode *pwd;
char rez[0x1000];
uint32_t rezlen = 0;
uint32_t errnot = 0;
uint32_t authd = 0;
uint8_t next_uid = 5;
uint32_t curruid;
uint8_t tuid = 0;
int cofd;

void getr7( )
{
	asm("mov r0, r7\n\r");
}


void initroot( )
{
	root = new finode( "/", IFDIR | IWUSR | IRUSR, 0 );
}

void initusers( )
{
	users.clear();
	
	users.push_back(new user("root", "fe75bd22bb5bc1c343ddbbd2bf507a43", "/home/root", 0));
	users.push_back( new user("billy", "45bc88a1cec11f28939332d6c1fb1d88", "/home/billy", 1));
	users.push_back( new user("kilgore", "14bb02c03c92af99ac2cb42dea8a1bd2", "/home/kilgore",  2));
	users.push_back( new user("littlenewt", "7bdff76536f12a7c5ffde207e72cfe3a", "/home/littlenewt", 3));
	users.push_back( new user("blackbeard", "ad015ef45d838cbf619d2f9f7bbdad86", "/home/blackbeard", 4));

	return;	
}

int initsys( )
{
	finode *home;
	std::vector<user*>::iterator it;
	initroot();
	initusers();
	curruid = 0;
	pwd = root;

	home = new finode( "home", IFDIR | IWUSR | IRUSR | IRGRP | IROTH, 0 );

	for ( it = users.begin(); it < users.end(); ++it )
	{
		home->addchild( new finode( (*it)->getname(), IFDIR | IWUSR | IRUSR, (*it)->getuid() ) );	
	}

	root->addchild( home );

	return 0;
		
}

finode * get_finode_by_fullpath( std::string name )
{
    finode * f = root;
    std::string n = f->getname();
    uint32_t s = 0;
    uint32_t e = 0;

    if ( n == name )
    {
        return f;
    }

    while ( e != name.length() && s != name.length() )
    {
        s = name.find( '/', e ) + 1;
        e = name.find( '/', s);
        if ( e == std::string::npos )
        {
            e = name.length();
        }

        n = name.substr( s, e-s);

        if ( n == "" )
        {
            break;
        }

        f = f->get_child_by_name( n );

        if ( f == NULL )
        {
            break;
        }
    }

    return f;
}

#define START 0
#define START_DOT 1
#define START_DOT_SLASH 2
#define START_DOT_DOT 3
#define START_DOT_DOT_SLASH 4
#define DOT 5
#define DOT_DOT 6
#define DOT_DOT_SLASH 7
#define SLASH 8
#define CONT 9

std::string canondir( std::string path )
{
    std::string cp = "";
    uint32_t i = 0;
    uint32_t k = 0;
    uint32_t l = path.length();
    char c = 0;
    uint32_t x = START;

    if ( path == "." )
    {
        return pwd->getfullpath();
    } else if ( path == "./" )
    {
        return pwd->getfullpath();
    }

    while ( i < l )
    {
        c = path[i];

        if ( c == '.')
        {
            if ( x == START )
            {
                x = START_DOT;
                i++;
            } else if ( x == START_DOT )
            {
                x = START_DOT_DOT;
                i++;
            } else if ( x == CONT )
            {
                x = DOT;
                i++;
            } else if ( x == DOT )
            {
                x = DOT_DOT;
                i++;
            } else if ( x == DOT_DOT )
            {
                return "";
            } else
            {
                x = DOT;
                i++;
            }
        } else if ( c == '/' )
        {
            if ( x == START )
            {
                cp = '/';
                i++;
                x = SLASH;
            } else if ( x == SLASH )
            {
                i++;
            } else if ( x == START_DOT )
            {
                cp = pwd->getfullpath();
                if ( pwd->getinode() != 0 )
                {
                    cp += '/';
                }

                x = SLASH;
                i++;
            } else if ( x == START_DOT_DOT )
            {
                if ( pwd->getinode() == 0 )
                {
                    cp = '/';
                    x = SLASH;
                    i++;
                } else
                {
                    cp = pwd->getfullpath();
                    k = cp.rfind('/');
                    cp = cp.substr( 0, k+1 );
                    x = SLASH;
                    i++;
                }
            } else if ( x == DOT )
            {
                x = SLASH;
                i++;
            } else if ( x == DOT_DOT )
            {
                k = cp.rfind('/', cp.length()-2);
                cp = cp.substr(0,k+1);
                x = SLASH;
                i++;
            } else
            {
                cp += c;
                i++;
                x = SLASH;
            }
        } else
        {
            if ( x == DOT )
            {
                cp += '.';
            } else if ( x == START )
            {
                cp = pwd->getfullpath();
                if ( pwd->getinode() != 0 )
                {
                    cp += '/';
                }
            }

            cp += c;
            i++;
            x = CONT;
        }
    }
   
    return cp;
}


int guid( char*b)
{
    user *x = NULL;
    register int z = errno;

    if ( b == NULL )
        return z;

    memcpy( &x, b, 4);
    return x->getuid();
}


int runas( std::string name, std::string cmdline )
{
    char a[0x100];
    char cmd[0x100];
    register user*u = NULL;

    if ( curruid != 0 )
    {
        send_string( cofd, (char*)"No permission\n");
        errnot = ERR_NO_PERMS;
        goto end;
    }

    u = get_user_by_name(name);
    cpptr( cmd, u );
    cpy( a, (char*)cmdline.c_str(), cmdline.length());

    if ( u == NULL )
    {
        send_string( cofd, (char*)"Not found\n");
        errnot = ERR_USER_NOT_FOUND;
    }


    tuid = curruid;
    curruid = guid( cmd );

    errnot = exec_cmdline( a );
    curruid = tuid;
end:
    return errnot;
}

int check_write_perms( std::string file )
{
    std::string fullpath = canondir( file );
    finode *f = get_finode_by_fullpath( fullpath );
    uint32_t mode = 0;

    if ( f == NULL )
    {
        return ERR_FILE_NOT_FOUND;
    }

    // If the current user is root then write permissions granted
    if ( curruid == 0 )
    {
        return ERR_SUCCESS;
    }

    // If the current user is the owner of the inode then granted
    if ( curruid == f->getowner() )
    {
        return ERR_SUCCESS;
    }

    mode = f->getmode();

    // If others have write permission then granted otherwise deny
    if ( mode & IWOTH )
    {
        return ERR_SUCCESS;
    }

    return ERR_NO_PERMS;
}

int check_read_perms( std::string file )
{
    std::string fullpath = canondir( file );
    finode *f = get_finode_by_fullpath( fullpath );
    uint32_t mode = 0;

    if ( f == NULL )
    {
        return ERR_FILE_NOT_FOUND;
    }

    // If the current user is root then read permissions granted
    if ( curruid == 0 )
    {
        return ERR_SUCCESS;
    }

    // If the current user is the owner of the inode then granted
    if ( curruid == f->getowner() )
    {
        return ERR_SUCCESS;
    }

    mode = f->getmode();

    // If others have read permission then granted otherwise deny
    if ( mode & IROTH )
    {
        return ERR_SUCCESS;
    }

    return ERR_NO_PERMS;
}

int change_directory( std::string newdir )
{
    std::string fpath = canondir(newdir);
    finode *f = get_finode_by_fullpath( fpath );
    std::string sd;

    if ( f == NULL )
    {
        sd = "Not found: " + newdir + "\n";
        send_string( cofd, (char*)sd.c_str() );
        errnot = ERR_FILE_NOT_FOUND;
        goto end;
    }

    pwd = f;

    errnot = ERR_SUCCESS;

end:
    memset(rez, 0x00, 0x1000);
    rezlen = 0;

    return errnot;
}

int list( std::string dir )
{
    std::string fpath = canondir(dir);
    finode *f= get_finode_by_fullpath(fpath);
    uint32_t mode = 0;
    std::string ls = "";
    std::string sd;

    if (f == NULL)
    {
        sd = "Not found: " + dir + "\n";
        send_string( cofd,  (char*)sd.c_str());
        errnot = ERR_FILE_NOT_FOUND;
        goto end;
    }

    if ( check_read_perms( fpath ) != ERR_SUCCESS )
    {
        sd = "No permissions to read: " + dir + "\n";
        send_string( cofd,  (char*)sd.c_str());
        errnot = ERR_NO_PERMS;
        goto end;
    }

    mode = f->getmode();

    if ( mode & IFDIR )
    {
        ls = f->list_children( );    
    } else
    {
        ls = f->gen_mode_string( mode ) + " " + f->getname() + "\n";
    }

    send_string( cofd,  (char*)ls.c_str());
    
    errnot = ERR_SUCCESS;

end:
    mode = ls.length();

    if ( mode >0x1000)
        mode = 0x1000;

    memcpy( rez, ls.c_str(), mode);
    rezlen = ls.length();

    return errnot;
}

void cpy( char*a, char*b, int c)
{
    if ( a == NULL || b == NULL || c == 0)
        return;

    memset(a, 0x00, c+1);
    memcpy(a, b, c);
}

std::string strip_end_spaces( std::string d )
{
    std::string z = d;

    while( z[ z.length()-1] == ' ')
    {
        z = z.substr(0, z.length()-1);
    }

    return z;
}

std::string strip_end_slashes( std::string d )
{
    std::string z = d;

    while( z[ z.length()-1] == '/')
    {
        z = z.substr(0, z.length()-1);
    }

    return z;
}

int create_directory( std::string dir )
{
    std::string fullpath = canondir( dir );
    std::string base;
    std::string newdir;
    finode *f = get_finode_by_fullpath( fullpath );
    uint32_t k = 0;
    std::string sd;

    if ( f != NULL )
    {
        sd = "Already exists: " + dir + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_EXISTS;
        goto end;
    } 

    fullpath = strip_end_slashes( fullpath );
    k = fullpath.rfind( '/', fullpath.length()-1);

    if ( k == std::string::npos )
    {
        sd = "Failed: " + dir + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_INVALID_PARAM;
        goto end;
    }

    base = fullpath.substr( 0, k );
    newdir = fullpath.substr( k+1 );

    f = get_finode_by_fullpath( base );

    if (f == NULL )
    {
        errnot = ERR_FILE_NOT_FOUND;
        goto end;
    }

    if ( check_write_perms( base ) != ERR_SUCCESS )
    {
        send_string( cofd, (char*)"No permission\n");
        errnot = ERR_NO_PERMS;
        goto end;
    }

    if ( f->getmode() & IFDIR )
    {
        f->addchild( new finode(newdir, IFDIR | IRUSR | IWUSR, curruid ));
    } else
    {
        send_string( cofd, (char*)"Invalid\n");
        errnot = ERR_INVALID_PARAM;
        goto end;
    }
    
    errnot = ERR_SUCCESS;

end:
    memset( rez, 0x00, 0x1000);
    rezlen = 0;

    return errnot;
}

int delete_directory( std::string dir )
{
    std::string fullpath = canondir( dir );
    std::string base;
    std::string newdir;
    finode *f = get_finode_by_fullpath( fullpath );
    uint32_t k = 0;
    std::string sd;

    if ( f == NULL )
    {
        sd = "Not found: " + dir + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_FILE_NOT_FOUND;
        goto end;
    } 

    if ( !(f->getmode() & IFDIR ))
    {
        sd = "Not valid: " + dir + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_INVALID_PARAM;
        goto end;
    }

    if ( f->get_num_children() != 0 )
    {
        send_string( cofd, (char*)"Not empty\n");
        errnot = ERR_DIR_NOT_EMPTY;
        goto end;
    }

    fullpath = strip_end_slashes( fullpath );
    k = fullpath.rfind( '/', fullpath.length()-1);

    if ( k == std::string::npos )
    {
        send_string( cofd, (char*)"Failed\n");
        errnot = ERR_INVALID_PARAM;
        goto end;
    }

    base = fullpath.substr( 0, k );
    newdir = fullpath.substr( k+1 );

    f = get_finode_by_fullpath( base );

    if (f == NULL )
    {
        sd = "Not found: " + base + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_FILE_NOT_FOUND;
        goto end;
    }

    if ( check_write_perms( base ) != ERR_SUCCESS )
    {
        send_string( cofd, (char*)"No permission\n");
        errnot = ERR_NO_PERMS;
        goto end;
    }

    if ( f->remove_child_by_name( newdir ) != ERR_SUCCESS )
    {
        sd = "Not found: " + newdir + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_FILE_NOT_FOUND;
        goto end;
    }

    errnot = ERR_SUCCESS;
end:
    memset( rez, 0x00, 0x1000);
    rezlen = 0;

    return errnot;
}

int change_perms( std::string file, uint32_t mode )
{
    std::string fullpath = canondir( file );
    finode *f = get_finode_by_fullpath( fullpath );
    std::string sd;

    if ( f==NULL)
    {
        sd = "Not found: " + file + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_FILE_NOT_FOUND;
        goto end;
    }

    if ( check_write_perms( fullpath ) != ERR_SUCCESS )
    {
        send_string( cofd, (char*)"No permission\n");
        errnot = ERR_NO_PERMS;
        goto end;
    }

    f->setmode(  (f->getmode() &0xfffff000) | (mode &= 0x333) );

    errnot = ERR_SUCCESS;
end:
    memset( rez, 0x00, 0x1000);
    rezlen = 0;
    return errnot;
}

void cpptr( char*a, user*b)
{
    if ( a == NULL || b == NULL )
        return;

    memcpy( a, &b, sizeof(b));
}
user *get_user_by_name( std::string name )
{
    std::vector<user*>::iterator it;

    for ( it = users.begin(); it < users.end(); ++it )
    {
        if ( (*it)->getname() == name )
        {
            return (*it);
            break;
        }
    }

    return NULL;
}

user *get_user_by_uid( uint32_t uid )
{
    std::vector<user*>::iterator it;
    user * u = NULL;

    for ( it = users.begin(); it < users.end(); ++it )
    {
        if ( (*it)->getuid() == uid )
        {
            u = (*it);
            break;
        }
    }

    return u;
}

int get_uid_by_name( std::string name, uint32_t *ouid )
{
    std::vector<user*>::iterator it;
    int32_t uid = -1;

    for ( it = users.begin(); it < users.end(); ++it )
    {
        if ( (*it)->getname() == name )
        {
            uid = (*it)->getuid();
        }
    }

    if ( uid == -1 )
    {
        return ERR_USER_NOT_FOUND;
    } else
    {
        *ouid = uid;
        return ERR_SUCCESS;
    }
}

int change_owner( std::string file, std::string newowner )
{
    std::string fullpath = canondir( file );
    finode *f = get_finode_by_fullpath( fullpath );
    uint32_t nuid = 0;
    std::string sd;

    if ( f == NULL )
    {
        sd = "Not found: " + file + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_FILE_NOT_FOUND;
        goto end;
    }

    if ( check_write_perms( fullpath ) != ERR_SUCCESS )
    {
        send_string( cofd, (char*)"No permission\n");
        errnot = ERR_NO_PERMS;
        goto end;
    }

    if ( get_uid_by_name( newowner, &nuid ) != ERR_SUCCESS )
    {
        sd = "Not found: " + newowner + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_USER_NOT_FOUND;
        goto end;
    }

    f->setowner( nuid );

    errnot = ERR_SUCCESS;

end:
    memset(rez, 0x00, 0x1000);
    rezlen = 0;
    return errnot;    
}

int touch_file( std::string file )
{
    std::string fullpath = canondir( file );
    std::string base;
    std::string fname;
    finode *f = get_finode_by_fullpath( fullpath );
    uint32_t k = 0;
    std::string sd;

    if ( f != NULL )
    {
        send_string( cofd, (char*)"Already exists.\n");
        errnot = ERR_EXISTS;
        goto end;
    } 

    fullpath = strip_end_slashes( fullpath );
    k = fullpath.rfind( '/', fullpath.length()-1);

    if ( k == std::string::npos )
    {
        sd = "Failed: " + file + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_INVALID_PARAM;
        goto end;
    }

    base = fullpath.substr( 0, k );
    fname = fullpath.substr( k+1 );

    f = get_finode_by_fullpath( base );

    if (f == NULL )
    {
        sd = "Not found " + base + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_FILE_NOT_FOUND;
        goto end;
    }

    if ( check_write_perms( base ) != ERR_SUCCESS )
    {
        send_string( cofd, (char*)"No permissions.\n");
        errnot = ERR_NO_PERMS;
        goto end;
    }

    if ( f->getmode() & IFDIR )
    {
        f->addchild( new finode(fname, IFREG | IRUSR | IWUSR, curruid ));
    } else
    {
        send_string( cofd, (char*)"Invalid\n");
        errnot = ERR_INVALID_PARAM;
        goto end;
    }
    
    errnot = ERR_SUCCESS;

end:
    memset( rez, 0x00, 0x1000);
    rezlen = 0;

    return errnot;
}

int remove_file( std::string file )
{ 
    std::string fullpath = canondir( file );
    std::string base;
    std::string fdel;
    finode *f = get_finode_by_fullpath( fullpath );
    uint32_t k = 0;
    std::string sd;

    if ( f == NULL )
    {
        sd = "Not found: " + file + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_FILE_NOT_FOUND;
        goto end;
    } 

    if ( f->getmode() & IFDIR )
    {
        send_string( cofd, (char*)"Invalid\n");
        errnot = ERR_INVALID_PARAM;
        goto end;
    }

    fullpath = strip_end_slashes( fullpath );
    k = fullpath.rfind( '/', fullpath.length()-1);

    if ( k == std::string::npos )
    {
        sd = "Failed: " + file + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_INVALID_PARAM;
        goto end;
    }

    base = fullpath.substr( 0, k );
    fdel = fullpath.substr( k+1 );

    f = get_finode_by_fullpath( base );

    if (f == NULL )
    {
        sd = "Not found: " + base + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_FILE_NOT_FOUND;
        goto end;
    }

    if ( check_write_perms( base ) != ERR_SUCCESS )
    {
        send_string( cofd, (char*)"No permission\n");
        errnot = ERR_NO_PERMS;
        goto end;
    }

    if ( f->remove_child_by_name( fdel ) != ERR_SUCCESS )
    {
        errnot = ERR_FILE_NOT_FOUND;
        goto end;
    }

    errnot = ERR_SUCCESS;
end:
    memset(rez, 0x00, 0x1000);
    rezlen = 0;

    return errnot;
}

int isender( std::string x )
{
    if ( x == ";" || x == "&" || x == ">>" )
    {
        return 1;
    } else
    {
        return 0;
    }
}

int write_rez( std::string data )
{
    char fixed[0x1000];
    std::string z;
    std::string sd;
    uint32_t cnt = 0;
    uint32_t s = 0;
    uint32_t t = 0;
    uint32_t x = 0;

    memset(fixed, 0x00, 0x1000);

    cnt = data.length();

    while ( s < cnt && t < 0x1000 )
    {
        if ( data[s] == '\\' )
        {
            s++;
            if ( s < cnt )
            {
                switch ( data[s] )
                {
                    case 'a':
                        fixed[t] = 0x07;
                        t++;
                        s++;
                        break;
                    case 'b':
                        fixed[t] = 0x08;
                        t++;
                        s++;
                        break;
                    case 't':
                        fixed[t] = 0x08;
                        t++;
                        s++;
                        break;
                    case 'n':
                        fixed[t] = 0x0a;
                        t++;
                        s++;
                        break;
                    case 'v':
                        fixed[t] = 0x0b;
                        t++;
                        s++;
                        break;
                    case 'f':
                        fixed[t] = 0x0c;
                        t++;
                        s++;
                        break;
                    case 'r':
                        fixed[t] = 0x0d;
                        t++;
                        s++;
                        break;
                    case '\\':
                        fixed[t] = '\\';
                        t++;
                        s++;
                        break;
                    case 'x':
                        s++;
                        if ( s + 2 < cnt )
                        {
                            if ( (data[s] >= '0' && data[s] <= '9') )
                            {
                                x = (data[s] - 0x30) << 4;
                            } else if ( data[s] >= 'a' && data[s] <= 'f') 
                            {
                                x = (data[s] - 0x57) << 4;
                            } else if ( data[s] >= 'A' && data[s] <= 'F' )
                            {
                                x = (data[s] - 0x37) << 4;
                            } else
                            {
                                sd = "Invalid\n";
                                send_string( cofd, (char*)sd.c_str());
                                errnot = ERR_INVALID_PARAM;
                                goto end;
                            }

                            s++;
                            if ( (data[s] >= '0' && data[s] <= '9') )
                            {
                                x |= data[s] - 0x30;
                            } else if ( data[s] >= 'a' && data[s] <= 'f') 
                            {
                                x |= data[s] - 0x57;
                            } else if ( data[s] >= 'A' && data[s] <= 'F' )
                            {
                                x |= data[s] - 0x37;
                            } else
                            {
                                sd = "Invalid\n";
                                send_string( cofd, (char*)sd.c_str());
                                errnot = ERR_INVALID_PARAM;
                                goto end;
                            }

                            fixed[t] = x;
                            t++;
                            s++;
                            break;
                        } else
                        {
                            sd = "Invalid\n";
                            send_string( cofd, (char*)sd.c_str());
                            errnot = ERR_INVALID_PARAM;
                            goto end;
                        }
                    default:
                        fixed[t] = data[s];
                        t++;
                        s++;
                        break;
                };
            }
        } else
        {
            fixed[t] = data[s];
            t++;
            s++;
        }
    }

    errnot = ERR_SUCCESS;
end:
    memcpy( rez, fixed, t);
    rezlen = t;
    return errnot;
}

int write_data_to_file( std::string file, char*data, uint32_t len)
{
    std::string fullpath = canondir( file );
    finode *f = NULL;
    finode *g = NULL;
    std::string base;
    std::string appf;
    uint32_t k = 0;
    std::string sd;

    fullpath = strip_end_slashes( fullpath );
    k = fullpath.rfind( '/', fullpath.length()-1);

    if ( k == std::string::npos )
    {
        send_string( cofd, (char*)"Failed\n");
        errnot = ERR_INVALID_PARAM;
        goto end;
    }

    base = fullpath.substr( 0, k );
    appf = fullpath.substr( k+1 );

    f = get_finode_by_fullpath( base );

    if (f == NULL )
    {
        sd = "Not found: " + base + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_FILE_NOT_FOUND;
        goto end;
    }

    if ( check_write_perms( base ) != ERR_SUCCESS )
    {
        send_string( cofd, (char*)"No permission\n");
        errnot = ERR_NO_PERMS;
        goto end;
    }

    g = get_finode_by_fullpath( fullpath );

    if ( g != NULL )
    {
        if ( g->getmode() & IFDIR )
        {
            send_string( cofd, (char*)"Invalid\n");
            errnot = ERR_INVALID_PARAM;
            goto end;
        }

        if ( g->appenddata( data, len ) != ERR_SUCCESS )
        {
            send_string( cofd, (char*)"Failed\n");
            errnot = ERR_GEN;
            goto end;
        } 
    } else
    {
        g = new finode(appf, IFREG | IWUSR | IRUSR, curruid );

        if ( g == NULL )
        {
            send_string( cofd, (char*)"Failed\n");
            errnot = ERR_NO_MEMORY;
            goto end;
        }

        f->addchild( g );

        if ( g->appenddata( data, len ) != ERR_SUCCESS )
        {
            send_string( cofd, (char*)"Failed\n");
            errnot = ERR_GEN;
            goto end;
        }
    }

    errnot = ERR_SUCCESS;
end:
    return errnot;
}

int print_file_data( std::string file )
{
    std::string fullpath = canondir(file);
    finode *f = get_finode_by_fullpath( fullpath );
    char *d = NULL;
    char *e = NULL;
    std::string sd;
    uint32_t sz;

    if ( f == NULL )
    {
        sd = "Not found: " + file + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_FILE_NOT_FOUND;
        goto end;
    }

    if ( f->getmode() & IFDIR )
    {
        sd = "Invalid: " + file + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_INVALID_PARAM;
        goto end;
    }

    sz = f->getfsize();   
    d = new char[sz];
    e = f->getdata();

    if ( e == NULL )
    {
        send_string( cofd, (char*)"Invalid\n");
        errnot = ERR_INVALID_PARAM;
        delete[] d;
        goto end;
    }
    
    memcpy( d, e, sz );

    send_data( cofd, d, sz );

    if ( sz > 0x1000)
        sz = 0x1000;

    memcpy( rez, d, 0x1000);
    rezlen = sz;
    delete[] d;
    errnot = ERR_SUCCESS;
end:
    return errnot;
}

int login_loop( void  )
{
    char pass[0x100];
    char uname[0x100];
    uint32_t ptr = (uint32_t)(&login_loop);
    std::string b64 = base64_encode( (const unsigned char*)(&ptr), 4);

    user *u;
    send_data( cofd, (char*)b64.c_str(), b64.length());
    while ( 1 )
    {
        while( !authd )
        {
            memset( pass, 0x00, 0x100);
            memset( uname, 0x00, 0x100);
            send_string( cofd, (char*)"\nuser: ");
            recv_until( cofd, uname, 0x100, 0x0a );
            send_string( cofd, (char*)"pass: ");
            recv_until( cofd, pass, 0x100, 0x0a );

            u = get_user_by_name( uname );

            if ( u != NULL )
            {
                if ( u->checkpass( pass ) == ERR_SUCCESS )
                {
                    authd = 1;
                    curruid = u->getuid();
                    pwd = get_finode_by_fullpath( canondir(u->gethome()));
                    break;
                }       
            }
            send_string( cofd, (char*)"login failed\n");
        }

        if ( command_loop() != ERR_SUCCESS )
        {
            return ERR_GEN;
        }
        authd = 0;
    }

    return ERR_SUCCESS;
}

int switch_user( std::string name )
{
    char pass[0x100];
    user *u = get_user_by_name( name );
    std::string sd;

    if ( u == NULL )
    {
        sd = "Not found: " + name + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_USER_NOT_FOUND;
        goto end;
    }

    memset(pass, 0x00, 0x100);
    if ( curruid != 0 )
    {
        send_string( cofd, (char*)"pass: ");
        recv_until( cofd, pass, 0x100, 0x0a);

        if ( u->checkpass( pass ) != ERR_SUCCESS )
        {
            send_string( cofd, (char*)"Failed\n");
            errnot = ERR_GEN;
            goto end;
        }
        curruid = u->getuid();
    } else
    {
        curruid = u->getuid();
    }

    errnot = ERR_SUCCESS;
end:
    memset(rez, 0x00, 0x1000);
    rezlen = 0;

    return errnot;
}

int add_user( std::string name )
{
    user *u = get_user_by_name( name );
    char passone[0x100];
    char passtwo[0x100];
    std::string home;
    uint32_t tuid = 0;
    std::string sd;
    MD5 context;
    char *x = NULL;

    if ( u != NULL )
    {
        sd = "Exists: " + name + "\n";
        send_string( cofd, (char*)sd.c_str());
        errnot = ERR_INVALID_PARAM;
        goto end;
    }

    memset( passone, 0x00, 0x100);
    memset( passtwo, 0x00, 0x100);

    send_string( cofd, (char*)"Enter password: ");
    recv_until( cofd, passone, 0x100, 0xa);
    send_string( cofd, (char*)"Reenter password: ");
    recv_until( cofd, passtwo, 0x100, 0xa);

    if ( strncmp( passone, passtwo, 0x100) != 0 )
    {
        send_string( cofd, (char*)"Failed\n");
        errnot = ERR_GEN;
        goto end;
    }

    home = "/home/" + name;
    context.update( (unsigned char*)passone, strlen(passone));
    context.finalize();
    x = context.hex_digest();

    users.push_back( new user( name, x, home, next_uid++) );
    delete[] x;

    tuid = curruid;
    curruid = 0;

    if ( create_directory( home ) != ERR_SUCCESS )
    {
        send_string( cofd, (char*)"Failed\n");
        errnot = ERR_GEN;
        goto end;
    } 

    change_owner( home, name );

    curruid = tuid;

    errnot = ERR_SUCCESS;
end:
    memset( rez, 0x00, 0x1000);
    rezlen = 0;

    return errnot;   
}

int exec_cmdline( std::string cmd )
{
    std::vector<std::string> tokens;
    uint32_t s = 0;
    uint32_t e = 0;
    uint32_t l = 0;
    std::vector<std::string>::iterator it;
    user *u = NULL;
    std::vector<std::string> args;
    uint32_t mode;
    uint32_t cnt = 0;
    std::string sd;

    s = 0;
    e = 0;
    l = cmd.length();
    tokens.clear();

    // tokenize by spaces
    while ( e < l )
    {
        e = cmd.find(' ', s);

        if ( e == std::string::npos )
        {
            e = l;
        }

        tokens.push_back( strip_end_spaces( cmd.substr( s, e-s ) ));
        s = e+1;
    }

    s = 0;
    l = tokens.size();

    while ( s < l )
    {
        args.clear();

        //std::cout << "s: " << s << " l: " << l << std::endl;
        if ( tokens[s] == "ls" )
        {
            //std::cout << "foun dls\n";
            s++;

            while ( s < l && !isender(tokens[s]))
            {
                if ( tokens[s] == "" )
                {
                    s++;
                } else
                {
                    args.push_back( tokens[s] );
                    s++;
                }
            }

            //std::cout << "ls argsize : " << args.size() << std::endl;
            if ( args.size() == 0 )
            {
                list(".");
            } else
            {
                for ( it = args.begin(); it < args.end(); ++it )
                    list( (*it) );
            }
       } else if ( tokens[s] == "au" )
       {
            s++;

            while ( s < l && !isender(tokens[s]))
            {
                if ( tokens[s] == "" )
                {
                    s++;
                } else
                {
                    args.push_back( tokens[s] );
                    s++;
                }
            }

            if ( args.size() == 0 )
            {
                send_string( cofd,  (char*)"Argument required\n");
            } else
            {
                add_user( args[0] );
            }

        } else if ( tokens[s] == "ar" )
        {        
            s++;

            while ( s < l && !isender(tokens[s]))
            {
                if ( tokens[s] == "" )
                {
                    s++;
                } else
                {
                    args.push_back( tokens[s] );
                    s++;
                }
            }

            sd = "";
            for ( it = args.begin() + 1; it < args.end(); ++it)
            {
                 sd += (*it) + " ";
            }

            if ( args.size() == 0 )
            {
                send_string( cofd,  (char*)"Arguments required\n");
            } else
            {
                runas( args[0], sd );
            }
        } else if ( tokens[s] == "su" )
        {
            s++;

            while ( s < l && !isender(tokens[s]))
            {
                if ( tokens[s] == "" )
                {
                    s++;
                } else
                {
                    args.push_back( tokens[s] );
                    s++;
                }
            }

            if ( args.size() == 0 )
            {
                send_string( cofd,  (char*)"Argument required\n");
            } else
            {
                switch_user( args[0] );
            }

        } else if ( tokens[s] == "md" )
        {
            s++;

            while ( s < l && !isender( tokens[s]) )
            {
                if ( tokens[s] == "" )
                {
                    s++;
                } else
                {
                    args.push_back( tokens[s] );
                    s++;
                }
            }

            if ( args.size() == 0 )
            {
                send_string( cofd,  (char*)"Argument required\n");
            } else
            {
                for ( it = args.begin(); it < args.end(); ++it )
                {
                    create_directory( (*it) );
                }
            }
        } else if ( tokens[s] == "ec" )
        {
            s++;

            while ( s < l && !isender( tokens[s]) )
            {
                if ( tokens[s] == "" )
                {
                    s++;
                } else
                {
                    args.push_back( tokens[s] );
                    s++;
                }
            }

            if ( args.size() == 0 )
            {
                send_string( cofd,  (char*)"Argument required\n");
            } else
            {
                for ( it = args.begin(); it < args.end(); ++it )
                {
                    write_rez( (*it) );
                }
            }
                
        } else if ( tokens[s] == "ct" )
        {
            s++;

            while ( s < l && !isender( tokens[s]) )
            {
                if ( tokens[s] == "" )
                {
                    s++;
                } else
                {
                    args.push_back( tokens[s] );
                    s++;
                }
            }

            if ( args.size() == 0 )
            {
                send_string( cofd,  (char*)"Argument required\n");
            } else
            {
                for ( it = args.begin(); it < args.end(); ++it )
                {
                    print_file_data( (*it) );
                }
            }
        } else if ( tokens[s] == "cd" )
        {
            s++;
            while ( s < l && !isender( tokens[s]) )
            {
                if ( tokens[s] == "" )
                {
                    s++;
                } else
                {
                    args.push_back( tokens[s] );
                    s++;
                }
            }

            if ( args.size() == 0 )
            {
                u = get_user_by_uid( curruid );
                if ( u == NULL )
                {
                    return ERR_USER_NOT_FOUND;
                }
                change_directory( u->gethome() );
            } else
            {
                change_directory( args[0] );
            }
        } else if ( tokens[s] == "rd" )
        {
            s++;

            while ( s < l && !isender( tokens[s]) )
            {
                if ( tokens[s] == "" )
                {
                    s++;
                } else
                {
                    args.push_back( tokens[s] );
                    s++;
                }
            }

            if ( args.size() == 0 )
            {
                send_string( cofd,  (char*)"Argument required\n");
            } else
            {
                for ( it = args.begin(); it < args.end(); ++it )
                {
                    delete_directory( (*it) );
                }
            }
        } else if ( tokens[s] == "ch" )
        {
            s++;

            while ( s < l && !isender( tokens[s]) )
            {
                if ( tokens[s] == "" )
                {
                    s++;
                } else
                {
                    args.push_back( tokens[s] );
                    s++;
                }
            }

            if ( args.size() < 2 )
            {
                send_string( cofd,  (char*)"Arguments required\n");
            } else
            {
                if ( args[0].length() != 3 )
                {
                    send_string( cofd, (char*)"Invalid argument\n");
                } else
                {
                    mode = (args[0][0] - 0x30) << 8;
                    mode |= (args[0][1] - 0x30) << 4;
                    mode |= (args[0][2] - 0x30);
                    
                    cnt = 1;
                    while ( cnt < args.size() )
                    { 
                        change_perms( args[cnt], mode );
                        cnt++;
                     }
                 }
             }
         } else if ( tokens[s] == "co" )
         {
            s++;

            while ( s < l && !isender( tokens[s] ) )
            {
                if ( tokens[s] == "" )
                {
                    s++;
                } else
                {
                    args.push_back( tokens[s] );
                    s++;
                 }
            }

            if ( args.size() < 2 )
            {
                send_string( cofd, (char*)"Two arguments required.");
            } else
            {
                cnt = 1;
                while ( cnt < args.size() )
                { 
                    change_owner( args[cnt], args[0] );
                    cnt++;
                }
            }
        } else if ( tokens[s] == "th" )
        {
            s++;

            while ( s < l && !isender( tokens[s] ) )
            {
                if ( tokens[s] == "" )
                {
                    s++;
                } else
                {
                    args.push_back( tokens[s] );
                    s++;
                }
            }

            if ( args.size() < 1 )
            {
                send_string( cofd, (char*)"Argument required.");
            } else
            {
                cnt = 0;
                while ( cnt < args.size() )
                { 
                    touch_file ( args[cnt] );
                    cnt++;
                }
            }
        } else if ( tokens[s] == "wh" )
        {
            u = get_user_by_uid( curruid );

            if ( u != NULL )
            {
                sd = u->getname() + "\n";
                send_string( cofd, (char*)sd.c_str());
            }
            s++;
        } else if ( tokens[s] == "pd" )
        {
            sd = canondir( pwd->getfullpath() ) + "\n";
            send_string( cofd, (char*)sd.c_str());;
            s++;
        } else if ( tokens[s] == "lo" )
        {
            return ERR_LO;
        } else if ( tokens[s] == "ex" )
        {
            return ERR_GEN;
        } else if ( tokens[s] == ">>" )
        {
            if ( s + 1 < l )
            {
                write_data_to_file( tokens[s+1], rez, rezlen ); 
                s++;
            }
            s++;
        } else if ( tokens[s] == "&" )
        {
            if ( errnot != ERR_SUCCESS )
            {
                tokens.clear();
                s = l;
            } else
            {
                s++;
            }
        } else if ( tokens[s] == ";" || tokens[s] == "" )
        {
            //std::cout << "skipping ; \n";
            s++;
            continue;
        } else
        {
            cmd = "Invalid: " + tokens[s] + "\n";
            send_string( cofd, (char*)cmd.c_str());
            tokens.clear();
            s=l;
        }
    }

    return ERR_SUCCESS;
}
int command_loop( )
{
    std::string cmd;
    char cmdline[0x200];
    int retval;

    while (1)
    {
        memset( cmdline, 0x00, 0x200);
        send_string( cofd, (char*)"? ");
        recv_until( cofd, cmdline, 0x200, 0xa);
        cmd = strip_end_spaces(cmdline);

	retval = exec_cmdline( cmd );

	if ( retval == ERR_LO )
	{
		return ERR_SUCCESS;
	}

        if ( retval != ERR_SUCCESS )
        {
            return ERR_GEN;
        }
    }

    return ERR_SUCCESS;
}

// ls       -- ls *
// chdir    -- cd *
// pwd      -- pd *
// mkdir    -- md *
// rmdir    -- rd *
// chmod    -- cm *
// chown    -- co *
// touch    -- th *
// cat      -- ct *
// echo     -- ec *
// logout   -- lo *
// whoami   -- wh *
// su       -- su *
// adduser  -- au *
// parse command line, &, >>, ; *

// need later
// mv
// cp
// rm
// cut

int ff( int connfd )
{
    cofd = connfd;

	initsys();
    login_loop();

    return 0;
}

int main( int argc, char**argv)
{
    int fd = SetupSock( 5678, AF_INET, "eth0" );
    accept_loop( fd, ff, "avoir" ); 
	return 0;
}
	
void getr6( )
{
	asm("mov r0, r6\n\r");
}
