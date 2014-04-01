#include "common.h"
#include "users.h"
#include "sharedfuncs.h"

int authenticated = 0;
int dropped = 0;
int mode = 0;
char*user = NULL;
char*pass = NULL;

#define MAXDROP 0x1000

struct linux_dirent {
	long           d_ino;
	off_t          d_off;
        unsigned short d_reclen;
        char           d_name[];
};

int getlist( uint8_t * dir, uint8_t *list, uint32_t size)
{
	int fd = 0;
	int nread;
	char *buf = alloca(0x100);
	struct linux_dirent *d;
	int bpos;
	uint32_t lpos = 0;
	uint32_t cnt = 0;

	if ( dir == NULL )
	{
		return -1;
	}

	if ( list == NULL )
	{
		return -1;
	}


        asm (
        " .arm\n\t"
        " mov r0, %[dir]\n\t"
        " mov r1, #0x4000\n\t"
        " mov r2, #0\n\t"
        " mov r7, #5\n\t"
        " svc #0\n\t"
        " mov %[fd], r0\n\t"
        : [fd] "+r" (fd)
        : [dir] "r" (dir)
        : );

	if ( fd == -1 )
	{
		return -1;
	}

	for ( ;lpos<size ; )
	{
		asm(
		" .arm\n\t"
		" push {r1-r2, r7}\n\t"
		" mov r0, %[fd]\n\t"
		" mov r1, %[buf]\n\t"
		" mov r2, #0x100\n\t"
		" mov r7, #141\n\t"
		" svc #0\n\t"
		" mov %[nread], r0\n\t"
		" pop {r1-r2, r7}\n\t"
		: [nread] "+r" (nread)
		: [fd] "r" (fd), [buf] "r" (buf)
		: );

		if ( nread == -1 )
		{
			return -1;
		}

		if ( nread == 0)
		{
			break;
		}

		for (bpos = 0; bpos < nread && lpos<size;)
		{
			d = (struct linux_dirent *)(buf+bpos);
			
			cnt = 0;
			while ( d->d_name[cnt] != 0 && lpos < size )
			{
				list[lpos++] = d->d_name[cnt++];
			}

			list[lpos++] = '\n';
			bpos += d->d_reclen;
		}
	}

	return 0;
}

int getdir( uint8_t *dir, uint32_t size )
{
	int retval = 0;

	if ( dir == NULL )
	{
		return -1;
	}

        asm (
        " .arm\n\t"
        " mov r0, %[dir]\n\t"
        " mov r1, %[size]\n\t"
        " mov r7, #183\n\t"
        " svc #0\n\t"
        " mov %[retval], r0\n\t"
        : [retval] "+r" (retval)
        : [dir] "r" (dir), [size] "r" (size)
        : );

	if ( retval != 0 )
	{
		retval = 0;
	}
	else
	{
		retval = -1;
	}

	return retval;

}

int filerename( uint8_t *path, uint8_t *newpath )
{
	int retval = 0;

	if ( path == NULL )
	{
		return -1;
	}

	if ( newpath == NULL )
	{
		return -1;
	}

	asm (
        " .arm\n\t"
        " mov r0, %[path]\n\t"
        " mov r1, %[newpath]\n\t"
	" mov r2, #0\n\t"
        " mov r7, #38\n\t"
        " svc #0\n\t"
        " mov %[retval], r0\n\t"
        : [retval] "+r" (retval)
        : [path] "r" (path), [newpath] "r" (newpath)
        : );

	return retval;
}

