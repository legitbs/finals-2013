#include "sharedfuncs.h"
#include <pthread.h>
#include <unistd.h>
#include "createfont.h"

#define GLYPHSZ 0x15
#define MAXGLYPH 0x5e

typedef struct glyph
{
    uint8_t *ops;
    uint32_t opsz;
    uint8_t data[21];
} glyph, *pglyph;

typedef struct font
{
    glyph glyphs[MAXGLYPH];
    uint32_t num_glyphs;
    uint8_t index[0x62];
} font, *pfont;

// Globals
uint8_t data[64];
uint8_t display[80*12];
uint32_t port_main = 4321;

unsigned char myfont[] = {
  0x1c, 0x69, 0x69, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x1f, 0xc0, 0x80, 0x82, 0x02, 0x00, 0x10, 0x00, 0x80, 0x00,
  0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x02, 0x00, 0x14, 0x00, 0x88, 0x04, 0x10, 0x3f, 0xe1, 0x00, 0x48, 0x00,
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80,
  0x81, 0x02, 0x02, 0x0f, 0xf0, 0x20, 0x20, 0x80, 0x43, 0xfe, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x81, 0x02,
  0x00, 0x08, 0x00, 0x20, 0x00, 0x81, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x81, 0x02, 0x04, 0x08,
  0x10, 0x20, 0x40, 0x81, 0x03, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x3f, 0xc0, 0x80, 0x02, 0x00, 0x0f, 0xc0, 0x20,
  0x00, 0x80, 0x03, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x3f, 0xc0, 0x80, 0x02, 0x00, 0x0f, 0xc0, 0x20, 0x00, 0x80,
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x0f, 0x00, 0x81, 0x02, 0x00, 0x09, 0xe0, 0x20, 0x40, 0x81, 0x00, 0xf0,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x40,
  0x81, 0x02, 0x04, 0x0f, 0xf0, 0x20, 0x40, 0x81, 0x02, 0x04, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xc0, 0x18, 0x00,
  0x60, 0x01, 0x80, 0x06, 0x00, 0x18, 0x03, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00, 0x04, 0x00,
  0x10, 0x20, 0x40, 0x81, 0x01, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x20, 0x40, 0x84, 0x02, 0x40, 0x0a, 0x00, 0x22,
  0x00, 0x84, 0x02, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x20, 0x00, 0x80, 0x02, 0x00, 0x08, 0x00, 0x20, 0x00, 0x80,
  0x03, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x20, 0x10, 0xc0, 0xc2, 0x85, 0x09, 0x24, 0x23, 0x10, 0x80, 0x42, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20,
  0xc0, 0x82, 0x82, 0x09, 0x08, 0x22, 0x20, 0x84, 0x82, 0x0e, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x80, 0x81, 0x02,
  0x04, 0x08, 0x10, 0x20, 0x40, 0x81, 0x01, 0xf8, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x81, 0x02, 0x04, 0x0f,
  0xe0, 0x20, 0x00, 0x80, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1f, 0x80, 0x81, 0x02, 0x04, 0x08, 0x10, 0x20,
  0x40, 0x87, 0x01, 0xfb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x3f, 0x80, 0x81, 0x02, 0x04, 0x0f, 0xe0, 0x28, 0x00, 0x90,
  0x02, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x1f, 0x80, 0x81, 0x02, 0x00, 0x07, 0xe0, 0x00, 0x40, 0x81, 0x01, 0xf8,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xc0,
  0x18, 0x00, 0x60, 0x01, 0x80, 0x06, 0x00, 0x18, 0x00, 0x60, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x40, 0x81, 0x02,
  0x04, 0x08, 0x10, 0x20, 0x40, 0x81, 0x01, 0xf8, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x05, 0x00, 0x22, 0x01, 0x04,
  0x08, 0x08, 0x40, 0x12, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x80, 0x82, 0x02, 0x08, 0x08, 0x22,
  0x20, 0x94, 0x81, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x10, 0x40, 0x22, 0x00, 0x50, 0x00, 0x80, 0x04, 0x80, 0x21,
  0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x20, 0x10, 0x40, 0x80, 0x84, 0x01, 0x20, 0x03, 0x00, 0x0c, 0x00, 0x30,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xf0,
  0x00, 0x80, 0x04, 0x00, 0x20, 0x01, 0x00, 0x08, 0x00, 0x7f, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x96, 0x96, 0x96, 0x00,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x05,
  0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
  0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
  0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
  0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x01, 0x01, 0x01
};


