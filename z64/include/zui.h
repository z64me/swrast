#ifndef Z64_ZUI_H_INCLUDED
#define Z64_ZUI_H_INCLUDED

#include <stdint.h>

struct zuiSprite;

struct zuiSprite *
zuiSprite_new(unsigned char *rgba8888, int w, int h);

void
zui_rect(
	void *dest
	, int x, int y, int w, int h
	, uint32_t color
);

void
zuiSprite_draw(
	void *dest
	, struct zuiSprite *sprite
	, int cx, int cy, int cw, int ch
	, int dx, int dy
);

struct zuiSprite *
zuiSprite_debugFont(void);

void
zuiSprite_debugFont_draw(
	void *dest
	, struct zuiSprite *font
	, char *str
	, int x
	, int y
);

#endif /* Z64_ZUI_H_INCLUDED */