int appfile( uint8_t *path, uint8_t *data, uint32_t size )
{
	int retval = 0;
	int fd = 0;

	if ( path == NULL )
	{
		return -1;
	}

	if ( data == NULL )
	{
		return -1;
	}

	if ( size == 0 )
	{
		return 0;
	}

        asm (
        " .arm\n\t"
        " mov r0, %[path]\n\t"
        " mov r1, #0x2\n\t"
        " mov r2, #0\n\t"
        " mov r7, #5\n\t"
        " svc #0\n\t"
        " mov %[fd], r0\n\t"
        : [fd] "+r" (fd)
        : [path] "r" (path)
        : );

        if ( fd == -1 )
        {
                return retval;
        }

	asm (
	" .arm\n\t"
	" mov r0, %[fd]\n\t"
	" mov r1, #0\n\t"
	" mov r2, #2\n\t" // SEEK_END
	" mov r7, #19\n\t"
	" svc #0\n\t"
	: [fd] "+r" (fd)
	: : );

   	asm (
   	" .arm\n\t"
	" mov r0, %[fd]\n\t"
   	" mov r1, %[data]\n\t"
   	" mov r2, %[size]\n\t"
   	" mov r7, #4\n\t"
   	" svc #0\n\t"
   	" mov %[retval], r0\n\t"
   	: [retval] "+r" (retval)
   	: [data] "r" (data), [size] "r" (size), [fd] "r" (fd)
   	: );

   	if ( retval == -1)
   	{
        	return retval;
   	}

   	asm (
   	" .arm\n\t"
	" mov r0, %[fd]\n\t"
   	" mov r7, #6\n\t"
   	" svc #0\n\t"
   	" pop {r7}\n\t"
   	: [fd] "+r" (fd)
   	: : );
	
	return 0;
}

int retrfile( uint8_t *path, uint8_t **data, uint32_t* size )
{
	int retval = 0;
	int sz = 0;
	uint8_t *dt = NULL;
	struct stat st;
	struct stat *pst = &st;
	int fd = 0;

	if ( path == NULL )
	{
		return -1;
	}

	if (data == NULL)
	{
		return -1;
	}

	if ( size == NULL )
	{
		return -1;
	}

   	asm (
   	" .arm\n\t"
   	" push {r7}\n\t"
   	" mov r0, %[path]\n\t"
   	" mov r1, %[pst]\n\t"
   	" mov r7, #106\n\t"
   	" svc #0\n\t"
   	" mov %[retval], r0\n\t"
   	" pop {r7}\n\t"
   	: [retval] "+r" (retval)
   	: [path] "r" (path), [pst] "r" (pst)
   	: );

	if ( retval != 0 )
	{
		return retval;
	}

	sz = st.st_nlink;
	dt = malloc( sz );

	if ( dt == NULL )
	{
		return -1;
	}

	asm (
	" .arm\n\t"
   	" mov r0, %[path]\n\t"
   	" mov r1, #0\n\t"
   	" mov r2, #0\n\t"
   	" mov r7, #5\n\t"
   	" svc #0\n\t"
   	" mov %[fd], r0\n\t"
   	: [fd] "+r" (fd)
   	: [path] "r" (path)
   	: );

   	if ( fd == -1 )
   	{
        	return retval;
   	}


   	asm (
   	" .arm\n\t"
	" mov r0, %[fd]\n\t"
   	" mov r1, %[dt]\n\t"
   	" mov r2, %[sz]\n\t"
   	" mov r7, #3\n\t"
   	" svc #0\n\t"
   	" mov %[retval], r0\n\t"
   	: [fd] "+r" (fd), [retval] "+r" (retval)
   	: [dt] "r" (dt), [sz] "r" (sz)
   	: );

   	if ( retval == -1)
   	{
        	return retval;
   	}

   	asm (
   	" .arm\n\t"
	" mov r0, %[fd]\n\t"
   	" mov r7, #6\n\t"
   	" svc #0\n\t"
   	" pop {r7}\n\t"
   	: [fd] "+r" (fd)
   	: : );

	*data = dt;
	*size = sz;

	return 0;
}

int deldir( uint8_t *path)
{
	int retval = 0;

	if (path == NULL)
	{
		return -1;
	}

	asm (
   	" .arm\n\t"
   	" push {r7}\n\t"
   	" mov r0, %[path]\n\t"
   	" mov r7, #40\n\t"
   	" svc #0\n\t"
   	" mov %[retval], r0\n\t"
   	" pop {r7}\n\t"
   	: [retval] "+r" (retval)
   	: [path] "r" (path)
   	: );

	return retval;
}