int ff( int connfd );
void *thread_function( void *arg );

void write_glyph_to_display( uint8_t*display, uint8_t*glyph, uint32_t pos )
{
    uint32_t left_base = 0;
    uint32_t i,j,gindex,gbyte, gbitindex, gbit;
    uint32_t display_x, display_y;

    if ( glyph == NULL )
    {
        //printf("[ERROR] write_glyph_to_display() invalid glyph pointer.\n");
        return;
    }

    if ( display == NULL )
    {
        //printf("[ERROR] write_glyph_to_display() invalid display pointer.\n");
        return;
    }
    
    if ( pos > 4 )
    {
        //printf("[ERROR] write_glyph_to_display() invalid position.\n");
        return;
    }

    // Set the left boundary
    left_base = pos * 15;    

    for ( i = 0; i < 12; i++)
    {
        for ( j = 0; j < 14; j++ )
        {
            // Calculate which index we are on in the glyph buffer
            gindex = ( (i * 14) + j ) / 8;
            
            // Get the byte we are currently working on.
            gbyte = glyph[gindex];

            // Get the bit that we are interested in.
            gbitindex = ( (i * 14) + j ) % 8;

            gbit = gbyte >> (7 - gbitindex) & 0x1;

            //printf("Index : %d Byte: %x BitIndex: %d Bit: %x\n", gindex, gbyte, gbitindex, gbit);

            // Calculate the display position
            display_x = left_base + j;
            display_y = i;

            // Place the '.'
            if ( gbit )
                display[(display_y * 80) + display_x] = '.';
            else
                display[(display_y * 80) + display_x] = ' ';
                
        }
    }

    for ( i = 0; i < 12; i++ )
    {
        display[ ( 80 * i ) + 79 ] = '\n';
    }

    return;
}

void bit_array_to_bit_glyph( uint8_t*cglyph, uint8_t*bglyph)
{
    uint32_t i;
    uint32_t byte_index;
    uint32_t bit_index;

    if ( cglyph == NULL )
        return;

    if ( bglyph == NULL )
        return;

    memset( bglyph, 0x00, 21 );

    i = 0;
    while ( cglyph[i] != 0xff )
    {
        byte_index = cglyph[i] / 8;

        bit_index = cglyph[i] % 8;

        bglyph[byte_index] |= ( 1 << (7-bit_index));

        i++;
    }

    return;
}

void font_to_glyphs( uint8_t *data, pfont new_font )
{
    uint32_t i, offset;
    uint32_t opsz;

    if ( data == NULL )
        return;

    if ( new_font == NULL )
        return;

    new_font->num_glyphs = data[0];
    //printf("num glyphs before copy: %x\n", new_font->num_glyphs);

    if ( new_font->num_glyphs > 0x5e )
    {
        //printf("too many glyphs send\n");
        return;
    }

    if (memcmp( data+1, "\x69\x69\x69", 3 ) != 0 )
    {
        //printf("[ERROR] font_to_glyphs() bad glyph start magic\n");
        return;
    }

    offset = 4;
    for ( i = 0; i < new_font->num_glyphs; i++ )
    {
        memcpy( new_font->glyphs[i].data, data + offset, 21 );
        offset += 21;
        opsz = data[offset];
        offset++;
        new_font->glyphs[i].opsz = opsz;
       
        if ( opsz > 0 )
        {
            new_font->glyphs[i].ops = malloc(opsz);
            memcpy( new_font->glyphs[i].ops, data + offset, opsz );
            offset += opsz;
        } else
        {
            new_font->glyphs[i].ops = NULL;
        }
    }

    if ( memcmp( data+offset, "\x96\x96\x96", 3) != 0)
    {
        //printf("[ERROR] font_to_glyphs() bad glyph end magic\n");
        return;
    }

    offset += 3;
    memcpy( new_font->index, data+offset, 0x5e );

    //printf("num glyphs after copy: %x\n", new_font->num_glyphs);
    return;
}

void print_display( uint8_t *display)
{
    int i,j;

    if ( display == NULL )
    {
        return;
    }

    for ( i = 0; i < 12; i++)
    {
	    for (j = 0; j <80; j++)
	    {
	        //printf("%c", display[ (i*80) +j]);
	    }
    }

    return;
}

