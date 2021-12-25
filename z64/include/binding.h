#ifndef ZZN64BINDING_H_INCLUDED
#define ZZN64BINDING_H_INCLUDED

#include <stdlib.h>

#define N64BINDING_NEW_ARGS \
	void *calloc(size_t nmemb, size_t size)

enum n64binding_type {
	N64BINDING_SWRAST
	, N64BINDING_RAYLIB
};

/* XXX IMPORTANT this must match the `swr_input` structure in
 * swrast/main/include/swrast/input.h */
struct n64input
{
	float delta_time_sec;
	struct {
		struct {
			int x;
			int y;
		} pos;
		struct {
			int left;
			int middle;
			int right;
			int leftclick;
		} button;
		struct {
			int dx;
			int dy;
		} wheel;
	} mouse;
	struct {
		char w, a, s, d, lshift, lctrl, space;
	} key;
};

struct n64binding
{
	/* pointer to binding-specific data structure */
	void *udata;
	
	/* pointer to n64ctx structure */
	struct n64ctx *n64ctx;
	
	/* pointer to initialization routine */
	/* returns 0 on success, non-zero otherwise */
	int (*init)(
		struct n64binding *binding
		, char *window_name
		, int window_width
		, int window_height
	);
	
	/* pointer to routine for clearing screen before drawing */
	void (*clear)(
		struct n64binding *binding
		, unsigned char r
		, unsigned char g
		, unsigned char b
		, unsigned char a
	);
	
	/* pointer to routine for showing screen contents after drawing */
	void (*show)(
		struct n64binding *binding
	);
	
	/* tests events, returning 0 on exit request */
	int (*input)(
		struct n64binding *binding
	);
	
	/* sets modelview matrix */
	void (*set_modelview)(
		struct n64binding *binding
		, float m[16]
	);
	
	/* returns pointer to internal `depth_ofs` variable, for
	 * slightly offsetting decals */
	float *(*depth_ofs)(
		struct n64binding *binding
	);
	
	/* cleans everything up */
	void (*cleanup)(
		struct n64binding *binding
	);
	
	/* type of binding */
	enum n64binding_type type;
	struct n64input *input_struct;
	
	/* surface */
	void *ctx_surf;
};

/* binding creation functions */
struct n64binding *
n64binding_new_swrast(N64BINDING_NEW_ARGS);

struct n64binding *
n64binding_new_raylib(N64BINDING_NEW_ARGS);

#endif /* ZZN64BINDING_H_INCLUDED */

