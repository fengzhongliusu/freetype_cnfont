/* example1.c                                                      */
/*                                                                 */
/* This small program shows how to print a rotated string with the */
/* FreeType 2 library.                                             */


#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <iconv.h>
#include <stdlib.h>
#include "ttf.h"

#include <ft2build.h>
#include FT_FREETYPE_H


#define WIDTH   64
#define HEIGHT  64


/* origin is the upper left corner */
unsigned char image[HEIGHT][WIDTH];

int glyph_width,glyph_height;
int start_x,start_y;


/* Replace this function with something useful. */

	void
draw_bitmap( FT_Bitmap*  bitmap,
		FT_Int      x,
		FT_Int      y)
{
	FT_Int  i, j, p, q;
	FT_Int  x_max = x + bitmap->width;
	FT_Int  y_max = y + bitmap->rows;

	for ( i = x, p = 0; i < x_max; i++, p++ )
	{
		for ( j = y, q = 0; j < y_max; j++, q++ )
		{
			if ( i < 0      || j < 0       ||
					i >= WIDTH || j >= HEIGHT )
				continue;

			image[j][i] |= bitmap->buffer[q * bitmap->width + p];
		}
	}
}

uint16_t get_unicode(uint16_t gb2312){
	uint8_t ch_H,ch_L;
	uint32_t ch_index,count;
	FILE* f_read;

	ch_H = gb2312 >> 8;
	ch_L = gb2312 - (ch_H << 8);
	ch_index = (ch_H - 0xb0)*94 + ch_L - 0xa1;

	return unic[ch_index];
}

void
show_image( void )
{
  int  i, j;
  for ( i = start_y; i < glyph_height; i++ )
  {
    for ( j = start_x; j < glyph_width; j++ )
      putchar(image[i][j] == 0?'-':image[i][j] <128?'*':'o');
    putchar( '\n' );
  }
}

void gen_bitmap(const char *filename)
{
  int i,j,k,num;
  FILE *fw = fopen(filename,"a");

  fprintf(fw,"#ifndef __GRAPHICS_NXFONTS_NXFONTS_CN50X50_H\n");
  fprintf(fw,"#define __GRAPHICS_NXFONTS_NXFONTS_SANS28X37_H\n");
  fprintf(fw,"#define NXFONT_ID         FONTID_CN50X50\n");
  fprintf(fw,"#define NXFONT_MIN7BIT    %d\n",33);
  fprintf(fw,"#define NXFONT_MAX7BIT    %d\n",126);
  fprintf(fw,"#define NXFONT_MIN8BIT	%d\n",0xb0a1);
  fprintf(fw,"#define NXFONT_MAX8BIT	%d\n",0xb0a1);
  fprintf(fw,"#define NXFONT_MAXHEIGHT	%d\n",64);
  fprintf(fw,"#define NXFONT_MAXWIDTH	%d\n",64);
  fprintf(fw,"#define NXFONT_SPACEWIDTH	%d\n",6);


  for ( i = start_y; i < glyph_height; i++ )
  {
    for ( j = start_x; j < glyph_width; j++ );
  }

  fprintf(fw,"#undef EXTERN	\
		  \n#if defined(__cplusplus)	\
		  \n#define EXTERN extern \"C\"	\
		  \nextern \"C\" {	\
		  \n#else	\
		  \n#define EXTERN extern	\
		  \n#endif	\
		  \n#undef EXTERN	\
		  \n#if defined(__cplusplus)	\
		  \n}	\
		  \n#endif	\
		  \n#endif\n");

  fclose(fw);
}


int
main( int     argc,
      char**  argv )
{
  FT_Library    library;
  FT_Face       face;

  FT_GlyphSlot  slot;
  FT_Matrix     matrix;                 /* transformation matrix */
  FT_Vector     pen;                    /* untransformed origin  */
  FT_Error      error;

  char*         filename;
  char*         text;

  double        angle;
  int           target_height;
  int           n, num_chars;

  iconv_t       cd;

  cd = iconv_open("utf-16","utf-8");

  if ( argc != 3 )
  {
    fprintf ( stderr, "usage: %s font sample-text\n", argv[0] );
    exit( 1 );
  }

  filename      = argv[1];                           /* first argument     */
  text          = argv[2];                           /* second argument    */
  num_chars     = strlen( text ) / 3;
  angle         = ( 0.0 / 360 ) * 3.14159 * 2;      /* use 25 degrees     */
  target_height = HEIGHT;

  error = FT_Init_FreeType( &library );              /* initialize library */
  /* error handling omitted */

  error = FT_New_Face( library, filename, 0, &face );/* create face object */
  /* error handling omitted */

  /* use 50pt at 100dpi */
  //error = FT_Set_Pixel_Sizes( face, 60, 60);                /* set character size */
  error = FT_Set_Char_Size( face, 40 * 64, 0, 102, 0 );                /* set character size */
  /* error handling omitted */

  /*if(FT_HAS_VERTICAL(face)){
	  printf("########################################\n");
	  printf("%ld %ld \n",face->glyph->metrics.width,face->glyph->metrics.height);
  }*/

  slot = face->glyph;

  /* set up matrix */
  matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
  matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
  matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
  matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );

  /* the pen position in 26.6 cartesian space coordinates; */
  /* start at (300,200) relative to the upper left corner  */
  pen.x = 0 * 64;
  pen.y = ( target_height - 50 ) * 64;

  for ( n = 0; n < num_chars; n++ )
  {
    uint32_t wch;
    uint16_t w16ch;
    char *inbuf, *outbuf;
    size_t insize, outsize;
    int k;

    /* set transformation */
    FT_Set_Transform( face, &matrix, &pen );

    inbuf = (char *)(text + 3*n);
    outbuf = (char *)&wch;
    insize = 3, outsize = 4;
    iconv(cd, &inbuf, &insize, &outbuf, &outsize);
    /*
    printf("%04x -> %04x(%d)\n", text[n], wch, outsize);

    for (k=0; k<strlen(text); ++k)
	    printf("%02x ", (unsigned char)text[k]);
    printf("\n");
    */
    if (outsize)
       w16ch = wch & 0xffff;
    else
       w16ch = wch >> 16;
    /* load glyph image into the slot (erase previous one) */
	w16ch = 0x41;
    error = FT_Load_Char( face, w16ch, FT_LOAD_RENDER );
	printf("%ld %ld %ld %ld\n",face->glyph->metrics.width/64,face->glyph->metrics.height/64,face->glyph->metrics.horiBearingX/64,face->glyph->metrics.vertBearingY/64);
    if ( error )
      continue;                 /* ignore errors */

    /* now, draw to our target surface (convert position) */
	glyph_width = (face->glyph->metrics.width + face->glyph->metrics.horiBearingX)/64;
	glyph_height = (face->glyph->metrics.height + face->glyph->metrics.vertBearingY)/64 + 1;
	start_x = face->glyph->metrics.horiBearingX/64;
	start_y = face->glyph->metrics.vertBearingY/64 + 1;

    draw_bitmap( &slot->bitmap,
                 slot->bitmap_left,
                 target_height - slot->bitmap_top );

    /* increment pen position */
    pen.x += slot->advance.x;
    pen.y += slot->advance.y;
  }

  show_image();

  FT_Done_Face    ( face );
  FT_Done_FreeType( library );

  iconv_close(cd);
  return 0;
}

/* EOF */