int makedir( uint8_t *path)
{
	int retval = 0;
   	int mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IFREG;

	if ( path == NULL )
	{
		return -1;
	}
   
	asm (
   	" .arm\n\t"
   	" push {r7}\n\t"
   	" mov r0, %[path]\n\t"
   	" mov r1, %[mode]\n\t"
   	" mov r7, #39\n\t"
   	" svc #0\n\t"
   	" mov %[retval], r0\n\t"
   	" pop {r7}\n\t"
   	: [retval] "+r" (retval)
   	: [path] "r" (path), [mode] "r" (mode)
   	: );

	return retval;
}

int filesize( uint8_t *path)
{
   int retval = 0;
   struct stat st;
   struct stat *pst = &st;

   if ( path == NULL )
   {
      return -1;
   }

   asm (
   " .arm\n\t"
   " push {r7}\n\t"
   " mov r0, %[path]\n\t"
   " mov r1, %[pst]\n\t"
   " mov r7, #106\n\t"
   " svc #0\n\t"
   " mov %[retval], r0\n\t"
   " pop {r7}\n\t"
   : [retval] "+r" (retval)
   : [path] "r" (path), [pst] "r" (pst)
   : );

   if ( retval == 0 )
   {
      retval = st.st_nlink;
   } else
   {
      retval = -1;
   }

   return retval;

}

int deletefile( uint8_t *path)
{
   int retval = 0;

   if ( path == NULL )
   {
      return -1;
   }

   asm (
   " .arm\n\t"
   " push {r7}\n\t"
   " mov r0, %[path]\n\t"
   " mov r7, #10\n\t"
   " svc #0\n\t"
   " mov %[retval], r0\n\t"
   " pop {r7}\n\t"
   : [retval] "+r" (retval)
   : [path] "r" (path)
   : );

   return retval;
}

void setval( )
{
	asm(
	" .arm\n\t"
	" str r3,[r0,#0]\n\t");
}

int makefile( uint8_t *path, uint8_t *data, uint32_t len )
{
   int retval = 0;
   int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IFREG;
   int fd = 0;

   if ( path == NULL )
   {
      goto error;
   }

   if ( data == NULL)
   {
	goto error;
   }


   asm (
   " .arm\n\t"
   " push {r7}\n\t"
   " mov r0, %[path]\n\t"
   " mov r1, %[mode]\n\t"
   " mov r2, #0\n\t"
   " mov r7, #14\n\t"
   " svc #0\n\t"
   " mov %[retval], r0\n\t"
   : [retval] "+r" (retval)
   : [path] "r" (path), [mode] "r" (mode)
   : );

   if ( retval != 0 )
   {
	goto error;
   }

   asm (
   " .arm\n\t"
   " mov r0, %[path]\n\t"
   " mov r1, #2\n\t"	// O_RDWR
   " mov r2, #0\n\t"
   " mov r7, #5\n\t"
   " svc #0\n\t"
   " mov %[fd], r0\n\t"
   : [fd] "+r" (fd)
   : [path] "r" (path)
   : );

   if ( fd == -1 )
   {
	goto error;
   }

   asm (
   " .arm\n\t"
   " mov r1, %[data]\n\t"
   " mov r2, %[len]\n\t"
   " mov r7, #4\n\t"
   " svc #0\n\t"
   " mov %[retval], r0\n\t"
   : [retval] "+r" (retval)
   : [data] "r" (data), [len] "r" (len)
   : );
   
   if ( retval == -1)
   {
   	goto error;
   }

   asm (
   " .arm\n\t"
   " mov r0, %[fd]\n\t"
   " mov r7, #6\n\t"
   " svc #0\n\t"
   " pop {r7}\n\t"
   : [fd] "+r" (fd)
   : : );

   return 0;

   error: return -1;
}


