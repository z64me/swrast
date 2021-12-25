#ifndef ZZN64CTX_H_INCLUDED
#define ZZN64CTX_H_INCLUDED

#define N64CTX_MATRIX_STACK_MAX 16

/* XXX n64 vertex buffer holds only 32 vertices at a time */
#define N64CTX_VBUF_MAX 32

#include <stdint.h>
#include <stddef.h>

struct n64binding;
struct f3dtree;
struct n64rast;
struct n64ctx;
struct vec4;

void
f3dtree_addChild(struct f3dtree *tree, struct f3dtree *child);

void
f3dtree_register(struct n64ctx *ctx, struct f3dtree *child);


struct f3dtree *
f3dtree_search_data(struct f3dtree *tree, unsigned char *data);


struct f3dtree *
f3dtree_new(void *calloc(size_t nmemb, size_t size));


struct n64rast *
n64rast_new(void *ctx);

/* swrast binding functions */
void
n64rast_use_n64ctx_swrast(struct n64ctx *ctx);
struct n64ctx *
n64ctx_new_swrast(void *ctx);

/* raylib binding functions */
void
n64rast_use_n64ctx_raylib(struct n64ctx *ctx);
struct n64ctx *
n64ctx_new_raylib(void *ctx);

void
n64rast_box(struct n64ctx *ctx, struct vec4 *vfirst, struct vec4 *vlast);


struct n64ctx *
n64ctx_new_obj(void *ctx);

#define N64_ZMODE_OPA   0
#define N64_ZMODE_INTER 1
#define N64_ZMODE_XLU   2
#define N64_ZMODE_DEC   3

#define N64_G_LIGHTING  0b00100000000000000000
#define N64_G_SHADE     0b0100


/* TODO do something smarter to detect
        whether to do custom vec4... */
#ifndef SWRAST_VECTOR_H_INCLUDED
struct vec4 {
	float x;
	float y;
	float z;
	float w;
} __attribute__ ((aligned (16)));
#endif

struct vec3 {
	float x;
	float y;
	float z;
} __attribute__ ((aligned (16)));

struct vec2 {
	float x;
	float y;
} __attribute__ ((aligned (8)));

struct n64vert {
	struct vec4 col;
	struct vec4 norm;
	struct vec4 pos;
	struct vec2 uv;
};

struct mat44f {
	struct vec4 x;
	struct vec4 y;
	struct vec4 z;
	struct vec4 w;
};

struct mat44s32 {
	int32_t v[4][4];
};

enum f3dtreeType {
	F3DTREE_UNSET = 0
	, F3DTREE_CULLDL
	, F3DTREE_CULLSPHERE
};

struct f3dtree {
	struct f3dtree   *parent;
	struct f3dtree   *child;
	struct f3dtree   *next;
	void             *data;
	char             *name;
	unsigned char     og_data[8];
	enum f3dtreeType  type;
	union {
		struct {
			struct vec4 vfirst;
			struct vec4 vlast;
		} culldl;
	} raw;
	uint32_t          dlofs;
	char              is_expanded;
};
	
/* raw texture data and other settings;
 * is set by setTIMG;
 * is then read as needed by texture loading opcodes */
struct n64timg {
	unsigned char *imgaddr;  /* pointer to raw texture data   */
	unsigned width:12;       /* width of texture              */
	unsigned siz:2;          /* bit size of pixels in texture */
	unsigned fmt:3;          /* pixel format of texture       */
};

struct n64tile
{
	/* offset of texture coordinates */
	int ulS;
	int ulT;
	
	/* these may as well be dimensions (width, height) */
	int lrS;
	int lrT;
	
	/* how many bits to shift texture coordinates       *
	 * if in range  1 <= n <= 10, texcoord >>= n        *
	 * if in range 11 <= n <= 15, texcoord <<= (16 - n) */
	unsigned shiftS:4;
	unsigned shiftT:4;
	
	/* TODO documentation */
	unsigned maskS:4;
	unsigned maskT:4;
	
	/* TODO documentation */
	unsigned palette:4;
	
