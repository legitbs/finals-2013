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

int main (int argc, char **argv)
{
    uint32_t size = 7 + (28*21) + 28 + 0x5e;
    uint8_t fdata[size];
    uint32_t offset = 0;
    uint32_t i;
    FILE *f;
    memset( fdata, 0x00, size );
    printf("Size: %x\n", size);
    fdata[0] = 28;
    memcpy( fdata+1, "\x69\x69\x69", 3);
    offset = 4;
    // these add 22 to add a padding 00 indicating no ops
    bit_array_to_bit_glyph( space_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( quest_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( a_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( b_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( c_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( d_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( e_bits, fdata+offset);
    printf("\nE: ");
    for ( i = offset; i < offset+21;i++)
        printf(" %x ", fdata[i]);
    offset += 22;
    bit_array_to_bit_glyph( f_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( g_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( h_bits, fdata+offset);
    printf("\nH: ");
    for ( i = offset; i < offset+21;i++)
        printf(" %x ", fdata[i]);
    offset += 22;
    bit_array_to_bit_glyph( i_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( j_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( k_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( l_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( m_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( n_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( o_bits, fdata+offset);
    printf("\nO: ");
    for ( i = offset; i < offset+21;i++)
        printf(" %x ", fdata[i]);
    offset += 22;
    bit_array_to_bit_glyph( p_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( q_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( r_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( s_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( t_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( u_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( v_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( w_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( x_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( y_bits, fdata+offset);
    offset += 22;
    bit_array_to_bit_glyph( z_bits, fdata+offset);
    offset += 22;
    memcpy( fdata+offset, "\x96\x96\x96", 3);
    offset += 3;

    // set the default to question mark
    memset( fdata+offset, 0x01, 0x5e );

    printf("Offxet %x\n", offset);

    // space is the first glyph
    fdata[offset] = 0;

    // Setup lowercase letters (yes, my font is all uppercase though)
    for ( i = 0x61; i < 0x7b; i++ )
    {
        fdata[offset+(i-0x20)] = i-0x5f;
    }

    // setup uppercase
    for ( i = 0x41; i < 0x5b; i++ )
    {
        fdata[offset+(i-0x20)] = i-0x3f;
    }
 
    f = fopen("myfont", "wb");
    fwrite( fdata, size, 1, f);
    fclose(f);
    return 0;
}