int changedir( uint8_t *path )
{
   int retval = 0;

   if ( path == NULL )
   {
      return -1;
   }

   asm (
   " .arm\n\t"
   " push {r7}\n\t"
   " mov r0, %[path]\n\t"
   " mov r7, #12\n\t"
   " svc #0\n\t"
   " mov %[retval], r0\n\t"
   " pop {r7}\n\t"
   : [retval] "+r" (retval)
   : [path] "r" (path)
   : );

   return retval;
}

#define NMMR 0x524d4d4e	// NMMR - List directory
#define PRND 0x444e5250	// PRND - Retrieve file
#define ARRT 0x54525241	// ARRT - Quit
#define DNNR 0x524e4e44 // DNNR - Put a file
#define EVOL 0x4c4f5645 // EVOL - change directory
#define POID 0x44494f50 // POID - file size
#define ANNU 0x554e4e41 // ANNU - list current directory
#define SUPP 0x50505553 // SUPP - delete a file
#define FAIT 0x54494146 // FAIT - make directory
#define SUPD 0x44505553 // SUPD - delete directory
#define RNMR 0x524d4e52 // RNMR - rename file
#define AJTR 0x52544a41 // AJTR - append to file
#define MTHD 0x4448544d // MTHD - set mode I or B
#define PERS 0x53524550 // PERS - username
#define MOTS 0x53544f4d // MOTS - password
#define AUTH 0x48545541 // AUTH - authenticate

#define SUC 0
#define ERR -1

uint8_t * recvfilename( int connfd )
{
	uint32_t dirlen = 0;
	uint8_t *dir = NULL;
	uint8_t *fdir = NULL;

	if ( recvdata( connfd, (char*)&dirlen, 2) == -1 )
	{
		goto err;
	}

	if ( dirlen > 0x10 )
	{
		goto err;
	}

	dir = malloc( dirlen + 1 );

	if ( dir == NULL )
	{
		goto err;
	}

	memset( dir, 0x00, dirlen + 1);

	if ( recvdata(connfd, (char*)dir, dirlen) == -1 )
	{
		goto freerr;
	}

	fdir = appendbasedir( dir );
	fdir = sanitizedir( fdir );

	free(dir);
	return fdir;	

	freerr:
	free(dir);
	err:
	return NULL;
}

int handleAUTH( int connfd, uint8_t *buff, uint32_t bsize)
{
	int ufound = 0;
	int c = 0;

	if ( buff == NULL )
	{
		return -1;
	}

	if ( user == NULL || pass == NULL )
	{
		goto error;
	}

	while ( users[c] != NULL )
	{
		if ( strcmp( users[c], user ) == 0 )
		{
			ufound = 1;
			break;
		}
		c++;
	}

	if ( ufound == 0 )
	{
		goto error;
	}

	if ( strcmp( passes[c], pass ) == 0 )
	{
		authenticated = 1;
	} else
	{
		goto error;
	}

	strcpy( (char*)buff, "Auth successful\n");
	return 0;

	error:
	strcpy( (char*)buff, "go away\n");
	return -1;
}

int handleMOTS( int connfd, uint8_t *buff, uint32_t bsize)
{
	uint32_t size;

	if ( buff == NULL )
	{
		return -1;
	}

	if ( recvdata( connfd, (char*)&size, 1) == -1 )
	{
		goto freerr;
	}

	if ( size > 0x100 || size == 0)
	{
		goto error;
	}

	if ( pass != NULL )
	{
		free(pass);
		pass = NULL;
	}

	pass = malloc( size +1);
	if ( pass == NULL )
	{
		goto error;
	}

	memset( pass, 0x00, size +1);
	
	if ( recvdata( connfd, pass, size ) == -1 )
	{
		goto freerr;
	}

	strcpy( (char*)buff,  "Thanks for all the fish\n");
	return 0;

	freerr:
	free( pass );
	user = NULL;
	error:
	strcpy( (char*)buff, "oh noze\n");
	return -1;
}