pglyph exec_ops( pfont font_s, pglyph glyph_s )
{
    uint32_t op_index = 0;
    uint32_t bitone;
    uint32_t bittwo;
    uint32_t i;

    if ( font_s == NULL )
    {
        return NULL;
    }

    if ( glyph_s == NULL )
    {
        return NULL;
    }

    if ( glyph_s->ops == NULL )
    {
        return NULL;
    }

    while ( op_index < glyph_s->opsz )
    {
        switch( glyph_s->ops[op_index] )
        {
            case 0x80:
                bitone = glyph_s->ops[++op_index];
                //printf("[INFO] Merging with glyph %x\n", bitone);
                for ( bittwo = 0; bittwo < 21; bittwo++)
                {
                    glyph_s->data[bittwo] |= font_s->glyphs[bitone].data[bittwo];
                }
                op_index++;
                break;
            case 0x90:
                bitone = glyph_s->ops[++op_index];
                //printf("[INFO] Setting bit %x. Byte: %x Bit: %x Value: %x\n", bitone, bitone/8, bitone%8, glyph_s->data[bitone/8]);
                glyph_s->data[bitone/8] |= (1<< ( (bitone%8)));
                op_index++;
                break;
            case 0x91:
                //printf("[INFO] Unetting bit %x. Byte: %x Bit: %x Value: %x\n", bitone, bitone/8, bitone%8, glyph_s->data[bitone/8]);
                bitone = glyph_s->ops[++op_index];
                glyph_s->data[bitone/8] &= (0xff^(1<< ( (bitone%8))));
                op_index++;
                break;
            case 0xb0:
                bitone = glyph_s->ops[++op_index];
                bittwo = glyph_s->ops[++op_index];
                //printf("[INFO] Setting range %.2x:%.2x\n", bitone, bittwo);
                for ( i = bitone; i < bittwo; i++)
                {
                    glyph_s->data[i/8] |= (1<< ( 7 - (i%8)));
                }
                op_index++;
                break;
            case 0xb1:
                bitone = glyph_s->ops[++op_index];
                bittwo = glyph_s->ops[++op_index];
                //printf("[INFO] Unsetting range %.2x:%.2x\n", bitone, bittwo);
                for ( i = bitone; i < bittwo; i++)
                {
                    glyph_s->data[i/8] &= (0xff^(1<< ( 7 - (i%8))));
                }
                op_index++;
                break;
            case 0xb2:
                bitone = glyph_s->ops[++op_index];
                glyph_s->data[bitone/8] ^= ( (~(glyph_s->data[bitone/8])) & (0xff&(1<<(7-(bitone%8)))));
                op_index++;
                break;
            case 0xb3:
                bitone = glyph_s->ops[++op_index];
                bittwo = glyph_s->ops[++op_index];
                //printf("[INFO] Inverting range %x:%x\n", bitone, bittwo);
                for ( i = bitone; i < bittwo; i++)
                {
                    glyph_s->data[i/8] ^= ( (~(glyph_s->data[i/8])) & (0xff&(1<<(7-(i%8)))));
                }
                op_index++;
                break;
            default:
                //printf("[ERROR] invalid opcode\n");
                return NULL;
                break;
        };
    }

    return glyph_s;
}

void update_display( pfont font_s )
{
    glyph update_glyph;
    register uint32_t i;
    register uint32_t gindex;

    if ( font_s == NULL )
    {
        return;
    }

    for ( i = 0; i < 5; i++ )
    {
        if ( data[i] < 0x20 || data[i] > 0x7e )
        {
            data[i] = '?';
        }

        gindex = font_s->index[ data[i] - 0x20 ];
        
        if ( gindex > font_s->num_glyphs )
        {
            continue;
        }

	memcpy( &update_glyph, &(font_s->glyphs[gindex]), sizeof(glyph));

        if ( update_glyph.opsz > 0 )
        {
            if ( exec_ops( font_s, &update_glyph ) != NULL )
            {
                write_glyph_to_display( display, update_glyph.data, i );
            }
        } else
        {
            write_glyph_to_display( display, update_glyph.data, i );
        }
    }

    return;
}

uint8_t *get_new_font_data( uint32_t connfd )
{
    uint32_t retval = 0;
    uint8_t *fontdata = NULL;
    uint32_t size;

    //printf("[INFO] get_new_font_data() entrance\n");

    //retval = recvdata( connfd, (char*)&size, 4 );
    retval = recvdata( fileno(stdin), (char*)&size, 4 );

    if ( retval == 0 )
    {
        //printf("[ERROR] get_new_font_data() recvdata() failed\n");
        return NULL;
    }

    if ( size > ( 0x5e + (0x5e*22) + 7 + (255*0x5e)) )
    {
        //send_string( connfd, "[!] Too big.\n");
        send_string( fileno(stdout), "[!] Too big.\n");
        return NULL;
    }

    //printf("About to receive %x bytes\n", size);
    fontdata = (uint8_t*)malloc( size );

    //retval = recvdata( connfd, (char*)fontdata, size);
    retval = recvdata( fileno(stdin), (char*)fontdata, size);
    //printf("Received data\n");
    if ( retval == 0)
    {
        free(fontdata);
        return NULL;
    }

    return fontdata;
}

