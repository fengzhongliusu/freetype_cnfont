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

int glyph_width,glyph_height,font_width,font_height;
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

void show_image(void)
{
  int  i, j;
  for ( i = start_y; i < glyph_height; i++ )
  {
    for ( j = start_x; j < glyph_width; j++ )
      putchar(image[i][j] == 0?'-':image[i][j] <128?'*':'o');
    putchar( '\n' );
  }
}


void append_code(uint16_t code,FT_Face face,FILE* fw){
	uint16_t i,j;
	uint16_t cn_unic = get_unicode(code);
	FT_GlyphSlot slot = face->glyph;

	memset(image,0,sizeof(image));
	FT_Error error;
	error =	FT_Load_Char(face,cn_unic,FT_LOAD_RENDER);
	if(error) exit(1);

	 glyph_width = (face->glyph->metrics.width + face->glyph->metrics.horiBearingX)/64;
	 glyph_height = (face->glyph->metrics.height + face->glyph->metrics.vertBearingY)/64 + 1;
	 start_x = face->glyph->metrics.horiBearingX/64;
	 start_y = face->glyph->metrics.vertBearingY/64 + 1;
	 font_width = face->glyph->metrics.width/64;
	 font_height = face->glyph->metrics.width/64;

	 draw_bitmap( &slot->bitmap,slot->bitmap_left,HEIGHT- slot->bitmap_top );
	 printf("%04x %ld %ld %ld %ld\n",code,face->glyph->metrics.width/64,face->glyph->metrics.height/64,face->glyph->metrics.horiBearingX/64,face->glyph->metrics.vertBearingY/64);
	

	 fprintf(fw,"#define NXFONT_METRICS_%04x {%d, %d, %d, %d, %d, 0}\n",code,glyph_width,font_width,font_height,start_x,start_y-1);
	 fprintf(fw,"#define NXFONT_BITMAP_%04x {",code);
	 for ( i = start_y; i < glyph_height; i++ )
		for ( j = start_x; j < glyph_width; j++ )
			if(i == glyph_height-1 && j == glyph_width-1)
				fprintf(fw,"0x%02x",image[i][j]);
			else
				fprintf(fw,"0x%02x,",image[i][j]);

	 fprintf(fw,"}\n");
}


void gen_bitmap(const char *filename, FT_Face face)
{
  uint16_t i;
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


  for(i=0xb0a1;i<0xf7fe;){
		if(i - (i&0xff00) == 0xfe){
			i = (i&0xff00) +0x1a1;
		}
		else{
			i++;
		}
	    append_code(i,face,fw);
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

  FT_Set_Transform( face, &matrix, &pen );


  /*
  uint16_t w16ch = 0x963f;
  error = FT_Load_Char( face, w16ch, FT_LOAD_RENDER );
  printf("%ld %ld %ld %ld\n",face->glyph->metrics.width/64,face->glyph->metrics.height/64,face->glyph->metrics.horiBearingX/64,face->glyph->metrics.vertBearingY/64);
  glyph_width = (face->glyph->metrics.width + face->glyph->metrics.horiBearingX)/64;
  glyph_height = (face->glyph->metrics.height + face->glyph->metrics.vertBearingY)/64 + 1;
  start_x = face->glyph->metrics.horiBearingX/64;
  start_y = face->glyph->metrics.vertBearingY/64 + 1;
  draw_bitmap( &slot->bitmap,slot->bitmap_left,target_height - slot->bitmap_top );
  show_image();
  */

  gen_bitmap("50x50.c",face);
  //append_code(0xf7fe,face,NULL);

  FT_Done_Face    ( face );
  FT_Done_FreeType( library );

  iconv_close(cd);
  return 0;
}

/* EOF */