int handlePERS( int connfd, uint8_t *buff, uint32_t bsize)
{
	uint32_t size;

	if ( buff == NULL )
	{
		return -1;
	}

	if ( recvdata( connfd, (char*)&size, 1) == -1 )
	{
		goto freerr;
	}

	if ( size > 0x100 || size == 0)
	{
		goto error;
	}

	if ( user != NULL )
	{
		free(user);
		user = NULL;
	}

	user = malloc( size +1);
	if ( user == NULL )
	{
		goto error;
	}

	memset( user, 0x00, size +1);
	
	if ( recvdata( connfd, user, size ) == -1 )
	{
		goto freerr;
	}

	strcpy( (char*)buff, "Yay for you\n");
	return 0;

	freerr:
	free( user );
	user = NULL;
	error:
	strcpy( (char*)buff, "Better luck next time\n");
	return -1;
}

int handleNMMR( int connfd, uint8_t *list, uint32_t size  )
{
	uint8_t *dir = NULL;

	dir = recvfilename( connfd );
	
	if ( dir == NULL )
	{
		goto error;
	}

	if ( getlist( dir, list, size ) != 0 )
	{
		goto freerr;
	}

	free(dir);
	return 0;

	freerr: free( dir );
	error:
	strcpy((char*)list, "Y U No Do Better?"); 
	return -1;
}

int handleDNNR( int connfd, uint8_t *buff, uint32_t bsize )
{
	int32_t fsize = 0;
	register uint8_t *f = NULL;
	register uint8_t *n = NULL;

	if ( buff == NULL )
	{
		return -1;
	}

	n = recvfilename( connfd );

	if ( n == NULL )
	{
		goto error;
	}

	if ( recvdata( connfd, (char*)&fsize, 2) == -1 )
	{
		goto freerr;
	}

	if ( dropped + fsize > MAXDROP )
	{
		goto freerr;
	}

	f = malloc( fsize );

	if ( f == NULL )
	{
		goto freerr;
	}

	memset( f, 0x00, fsize );

	if ( recvdata( connfd, (char*)f, fsize ) == -1 )
	{
		goto freerr2;
	}

	dropped += fsize;

	if ( makefile( n, f, fsize) == -1 )
	{
		goto freerr2;
	}

	strcpy( (char*)buff, "OOOh, good luck for you");
	free(n);
	free(f);
	return 0;
 
	freerr2: free( f );
	freerr: free( n );
	error:
	strcpy( (char*)buff, "Sorry, no luck");
	return -1;
}

int handlePRND( int connfd, uint8_t *buffer, uint32_t bsize )
{
	uint8_t *n = NULL;
	uint32_t fsize;
	uint8_t* f = NULL;

	if ( buffer == NULL )
	{
		return -1;
	}

	n = recvfilename( connfd );
	
	if ( n == NULL )
	{
		goto error;
	}

	if ( retrfile( n, &f, &fsize ) == -1 )
	{
		goto freerr;
	}

	send_data( connfd, (char*)f, fsize);

	free( f );
	free( n );
	strcpy( (char*)buffer, "Yay for you");

	return 0;

	freerr:
	free( n );
	error:
	strcpy( (char*)buffer, "Oh noes");
	return -1;
}

int handleEVOL( int connfd, uint8_t *buffer, uint32_t bsize )
{
	uint8_t *d = NULL;

	if ( buffer == NULL )
	{
		return -1;
	}

	d = recvfilename( connfd );

	if ( d == NULL )
	{
		goto error;
	}

	if ( changedir( d ) != 0 )
	{
		goto freerr;
	}
	
	strcpy((char*)buffer, "Done for you" );
	free(d);

	return 0;

	freerr:
	free(d);
	error:
	strcpy((char*)buffer, "I don't wanna wait for you to get better");
	return -1;
}

