
#include "n64ctx.h"

#define F3DZEXOPARGS struct n64ctx *ctx, unsigned char *b

enum n64microcode {
	N64MICRO_F3DZEX
	, N64MICRO_F3DEX
};


void
f3dzex_select_microcode(enum n64microcode microcode);


unsigned char *
f3dzex_ptr(F3DZEXOPARGS);


void
f3dzex_walk(F3DZEXOPARGS);


void
f3dzex_walk_zmode(F3DZEXOPARGS);


struct f3dtree *
f3dzex_get_tree(F3DZEXOPARGS, void *calloc(size_t nmemb, size_t size));

