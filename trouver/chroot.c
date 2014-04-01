#include "common.h"

uint8_t basedir[0x100];
uint32_t blen = 0;

uint8_t *appendbasedir( uint8_t *dir )
{
	uint8_t *fdir = NULL;
	uint32_t len = 0;

	if (dir == NULL)
	{
		goto error;
	}

	len = strlen((char*)dir);

	fdir = malloc( blen + len + 1);
	
	if ( fdir == NULL)
	{
		goto error;
	}

	memset( fdir, 0x00, blen + len + 1);

	memcpy( fdir, basedir, blen );
	memcpy( fdir + blen, dir, len );

	return fdir;

	error:
	return NULL;
}
	
uint8_t * sanitizedir( uint8_t * dir )
{
   int c = 0;
   int d = 0;
   int x = 0;
   int status = 0;

   if ( dir == NULL )
   {
      return NULL;
   }


   while ( dir[c] != '\x00' )
   {
      x = dir[c];

      if ( x == '/' )
      {
         if ( status == 1 )
         {
            c++;
         } else
         {
            status = 1;
            dir[d] = x;
            c++;
            d++;
         }
      } else if ( x != '.' )
      {
         status = 0;
         dir[d] = x;
         c++;
         d++;
      } else
      {
         c++;
      }
   }

   dir[d] = '\x00';

   return dir;
}


int setupbase( )
{
	if ( getcwd( (char*)basedir, 0x100) == NULL )
	{
		goto error;
	}

	blen = strlen( (char*)basedir );

	if ( blen + 10 < 0x100 )
	{
        	memcpy( basedir+blen, "/tmp/tmpXXXXXX",14);
		blen += 14;
	} else
	{
		goto error;
	}

        mktemp((char*)basedir);

        if (basedir[0] == 0)
        {
                goto error;
        }

        basedir[blen] = '/';
        blen++;

	if ( mkdir((const char*) basedir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP) == -1 )
	{
		goto error;
	}

	chdir( (char*)basedir );

        return 0;
	error:
	return -1;
}

uint8_t *removebase( uint8_t * dir)
{
	register uint32_t max = 0;

	if ( dir == NULL)
	{
		goto error;
	}

	if ( memcmp( dir, basedir, blen) != 0 )
	{
		goto error;
	}


	max = strlen((char*)dir);

	memcpy( dir, dir+blen-1, max-blen+1);
	memset( dir+(max-blen)+1, 0x00, blen);
	return dir;
	error:
	dir[0] = 0;
	return NULL;
}