int handlePOID( int connfd, uint8_t *buffer, uint32_t bsize)
{
	uint32_t nsize = 0;
	uint8_t *n = 0;
	uint32_t i = 0;
	uint32_t j = 0;

	if ( buffer == NULL )
	{
		return -1;
	}

	n = recvfilename( connfd );

	if ( n == NULL )
	{
		goto error;
	}

	if ( (nsize = filesize( n )) == -1 )
	{
		goto freerr;
	}

	memset( buffer, 0x00, bsize);
	j = nsize;
	for( i = 1; j != 0; i++)
	{
		j = j / 10;
	}
	
	while ( i )
	{
		buffer[i-1] = (nsize % 10) + 0x30;
		i--;
		nsize /= 10;
	}

	free(n);
	return 0;

	freerr:
	free(n);
	error:
	strcpy( (char*)buffer, "something stinks");
	return -1;
}

int handleANNU( int connfd, uint8_t *buffer, uint32_t bsize )
{
	register uint32_t len = 0;

	if ( buffer == NULL )
	{
		return -1;
	}

	if (  getdir( buffer, bsize ) != 0 )
	{
		goto error;
	}

	len = strlen((char*)buffer);

	buffer[len] = '/';
	buffer[len+1] = 0;

	removebase( buffer );
	return 0;

	error:
	strcpy( (char*)buffer, "not this time" );
	return -1;
}

int handleSUPP( int connfd, uint8_t *buffer, uint32_t bsize )
{
	uint8_t *d = NULL;

	if ( buffer == NULL )
	{
		return -1;
	}

	d = recvfilename( connfd );

	if ( d == NULL )
	{
		goto error;
	}

	if ( deletefile( d ) != 0 )
	{
		goto freerr;
	}

	strcpy( (char*)buffer, "Finito, success");

	free(d);
	return 0;
	freerr:
	free(d);
	error:
	strcpy( (char*)buffer, "time is running out" );
	return -1;
}

int handleFAIT( int connfd, uint8_t *buffer, uint32_t bsize )
{
	uint8_t *d = NULL;

	if ( buffer == NULL )
	{
		return -1;
	}

	d = recvfilename( connfd );

	if ( d == NULL )
	{
		goto error;
	}

	if ( makedir( d ) != 0)
	{
		goto freerr;
	}

	strcpy( (char*)buffer, "Your wish is my command");
	free(d);
	return 0;

	freerr:
	free(d);
	error:
	strcpy((char*)buffer, "where am i");
	return -1;
}

int handleMTHD( int connfd, uint8_t *buffer, uint32_t bsize )
{
	uint32_t x = 0;

	if ( buffer == NULL)
	{
		return -1;
	}

	recvdata( connfd, (char*)&x, 1);

	if ( x == 'X')
	{
		mode = 1;
	} else if ( x == 'Y' )
	{
		mode = 0;	
	} else
	{
		goto error;
	}

	strcpy((char*)buffer, "yes yes yes yes yes");
	return 0;
	error:
	strcpy((char*)buffer, "no no no no no no no");
	return -1;
}

int handleSUPD( int connfd, uint8_t *buffer, uint32_t bsize )
{
	uint8_t *d = NULL;

	if ( buffer == NULL )
	{
		return -1;
	}

	d = recvfilename( connfd );

	if ( d == NULL )
	{
		goto error;
	}

	if ( deldir( d ) != 0)
	{
		goto freerr;
	}

	strcpy( (char*)buffer, "Nothing that I wouldn't do for you");
	free(d);
	return 0;

	freerr:
	free(d);
	error:
	strcpy((char*)buffer, "another one bytes the dust");
	return -1;
}

int handleRNMR( int connfd, uint8_t *buffer, uint32_t bsize )
{
	uint8_t *d = NULL;
	uint8_t *t = NULL;

	if ( buffer == NULL )
	{
		return -1;
	}

	d = recvfilename( connfd );

	if ( d == NULL )
	{
		goto error;
	}

	t = recvfilename( connfd );

	if ( t == NULL )
	{
		goto freerr;
	}

	if ( filerename( d, t ) != 0)
	{
		goto freerr2;
	}

	strcpy( (char*)buffer, "Anything else that I can do");
	free(d);
	free(t);
	return 0;

	freerr2:
	free(t);
	freerr:
	free(d);
	error:
	strcpy((char*)buffer, "another one bytes the dust");
	return -1;
}

