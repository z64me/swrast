#include <stdlib.h>
#include <stdint.h>
#include <SDL/SDL.h>

#include <font.h>

struct zuiSprite {
	SDL_Surface *surface;
};

struct zuiSprite *
zuiSprite_new(unsigned char *rgba8888, int w, int h)
{
	if (!rgba8888)
		return 0;
	struct zuiSprite *sprite = malloc(sizeof(*sprite));
	if (!sprite)
		return 0;
	
	uint32_t rmask, gmask, bmask, amask;

	/* SDL interprets each pixel as a 32-bit number, so our masks must depend
	 on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
	
	sprite->surface =
		SDL_CreateRGBSurfaceFrom(
			rgba8888
			, w, h
			, 32
			, w * 4
			, rmask, gmask, bmask, amask
		)
	;
	
	if (!sprite->surface)
		return 0;
	
	/*SDL_SaveBMP(sprite->surface, "wow.bmp");*/
	
	return sprite;
}

void
zui_rect(
	void *dest
	, int x, int y, int w, int h
	, uint32_t color
)
{
	SDL_Rect rect = {x, y, w, h};
	SDL_FillRect(dest, &rect, color);
}

void
zuiSprite_draw(
	void *dest
	, struct zuiSprite *sprite
	, int cx, int cy, int cw, int ch
	, int dx, int dy
)
{
	SDL_Surface *surf = sprite->surface;
	SDL_Surface *screen = dest;
	if (cw == 0)
		cw = surf->w;
	if (ch == 0)
		ch = surf->h;
	SDL_Rect srect = {cx, cy, cw, ch};
	SDL_Rect drect = {dx, dy, cw, ch};
	SDL_BlitSurface(surf, &srect, screen, &drect);
}

void
zuiSprite_debugFont_draw(
	void *dest
	, struct zuiSprite *font
	, char *str
	, int x
	, int y
)
{
	int ox = x;
	int oy = y;
	int fonttype = 0;
	
	for (int a=0; str[a]; ++a)
	{
		unsigned char c = str[a];
		int cy;
		if( (c==0x8C || c==0x8D) && ( !a || (a && ((unsigned char)str[a-1])!=0x8E) ) )
			fonttype = (c==0x8D);
		else switch(c) {
			case 0x0A: // newline
				y += 8;
				x = ox;
				continue;
				break;
			case 0xFF: // do nothing
				break;
			default:   // text
				/* Japanese character set text; hiragana & katakana */
				if(c==0x8E) {
					a++;
					c = str[a];
					/* hiragana = add 0x20 if character >=0xC0;
					   otherwise, subtract 0x20 */
					c += ((c>=0xC0)?0x20:-0x20)*fonttype;
				}
				cy = c * 8;
				zuiSprite_draw(dest, font, 0, cy, 8, 8, x, y);
				break;
		}
		x += 8;
	}
}

struct zuiSprite *
zuiSprite_debugFont(void)
{
	long int bytes = sizeof(EGh_bin) * 8 * 4;
	unsigned char *result = malloc(bytes * 4);
	if (!result)
		return 0;
	
	unsigned char *w;
	long int a;
	for (a = 0; a < 64 * 256; a++) {
		int x = a % 64;
		int y = a / 64;
		x = a & 7;
		y = a / 64;
		y += (64-8) * (a / (64*8));
		y += 8 * ((a%64) / 8);
		//if ((a%W) &7)
		//if (a >= 8 * 64*3)
		//	break;
		w = result + y * 8 * 4 + x * 4;
		int byte=((a&7)/2)+4*(a/32), bit=((a%32)/8)+4*!(a&1);
		if ((EGh_bin[byte]>>bit)&0x1)
			w[0] = w[1] = w[2] = w[3] = 0xFF;
		else
			w[0] = w[1] = w[2] = w[3] = 0x00;
	}
	
	return zuiSprite_new(result, 8, 2048);
}