	/* wrap behavior for each axis; can be combination of:
	 * G_TX_NOMIRROR = 0
	 * G_TX_MIRROR   = 1
	 * G_TX_WRAP     = 0
	 * G_TX_CLAMP    = 2    */
	unsigned cmS:2;
	unsigned cmT:2;
	
	/* bit size of each pixel; possible values:
	 * G_IM_SIZ_4b   = 0
	 * G_IM_SIZ_8b   = 1
	 * G_IM_SIZ_16b  = 2
	 * G_IM_SIZ_32b  = 3    */
	unsigned siz:2;
	
	/* color format of texture; possible values:
	 * G_IM_FMT_RGBA = 0
	 * G_IM_FMT_YUV  = 1
	 * G_IM_FMT_CI   = 2
	 * G_IM_FMT_IA   = 3
	 * G_IM_FMT_I    = 4    */
	unsigned fmt:3;
	
	/* number of 64-bit (8 byte) values per row of the texture;   *
	 * this value is 4 for the 16x16 blade texture                *
	 * 4 * 8 = 32; 32 / (2 bytes per pixel) = 16 px wide          *
	 * so this value is potentially useful for calculating width, *
	 * and actually seems to equate to what we have named `pitch` */
	/* TODO what would this mean for an 8x8 CI-4 texture? */
	unsigned line:10;
	
	/* TODO documentation */
	unsigned tmem:9;
	
	/* stuff that I added */
	
	/* pixel data */
	unsigned char *pix;
	
	/* dimensions */
	int width;
	int height;
	
	/* clamp dimensions */
	int clamp_w;
	int clamp_h;
	
	/* length of a row of pixels in bytes */
	long int pitch;
	
	/* floating point representations of shiftS and shiftT; multiply */
	float shiftS_f;
	float shiftT_f;
	
	/* number of f2 commands encountered; *
	 * the second f2 sets only clamp w/h; *
	 * reset on each F5 command           */
	/* TODO this is inaccurate; tile descriptors are the proper way */
	int f2;
};

struct setcombineBitfield
{
	unsigned
		  op  : 8
		, a0  : 4
		, c0  : 5
		, Aa0 : 3
		, Ac0 : 3
		, a1  : 4
		, c1  : 5
		, b0  : 4
		, b1  : 4
		, Aa1 : 3
		, Ac1 : 3
		, d0  : 3
		, Ab0 : 3
		, Ad0 : 3
		, d1  : 3
		, Ab1 : 3
		, Ad1 : 3
	;
};

/* a n64-inspired rendering context for materials and meshes */
struct n64ctx
{
	/* pointer to primary rasterizer itself; the
	 * internal structure varies with target */
	struct n64rast *rast;
	
	/* additional pointer to primary rasterizer's
	 * internal context, for performance */
	void *rctx;
	
	/* when `f3dzex_prewalk` and similar functions are
	 * used, this is propagated with a tree structure
	 * that describes the findings */
	struct f3dtree *f3dtree;
	
	/* pointer to function used for drawing triangle */
	void (*func_triangle)(
		struct n64ctx *ctx
		, int v0
		, int v1
		, int v2
	);
	
	/* pointer to function to call for setcombine */
	void (*func_setcombine)(
		struct n64ctx *ctx
		, unsigned char *b
	);
	
	/* pointer to function to call for loadblock */
	void (*func_loadblock)(
		struct n64ctx *ctx
		, unsigned char *b
	);
	
	/* pointer to function to call for geometrymode */
	void (*func_geometrymode)(
		struct n64ctx *ctx
	);
	
	/* pointer to function to call for setothermode */
	void (*func_othermode)(
		struct n64ctx *ctx
	);
	
	void (*func_update_tile_sampler)(
		struct n64ctx *ctx
		, struct n64tile *tile
	);
	
	void (*func_vertex)(
		struct n64ctx *ctx
		, struct n64vert *vert
	);
	
	void (*func_vbuf)(
		struct n64ctx *ctx
		, int numv
		, int vbidx
	);
	
	/* used for walking a display list */
	void (*func_walk)(
		struct n64ctx *ctx, unsigned char *b
	);
	