int handleAJTR( int connfd, uint8_t *buffer, uint32_t bsize )
{
	uint8_t *d = NULL;
	uint32_t tsize = 0;
	uint8_t *t = NULL;

	if ( buffer == NULL )
	{
		return -1;
	}

	d = recvfilename( connfd );

	if ( d == NULL )
	{
		goto error;
	}

	if ( recvdata( connfd, (char*)&tsize, 2) == -1)
	{
		goto freerr;
	}

	if ( dropped + tsize > MAXDROP )
	{
		goto freerr;
	}

	dropped += tsize;

	t = malloc( tsize + 1);

	if ( t == NULL )
	{
		goto freerr;
	}

	memset( t, 0x00, tsize + 1);

	if ( recvdata( connfd, (char*)t, tsize) == -1 )
	{
		goto freerr2;
	}

	if ( appfile( d, t, tsize ) != 0)
	{
		goto freerr2;
	}

	strcpy( (char*)buffer, "You have two wishes left");
	free(d);
	free(t);
	return 0;

	freerr2:
	free(t);
	freerr:
	free(d);
	error:
	strcpy((char*)buffer, "i am running out of room here");
	return -1;
}

int handlereqs( int connfd )
{
	uint32_t bsize = 0x200;
	uint8_t buffer[0x200];
	uint32_t command = 0;

	while ( authenticated == 0 )
	{
		command = 0;
		memset(buffer, 0x00, 0x200);

		if ( recvdata( connfd, (char*)&command, 4) == -1 )
		{
			return -1;
		}

		switch (command)
		{
			case PERS:
				handlePERS( connfd, buffer, bsize );
				break;
			case MOTS:
				handleMOTS( connfd, buffer, bsize );
				break;
			case AUTH:
				handleAUTH( connfd, buffer, bsize);
				break;
			default:
				strcpy((char*)buffer, "NO\n");
				break;
		};

		send_string( connfd, (char*)buffer );
	}

	while ( command != ARRT )
	{
		if ( recvdata( connfd, (char*)&command, 4) == -1 )
		{
			return -1;
		}

		memset( buffer, 0x00, 0x200);
		switch (command)
		{
			case NMMR:
				handleNMMR( connfd, buffer, bsize );
				break;
			case DNNR:
				handleDNNR( connfd, buffer, bsize );
				break;
			case PRND:
				handlePRND( connfd, buffer, bsize  );
				break;
			case EVOL:
				handleEVOL( connfd, buffer, bsize );
				break;
			case POID:
				handlePOID( connfd, buffer, bsize );
				break;
			case ANNU:
				handleANNU( connfd, buffer, bsize );
				break;
			case SUPP:
				handleSUPP( connfd, buffer, bsize );
				break;
			case FAIT:
				handleFAIT( connfd, buffer, bsize);
				break;
			case SUPD:
				handleSUPD( connfd, buffer, bsize);
				break;
			case RNMR:
				handleRNMR( connfd, buffer, bsize);
				break;
			case MTHD:
				handleMTHD( connfd, buffer, bsize);
				break;
			case AJTR:
				handleAJTR( connfd, buffer, bsize);
				break;
			case ARRT:
				strcpy((char*)buffer, "so long");
				continue;
				break;
			default:
				strcpy((char*)buffer, "fail");
		};

		if ( mode == 0 )
		{
			send_string( connfd, (char*)buffer );
		} else
		{
			send_data( connfd, (char*)buffer, bsize);
		}
	}

	close( connfd );
	return 0;
}

int ff( int connfd )
{
	setupbase( );
	send_string( connfd, "---------------Welcome---------------\n");

	return handlereqs( connfd);
}

int main( int argc, char** argv)
{
	setvbuf( stdout, NULL, _IONBF, 0);
	ff( fileno(stdout) ); 
	return 0;
}

void getval( void )
{
    asm (
    " .arm\n\t"
    " ldr r0, [r3,#0]\n\t"
    " ldr r1, [r3,#4]\n\t"
    " ldr r2, [r3,#8]\n\t");
}
