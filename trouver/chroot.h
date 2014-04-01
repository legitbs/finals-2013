#ifndef __INITSETUP__
#define __INITSETUP__

int setupbase( void );
uint8_t *appendbasedir( uint8_t * dir);
uint8_t *sanitizedir( uint8_t *dir);
uint8_t *removebase( uint8_t *dir);

#endif