	/* level used in walking: internal use only */
	int walk_nesting_level;
	
	/* force a branch to another display list; internal use only */
	unsigned char *force_branch;
	
	/* force highlighting when a `walk` function is called *
	 * with the following address */
	void *highlight_me;
	
#if 0
	/* memory for storing palette and texture before conversion */
	unsigned char *palette;
	unsigned char *texture;
#endif
	
	/* pointer to last palette specified */
	unsigned char *palette;
	
	/* pointer to blank texture (white, opaque) */
	unsigned char *blank_texture;
	
	/* pointer to raw data currently being parsed */
	unsigned char *b;
	
	/* raw texture data and other settings */
	struct n64timg timg;
	
	/* memory for storing textures after converting to rgba32, *
	 * and other texel-related settings */
	struct n64tile tile[8];
	
	/* list of ram segments */
	unsigned char *segment[16];
	
	/* for direct ram addresses like 80 and 81, *
	 * this is the base ram address to sbutract */
	long unsigned int segment80;
	
	/* vertex buffer */
	struct n64vert vbuf[N64CTX_VBUF_MAX];
	
	union
	{
		struct setcombineBitfield bitfield;
		
		/* blending forced by combiner, when texels/textures
		 * are sampled for alpha values */
		char force_blending;
		/*struct
		{
			uint32_t hi;
			uint32_t lo;
		} word;*/
	} combiner;
	
	struct vec4
		fogcolor
		, blendcolor
		, primcolor
		, envcolor
		, vertcolor
		, texel0
		, texel1
		, combined
	;
	
	struct vec3
		scale
		, center
		, noise
	;
	
	struct {
		unsigned long int hi;
		unsigned long int lo;
	} rdp;
	
	struct {
		unsigned long int hi;
		unsigned long int lo;
		unsigned alphacompare:2;
		struct {
			unsigned aa_en:1;         /* TODO */
			unsigned z_cmp:1;         /* TODO */
			unsigned z_upd:1;         /* TODO */
			unsigned im_rd:1;         /* TODO */
			unsigned clr_on_cvg:1;    /* TODO */
			unsigned cvg_dst:2;       /* TODO */
			unsigned zmode:2;         /* the layer being rendered on */
			unsigned cvg_x_alpha:1;   /* texture alpha blending */
			unsigned alpha_cvg_sel:1; /* TODO */
			unsigned force_bl:1;      /* force alpha blending */
			unsigned int v;
		} indep;
	} othermode;
	
	struct {
		unsigned current:32;
		unsigned texgen:2;
		unsigned cull_front:1;
		unsigned cull_back:1;
	} geometrymode;
	
	struct {
		int             stack_level;
		struct mat44f   stack[N64CTX_MATRIX_STACK_MAX];
		struct mat44f  *mat;
	} matrix;
	
	float minlevel;
	/* `minlevel` defines the minimum possible value for LOD
	    to have, when the LOD calculated for a particular part
	    of the primitive is less than 1.0. In other words, the
	    LOD of any part of the primitive is clamped at the lower
	    end to max(minlevel, LOD). */
	
	float lodfrac;
	/* `lodfrac` specifies a fraction that the programmer can
	   specify for use in the color combiner of the RDP. It's
	   meant to offer a further refinement (XXX verify that,
	   not 100% sure on it) on the linear filtering of two mipmaps.
	   (Typically referred to in the SDK and elsewhere as "trilinear"
	   filtering; however, due to the potential ambiguity of this term,
	   we won't use it here. See the note on terminology at 
	   https://www.opengl.org/wiki/Sampler_Object#Filtering ) */
	
	float primLodfrac;
	float k4;
	float k5;
	
	char use_shading;
	char use_vertex_colors;
	char use_vertex_normals;
	char use_point_filtering;
	
	/* render only requested zmode */
	char only_zmode;
	
	/* forces alpha blending on certain layers */
	char force_blending_override;
	
	/* forces highlighting */
	char highlight_on;
	
	/* treat branches as DE commands */
	char branches_are_DLs;
};

#endif /* ZZN64CTX_H_INCLUDED */