uint32_t handle_menu ( uint32_t connfd )
{
    uint32_t retval = 1;
    uint8_t *new_font = NULL;
    font font_s;

    // Set up the default font
    font_to_glyphs( myfont, &font_s );

    // Write the initial data to the display
    update_display( &font_s );

    while ( retval != 0 )
    {
    	//send_string(connfd, "1 ) Change display text.\n2 ) Upload a new font.\n3 ) Exit.\n--> ");
    	send_string(fileno(stdout), "1 ) Change display text.\n2 ) Upload a new font.\n3 ) Exit.\n--> ");

    	//if ( recv_until( connfd, (char*)&retval, 2, '\x0a' ) == 0 )
    	if ( recv_until( fileno(stdin), (char*)&retval, 2, '\x0a' ) == 0 )
    	{
            //printf("[ERROR] handle_menu() recv_until() choice failed\n");
        	return -1;
    	}

    	retval = (retval&0xff) - 0x30;

    	switch (retval)
    	{
            case 1:
                //if ( recv_until( connfd, (char*)data, 6, '\x0a') == 0 )
                if ( recv_until( fileno(stdin), (char*)data, 6, '\x0a') == 0 )
		        {
                    //printf("[ERROR] handle_menu() recv_until() display text failed\n");
                    return -1;
                }
                update_display( &font_s );
            	break;
            case 2:
                new_font = get_new_font_data( connfd );
		        if ( new_font != NULL )
                {
                    font_to_glyphs( new_font, &font_s );
                    update_display( &font_s );
                }
            	break;
            case 3:
                //send_string( connfd, "Thank you for playing.\n");
                send_string( fileno(stdout), "Thank you for playing.\n");
                retval = 0;
            	break;
            default:
                //send_string( connfd, "[!] Invalid response.\n");
                send_string( fileno(stdout), "[!] Invalid response.\n");
                break;
        };
    }

    //printf("[INFO] handle_menu() exiting\n");
    return retval;
}

int main( int argc, char **argv)
{
//    int fd = SetupSock( 4321, AF_INET, "eth0" );

//    accept_loop( fd, ff, "lonetuna" );
    ff(0);
    return 0;
}


int ff (int connfd )
{
    pthread_t tid;
    int err;
    int secondaryfd;
    int port_second;
    uint8_t port_string[48];

    // Seed rand and setup secondary port/connection
    srand(time(NULL));
    port_second = (rand() % 10000) + 50000;

    // loop until an open port is available.
    while ( ( secondaryfd = SetupSock( port_second, AF_INET, "eth0") ) == 0 )
    {
        //printf("Port %d unavailable\n", port_second);
        port_second = (rand() % 10000) + 50000;
    }     

    // Setup the string to send
    snprintf( (char*)port_string, 48, "Connect to %d to view the display.\n", port_second);
    
    // Set up the initial data string
    memcpy(data, "HELLO", 16);

    // Initialize the display to all spaces
    memset(display, 0x20, 80*12);

    // Create the thread to handle sending the display to the user
    err = pthread_create(&tid, NULL, &thread_function, &secondaryfd);
   
    if (err != 0)
    {
        printf("[ERROR] Failed to create thread.\n");
        return -1;
    } else
    {
        //send_string(connfd, (char*)port_string );
        send_string(fileno(stdout), (char*)port_string );
    }

    handle_menu( connfd );

    //printf("Ending ff\n");
//    close(connfd);
    close(secondaryfd);
    return 0;
}

void *thread_function( void *arg )
{
    struct sockaddr_in6 sin6;
    socklen_t sinlen = sizeof(sin6);
    int fd = ((int*)arg)[0];
    int conn = accept( fd, (struct sockaddr *)&sin6, &sinlen);

    //printf("inside thread %x\n", conn);

    while( memcmp(data, "end", 3) != 0 )
    {
        send_data( conn, (char*)display, 80*12 );
        sleep(1);
    }

    //printf("the end is nigh\n");
    close(conn);
    close(fd);
    return NULL;
}
