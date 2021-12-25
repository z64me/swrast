#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <swrast/inputassembler.h>
#include <swrast/context.h>
#include <swrast/rasterizer.h>
#include <swrast/shader.h>

#include <n64ctx.h>
#include <f3dzex.h>

/* jump to address of specific pixel, given coordinates */
#define PIXELADDR(T, X, Y) T->imgaddr + (Y * T->pitch + X)

#define FRAG_NAME "outcolor"

#define Z64OPARGS struct n64ctx *mat,struct vec4 *v

#ifdef Z64MATGEN_BUILDING_LIST
void (*z64FC_c_oplist_add[48])(Z64OPARGS) = {0};
void (*z64FC_c_oplist_sub[48])(Z64OPARGS) = {0};
void (*z64FC_c_oplist_mul[48])(Z64OPARGS) = {0};
void (*z64FC_c_oplist_set[48])(Z64OPARGS) = {0};

void (*z64FC_a_oplist_add[48])(Z64OPARGS) = {0};
void (*z64FC_a_oplist_sub[48])(Z64OPARGS) = {0};
void (*z64FC_a_oplist_mul[48])(Z64OPARGS) = {0};
void (*z64FC_a_oplist_set[48])(Z64OPARGS) = {0};
#else
#include "fc.h"
#endif

static struct n64ctx *g_n64ctx = 0;

/* filtering options */
#define N64_FILTER_BILINEAR 0 /* default */
#define N64_FILTER_NONE     1

/* Z64GEN_CLAMPER_FABS is from https://stackoverflow.com/questions/427477/fastest-way-to-clamp-a-real-fixed-floating-point-value#427580 (search `minicomputer`) */
#define Z64GEN_CLAMPER_FABS          0 /* one function call */
#define Z64GEN_CLAMPER_FMIN_FMAX     1 /* two function calls */
#define Z64GEN_CLAMPER_DEPENDS       2 /* uses different functions based on operation */
#define Z64GEN_CLAMPER_NONE          -1
#define Z64GEN_CLAMPER               Z64GEN_CLAMPER_NONE

/* #define this when building to use LUTs for int2float conversions */
#define N64RAST_USE_LUTS

#ifndef N64RAST_USE_LUTS

/* use multiplication for these */
#define DIV_1_7(x)   (0.142857143f * (x))
#define DIV_1_15(x)  (0.066666667f * (x))
#define DIV_1_31(x)  (0.032258065f * (x))
#define DIV_1_255(x) (0.003921569f * (x))

#else
#define DIV_1_7(x)   lut_1_7_float[x]
#define DIV_1_15(x)  lut_1_15_float[x]
#define DIV_1_31(x)  lut_1_31_float[x]
#define DIV_1_255(x) lut_1_255_float[x]

static float lut_1_7_float[] =
{
	0.000000, 0.142857, 0.285714, 0.428571
	, 0.571429, 0.714286, 0.857143, 1.000000
};

static float lut_1_15_float[] =
{
	0.000000, 0.066667, 0.133333, 0.200000
	, 0.266667, 0.333333, 0.400000, 0.466667
	, 0.533333, 0.600000, 0.666667, 0.733333
	, 0.800000, 0.866667, 0.933333, 1.000000
};

static float lut_1_31_float[] =
{
	0.000000, 0.032258, 0.064516, 0.096774
	, 0.129032, 0.161290, 0.193548, 0.225806
	, 0.258065, 0.290323, 0.322581, 0.354839
	, 0.387097, 0.419355, 0.451613, 0.483871
	, 0.516129, 0.548387, 0.580645, 0.612903
	, 0.645161, 0.677419, 0.709677, 0.741935
	, 0.774194, 0.806452, 0.838710, 0.870968
	, 0.903226, 0.935484, 0.967742, 1.000000
};

static float lut_1_255_float[256] = {
	0.000000f, 0.000304f, 0.000607f, 0.000911f, 0.001214f, 0.001518f, 0.001821f, 0.002125f, 0.002428f, 0.002732f, 0.003035f,
	0.003347f, 0.003677f, 0.004025f, 0.004391f, 0.004777f, 0.005182f, 0.005605f, 0.006049f, 0.006512f, 0.006995f, 0.007499f,
	0.008023f, 0.008568f, 0.009134f, 0.009721f, 0.010330f, 0.010960f, 0.011612f, 0.012286f, 0.012983f, 0.013702f, 0.014444f,
	0.015209f, 0.015996f, 0.016807f, 0.017642f, 0.018500f, 0.019382f, 0.020289f, 0.021219f, 0.022174f, 0.023153f, 0.024158f,
	0.025187f, 0.026241f, 0.027321f, 0.028426f, 0.029557f, 0.030713f, 0.031896f, 0.033105f, 0.034340f, 0.035601f, 0.036889f,
	0.038204f, 0.039546f, 0.040915f, 0.042311f, 0.043735f, 0.045186f, 0.046665f, 0.048172f, 0.049707f, 0.051269f, 0.052861f,
	0.054480f, 0.056128f, 0.057805f, 0.059511f, 0.061246f, 0.063010f, 0.064803f, 0.066626f, 0.068478f, 0.070360f, 0.072272f,
	0.074214f, 0.076185f, 0.078187f, 0.080220f, 0.082283f, 0.084376f, 0.086500f, 0.088656f, 0.090842f, 0.093059f, 0.095307f,
	0.097587f, 0.099899f, 0.102242f, 0.104616f, 0.107023f, 0.109462f, 0.111932f, 0.114435f, 0.116971f, 0.119538f, 0.122139f,
	0.124772f, 0.127438f, 0.130136f, 0.132868f, 0.135633f, 0.138432f, 0.141263f, 0.144128f, 0.147027f, 0.149960f, 0.152926f,
	0.155926f, 0.158961f, 0.162029f, 0.165132f, 0.168269f, 0.171441f, 0.174647f, 0.177888f, 0.181164f, 0.184475f, 0.187821f,
	0.191202f, 0.194618f, 0.198069f, 0.201556f, 0.205079f, 0.208637f, 0.212231f, 0.215861f, 0.219526f, 0.223228f, 0.226966f,
	0.230740f, 0.234551f, 0.238398f, 0.242281f, 0.246201f, 0.250158f, 0.254152f, 0.258183f, 0.262251f, 0.266356f, 0.270498f,
	0.274677f, 0.278894f, 0.283149f, 0.287441f, 0.291771f, 0.296138f, 0.300544f, 0.304987f, 0.309469f, 0.313989f, 0.318547f,
	0.323143f, 0.327778f, 0.332452f, 0.337164f, 0.341914f, 0.346704f, 0.351533f, 0.356400f, 0.361307f, 0.366253f, 0.371238f,
	0.376262f, 0.381326f, 0.386430f, 0.391573f, 0.396755f, 0.401978f, 0.407240f, 0.412543f, 0.417885f, 0.423268f, 0.428691f,
	0.434154f, 0.439657f, 0.445201f, 0.450786f, 0.456411f, 0.462077f, 0.467784f, 0.473532f, 0.479320f, 0.485150f, 0.491021f,
	0.496933f, 0.502887f, 0.508881f, 0.514918f, 0.520996f, 0.527115f, 0.533276f, 0.539480f, 0.545725f, 0.552011f, 0.558340f,
	0.564712f, 0.571125f, 0.577581f, 0.584078f, 0.590619f, 0.597202f, 0.603827f, 0.610496f, 0.617207f, 0.623960f, 0.630757f,
	0.637597f, 0.644480f, 0.651406f, 0.658375f, 0.665387f, 0.672443f, 0.679543f, 0.686685f, 0.693872f, 0.701102f, 0.708376f,
	0.715694f, 0.723055f, 0.730461f, 0.737911f, 0.745404f, 0.752942f, 0.760525f, 0.768151f, 0.775822f, 0.783538f, 0.791298f,
	0.799103f, 0.806952f, 0.814847f, 0.822786f, 0.830770f, 0.838799f, 0.846873f, 0.854993f, 0.863157f, 0.871367f, 0.879622f,
	0.887923f, 0.896269f, 0.904661f, 0.913099f, 0.921582f, 0.930111f, 0.938686f, 0.947307f, 0.955974f, 0.964686f, 0.973445f,
	0.982251f, 0.991102f, 1.0f
};

#endif /* N64RAST_USE_LUTS */

/* if you just want it to compile to create a list,
   #define Z64MATGEN_BUILDING_LIST 1 before compiling */
//#define Z64MATGEN_BUILDING_LIST

#if 0
#ifdef Z64MATGEN_BUILDING_LIST
struct swr_shaderProgram { int ok; };
struct swr_context { void *n64ctx; int *textures[8]; };
struct rs_vertex { int *attribs[8]; };

#define  ATTRIB_TEX0 0
#define  ATTRIB_TEX1 1
#endif
#endif
	
struct n64rast
{
	/* main rendering context (ex. a `struct swr_context`) */
	void *ctx;
	
	/* operation list used for calculating fragments */
	void (*oplist[16])(Z64OPARGS);
	void (**oplist_ptr)(Z64OPARGS);  /* this is the list you walk; *
	                                  * exists to make omitting    *
	                                  * entries slightly faster    */
	
	/* pointer to function used for fragment operations */
	struct vec4 (*func_frag)(
		struct swr_shaderProgram *prog
		, struct swr_context *ctx
		, struct rs_vertex *frag
	);
	
	/* pointer to tile samplers: one per tile */
	void (*func_tile_sample[8])(
		struct n64tile *tile
		, struct vec4 *uv
		, struct vec4 *color
	);
	
	int oplist_count;
};


struct n64rast *
n64rast_new(void *ctx)
{
	struct n64rast *r;
	
	r = calloc(1, sizeof(*r));
	
	if (!r)
		exit(EXIT_FAILURE);
	
	r->ctx = ctx;
	
	return r;
}

static
inline
void
texture_sample_inplace(
	struct vec4 *v
	, struct texture *a
	, struct vec4 *b
)
{
}

#define N64_COLOR_FUNC(FMT) \
static \
inline \
void \
n64_color_v4_##FMT( \
	struct vec4 *color \
	, unsigned char *b \
)

#define N64_COLOR_FUNC_4BPP(FMT) \
static \
inline \
void \
n64_color_v4_int_##FMT( \
	struct vec4 *color \
	, int *b \
)

N64_COLOR_FUNC(i4){}
N64_COLOR_FUNC_4BPP(i4)
{
	color->x = DIV_1_15(*b);
	color->y = color->x;
	color->z = color->x;
	color->w = color->x * 0.5; /* TODO: * 0.5 fixes kokiri path */
}

N64_COLOR_FUNC(ia4){}
N64_COLOR_FUNC_4BPP(ia4)
{
	color->x = DIV_1_7(*b >> 1);
	color->y = color->x;
	color->z = color->x;
	color->w = (*b) & 1;
}

N64_COLOR_FUNC_4BPP(i8){};
N64_COLOR_FUNC(i8)
{
	color->x = DIV_1_255(*b);
	color->y = color->x;
	color->z = color->x;
	color->w = color->x;
}

N64_COLOR_FUNC_4BPP(ia8){};
N64_COLOR_FUNC(ia8)
{
	color->x = DIV_1_15(*b >> 4);
	color->y = color->x;
	color->z = color->x;
	color->w = DIV_1_15(*b & 0xF);
}

N64_COLOR_FUNC_4BPP(ia16){};
N64_COLOR_FUNC(ia16)
{
	color->x = DIV_1_255(*b);
	color->y = color->x;
	color->z = color->x;
	color->w = DIV_1_255(b[1]);
}

N64_COLOR_FUNC_4BPP(rgba5551){};
N64_COLOR_FUNC(rgba5551)
{
	int comb = (b[0]<<8)|b[1];
	color->x = DIV_1_31((comb >> 11) & 0x1F);
	color->y = DIV_1_31((comb >>  6) & 0x1F);
	color->z = DIV_1_31((comb >>  1) & 0x1F);
	color->w = comb & 1;
}

N64_COLOR_FUNC_4BPP(ci8){};
N64_COLOR_FUNC(ci8)
{
	/* note that this is just rgba5551 */
	int comb = (b[0]<<8)|b[1];
	color->x = DIV_1_31((comb >> 11) & 0x1F);
	color->y = DIV_1_31((comb >>  6) & 0x1F);
	color->z = DIV_1_31((comb >>  1) & 0x1F);
	color->w = comb & 1;
}

N64_COLOR_FUNC_4BPP(ci4){};
N64_COLOR_FUNC(ci4)
{
	/* note that this is just rgba5551 */
	int comb = (b[0]<<8)|b[1];
	color->x = DIV_1_31((comb >> 11) & 0x1F);
	color->y = DIV_1_31((comb >>  6) & 0x1F);
	color->z = DIV_1_31((comb >>  1) & 0x1F);
	color->w = comb & 1;
}

N64_COLOR_FUNC_4BPP(rgba8888){};
N64_COLOR_FUNC(rgba8888)
{
	color->x = DIV_1_255(b[0]);
	color->y = DIV_1_255(b[1]);
	color->z = DIV_1_255(b[2]);
	color->w = DIV_1_255(b[3]);
}


static
inline
void
get_uv_wrap(
	int *v
	, int const *range
	, int const *range_clamp
)
{
	/* if out of bounds, clamp to within bounds; *
	 * note that this v&=w-1 hack will not work  *
	 * on NPOTs; you have to use v%=w for those  */
	*v &= *range - 1;
}


static
inline
void
get_uv_clamp(
	int *v
	, int const *range
	, int const *range_clamp
)
{
	/* if out of bounds, shift to within bounds; *
	 * note that this v&=w-1 hack will not work  *
	 * on NPOTs; you have to use v%=w for those  */
	if (*v < 0)
		*v = 0;
	else if (*v >= *range_clamp)
		*v = *range - 1;
	else
		*v &= *range - 1;
}


static
inline
void
get_uv_mirror(
	int *v
	, int const *range
	, int const *range_clamp
)
{
	/* handle negative values */
	if (*v < 0)
	{
		if (*v & *range)
			*v = *range - (*v & (*range - 1) );
		else
			*v = (*range * 2) - (*v & (*range - 1) );
	}
	
	/* if odd repetition, use flipped version */
	if (*v & *range)
		*v = *range - ( (*v & (*range - 1) ) + 1);
	if (*v >= *range)
		*v &= *range - 1;
}


static
inline
void
get_uv_clmir(
	int *v
	, int const *range
	, int const *range_clamp
)
{
	/* negatives are clamped */
	if (*v < 0)
		*v = 0;
		
	/* anything exceeding mirror range is clamped */
	/* `*range * 2` replaced with `*range_clamp` */
	else if (*v >= *range_clamp)
	{
		*v = (*range_clamp) - 1;
		
		/* copied from get_uv_mirror for speed: */
		/* if odd repetition, use flipped version */
		if (*v & *range)
			*v = *range - ( (*v & (*range - 1) ) + 1);
		if (*v >= *range)
			*v &= *range - 1;
	}
	
	/* all other cases are mirrored */
	else
		get_uv_mirror(v, range, range_clamp);
}


static
inline
float
lerp(float *s, float *e, float *t)
{
	return *s + (*e - *s) * *t;
}


static
inline
float
lerpX(float s, float e, float *t)
{
	return s + (e - s) * *t;
}


static
inline
float
blerp(
	float *c00
	, float *c10
	, float *c01
	, float *c11
	, float *tx
	, float *ty
)
{
	return
		lerpX(
			lerp(c00, c10, tx)
			, lerp(c01, c11, tx)
			, ty
		)
	;
}


#define N64_SAMPLER_NAME(FMT, FILTER, BPP, WRAPs, WRAPt) \
n64_sampler_##FMT##WRAPs##WRAPt##BPP##FILTER

#define IS_4BIT(BPP) BPP == 0

#define N64_SAMPLE_UV(COLOR, XEQ, YEQ, WRAPs, WRAPt, FMT, BPP, IS_CI) \
	x = XEQ;                                                      \
	y = YEQ;                                                      \
	get_uv_##WRAPs(&x, &tile->width, &tile->clamp_w);                             \
	get_uv_##WRAPt(&y, &tile->height, &tile->clamp_h);                            \
	/* 4 bit image */ \
	if (IS_4BIT(BPP)) \
	{ \
		ptr = tile->pix + (y * tile->pitch + x / 2);               \
		if (x & 1) \
			x = (*ptr) & 0xF; \
		else \
			x = (*ptr) >> 4; \
	} \
	/* other */ \
	else \
		ptr = tile->pix + (y * tile->pitch + x * BPP); \
	if (IS_CI)                                                    \
	{ \
		if (IS_4BIT(BPP)) \
			ptr = g_n64ctx->palette + 2 * x;  \
		else \
			ptr = g_n64ctx->palette + 2 * *ptr;                     \
		n64_color_v4_##FMT(COLOR, ptr); \
	} \
	else { \
		if (IS_4BIT(BPP)) \
			n64_color_v4_int_##FMT(COLOR, &x); \
		else \
			n64_color_v4_##FMT(COLOR, ptr); \
	}

#define N64_PIXEL_LERP(MEMBER) \
		color->MEMBER =          \
			blerp(                \
				&c00.MEMBER        \
				, &c10.MEMBER      \
				, &c01.MEMBER      \
				, &c11.MEMBER      \
				, &gx              \
				, &gy              \
			)
			
#define N64_SAMPLE_BILINEAR(FMT, BPP, WRAPs, WRAPt, IS_CI) \
/* bilinear interpolation start */ \
	/* get base */ \
	float gx = uv->x - 0.5f; \
	float gy = uv->y - 0.5f;\
	/* we want it as an int */ \
	/* sample the four colors */ \
	struct vec4 c00; \
	struct vec4 c10; \
	struct vec4 c01; \
	struct vec4 c11; \
	/* sample four pixels; gx is necessary; gxi causes issues */ \
	N64_SAMPLE_UV(&c00, gx  , gy  , WRAPs, WRAPt, FMT, BPP, IS_CI) \
	N64_SAMPLE_UV(&c10, gx+1, gy  , WRAPs, WRAPt, FMT, BPP, IS_CI) \
	N64_SAMPLE_UV(&c11, gx+1, gy+1, WRAPs, WRAPt, FMT, BPP, IS_CI) \
	N64_SAMPLE_UV(&c01, gx  , gy+1, WRAPs, WRAPt, FMT, BPP, IS_CI) \
	/* gx gy */ \
	gx -= (int)gx; \
	gy -= (int)gy; \
	/* necessary bounds-checking to fix "fireflies" */ \
	if (gx > 1) gx -= 1; \
	if (gx < 0) gx += 1; \
	if (gy > 1) gy -= 1; \
	if (gy < 0) gy += 1; \
	N64_PIXEL_LERP(x); \
	N64_PIXEL_LERP(y); \
	N64_PIXEL_LERP(z); \
	N64_PIXEL_LERP(w); \
	/* bilinear interpolation end */

#define NEW_N64_SAMPLER_WRAPst(FMT, IS_CI, FILTER, BPP, WRAPs, WRAPt) \
static \
inline \
void \
N64_SAMPLER_NAME(FMT, FILTER, BPP, WRAPs, WRAPt)( \
	struct n64tile *tile \
	, struct vec4 *uv \
	, struct vec4 *color \
) \
{ \
	unsigned char *ptr; \
	int x, y; \
	if (FILTER == N64_FILTER_NONE) \
	{ \
		N64_SAMPLE_UV(           \
			color    \
			, uv->x    \
			, uv->y    \
			, WRAPs    \
			, WRAPt    \
			, FMT    \
			, BPP    \
			, IS_CI    \
		) \
	} \
	if (FILTER == N64_FILTER_BILINEAR) \
	{ \
		\
		N64_SAMPLE_BILINEAR( \
			FMT \
			, BPP \
			, WRAPs \
			, WRAPt \
			, IS_CI \
		) \
		\
	} \
}

/* creates samplers with all wrap/mirror/clamp combinations */
#define NEW_N64_SAMPLER(FMT, IS_CI, FILTER, BPP) \
NEW_N64_SAMPLER_WRAPst(FMT, IS_CI, FILTER, BPP, wrap, wrap)     \
NEW_N64_SAMPLER_WRAPst(FMT, IS_CI, FILTER, BPP, wrap, mirror)   \
NEW_N64_SAMPLER_WRAPst(FMT, IS_CI, FILTER, BPP, wrap, clamp)    \
NEW_N64_SAMPLER_WRAPst(FMT, IS_CI, FILTER, BPP, wrap, clmir)    \
                                                             \
NEW_N64_SAMPLER_WRAPst(FMT, IS_CI, FILTER, BPP, mirror, wrap)   \
NEW_N64_SAMPLER_WRAPst(FMT, IS_CI, FILTER, BPP, mirror, mirror) \
NEW_N64_SAMPLER_WRAPst(FMT, IS_CI, FILTER, BPP, mirror, clamp)  \
NEW_N64_SAMPLER_WRAPst(FMT, IS_CI, FILTER, BPP, mirror, clmir)  \
                                                             \
NEW_N64_SAMPLER_WRAPst(FMT, IS_CI, FILTER, BPP, clamp, wrap)    \
NEW_N64_SAMPLER_WRAPst(FMT, IS_CI, FILTER, BPP, clamp, mirror)  \
NEW_N64_SAMPLER_WRAPst(FMT, IS_CI, FILTER, BPP, clamp, clamp)   \
NEW_N64_SAMPLER_WRAPst(FMT, IS_CI, FILTER, BPP, clamp, clmir)   \
                                                             \
NEW_N64_SAMPLER_WRAPst(FMT, IS_CI, FILTER, BPP, clmir, wrap)    \
NEW_N64_SAMPLER_WRAPst(FMT, IS_CI, FILTER, BPP, clmir, mirror)  \
NEW_N64_SAMPLER_WRAPst(FMT, IS_CI, FILTER, BPP, clmir, clamp)   \
NEW_N64_SAMPLER_WRAPst(FMT, IS_CI, FILTER, BPP, clmir, clmir)   

/* creates sampler list with all wrap/mirror/clamp combinations, *
 * for use in arrays */
#define N64_SAMPLER_LIST(FMT, FILTER, BPP) \
N64_SAMPLER_NAME(FMT, FILTER, BPP, wrap, wrap),     \
N64_SAMPLER_NAME(FMT, FILTER, BPP, wrap, mirror),   \
N64_SAMPLER_NAME(FMT, FILTER, BPP, wrap, clamp),    \
N64_SAMPLER_NAME(FMT, FILTER, BPP, wrap, clmir),    \
                                                             \
N64_SAMPLER_NAME(FMT, FILTER, BPP, mirror, wrap),   \
N64_SAMPLER_NAME(FMT, FILTER, BPP, mirror, mirror), \
N64_SAMPLER_NAME(FMT, FILTER, BPP, mirror, clamp),  \
N64_SAMPLER_NAME(FMT, FILTER, BPP, mirror, clmir),  \
                                                             \
N64_SAMPLER_NAME(FMT, FILTER, BPP, clamp, wrap),    \
N64_SAMPLER_NAME(FMT, FILTER, BPP, clamp, mirror),  \
N64_SAMPLER_NAME(FMT, FILTER, BPP, clamp, clamp),   \
N64_SAMPLER_NAME(FMT, FILTER, BPP, clamp, clmir),   \
                                                             \
N64_SAMPLER_NAME(FMT, FILTER, BPP, clmir, wrap),    \
N64_SAMPLER_NAME(FMT, FILTER, BPP, clmir, mirror),  \
N64_SAMPLER_NAME(FMT, FILTER, BPP, clmir, clamp),   \
N64_SAMPLER_NAME(FMT, FILTER, BPP, clmir, clmir),   

/* for formats that don't exist */
#define N64_SAMPLER_LIST_0 \
0, 0, 0, 0, \
0, 0, 0, 0, \
0, 0, 0, 0, \
0, 0, 0, 0, 

/* generic samplers */
NEW_N64_SAMPLER(rgba8888, 0, N64_FILTER_BILINEAR, 4)
NEW_N64_SAMPLER(rgba5551, 0, N64_FILTER_BILINEAR, 2)
NEW_N64_SAMPLER(ia16,     0, N64_FILTER_BILINEAR, 2)
NEW_N64_SAMPLER(ia8,      0, N64_FILTER_BILINEAR, 1)
NEW_N64_SAMPLER(i8,       0, N64_FILTER_BILINEAR, 1)

/* 4-bit samplers */
NEW_N64_SAMPLER(ia4,      0, N64_FILTER_BILINEAR, 0)
NEW_N64_SAMPLER(i4,       0, N64_FILTER_BILINEAR, 0)

/* color indexed samplers */
NEW_N64_SAMPLER(ci8,      1, N64_FILTER_BILINEAR, 1)
NEW_N64_SAMPLER(ci4,      1, N64_FILTER_BILINEAR, 0)

/* samplers without filtering */
NEW_N64_SAMPLER(rgba8888, 0, N64_FILTER_NONE, 4)
NEW_N64_SAMPLER(rgba5551, 0, N64_FILTER_NONE, 2)
NEW_N64_SAMPLER(ia16,     0, N64_FILTER_NONE, 2)
NEW_N64_SAMPLER(ia8,      0, N64_FILTER_NONE, 1)
NEW_N64_SAMPLER(i8,       0, N64_FILTER_NONE, 1)
NEW_N64_SAMPLER(ia4,      0, N64_FILTER_NONE, 0)
NEW_N64_SAMPLER(i4,       0, N64_FILTER_NONE, 0)
NEW_N64_SAMPLER(ci8,      1, N64_FILTER_NONE, 1)
NEW_N64_SAMPLER(ci4,      1, N64_FILTER_NONE, 0)

/* given an array index, return format and such */
static
char *
n64_sampler_array_name(int idx)
{
	static char *name = 0;
	if (!name)
	{
		name = malloc(1024);
		
		if (!name)
			exit(EXIT_FAILURE);
	}
	
	
	const char *s_fmt[] = { "rgba", "yuv", "ci", "ia", "i", "rgba", "yuv", "ci", "ia", "i" };
	const char *s_bpp[] = { "4", "8", "16", "32" };
	
	int fmt = idx / 64;
	int bpp = (idx & (64 - 1)) / 16;
	
	strcpy(name, s_fmt[fmt]);
	strcat(name, s_bpp[bpp]);
	
	return name;
}

/* sampler array */
static
void
(*n64_sampler_array[])(
	struct n64tile *tile
	, struct vec4 *uv
	, struct vec4 *color
) = {
	/* rgba = 0 */
	N64_SAMPLER_LIST_0            /*  4b */
	N64_SAMPLER_LIST_0            /*  8b */
	N64_SAMPLER_LIST(rgba5551, N64_FILTER_BILINEAR, 2) /* 16b */
	N64_SAMPLER_LIST(rgba8888, N64_FILTER_BILINEAR, 4) /* 32b */
	/* yuv = 1 */
	N64_SAMPLER_LIST_0            /*  4b */
	N64_SAMPLER_LIST_0            /*  8b */
	N64_SAMPLER_LIST_0            /* 16b */
	N64_SAMPLER_LIST_0            /* 32b */
	/* ci = 2 */
	N64_SAMPLER_LIST(ci4, N64_FILTER_BILINEAR, 0)      /*  4b */
	N64_SAMPLER_LIST(ci8, N64_FILTER_BILINEAR, 1)      /*  8b */
	N64_SAMPLER_LIST_0            /* 16b */
	N64_SAMPLER_LIST_0            /* 32b */
	/* ia = 3 */
	N64_SAMPLER_LIST(ia4, N64_FILTER_BILINEAR, 0)      /*  4b */
	N64_SAMPLER_LIST(ia8, N64_FILTER_BILINEAR, 1)      /*  8b */
	N64_SAMPLER_LIST(ia16, N64_FILTER_BILINEAR, 2)     /* 16b */
	N64_SAMPLER_LIST_0            /* 32b */
	/* i = 4 */
	N64_SAMPLER_LIST(i4, N64_FILTER_BILINEAR, 0)       /*  4b */
	N64_SAMPLER_LIST(i8, N64_FILTER_BILINEAR, 1)       /*  8b */
	N64_SAMPLER_LIST_0            /* 16b */
	N64_SAMPLER_LIST_0            /* 32b */
	
	/* now repeat, but without filtering */
	/* rgba = 0 */
	N64_SAMPLER_LIST_0            /*  4b */
	N64_SAMPLER_LIST_0            /*  8b */
	N64_SAMPLER_LIST(rgba5551, N64_FILTER_NONE, 2) /* 16b */
	N64_SAMPLER_LIST(rgba8888, N64_FILTER_NONE, 4) /* 32b */
	/* yuv = 1 */
	N64_SAMPLER_LIST_0            /*  4b */
	N64_SAMPLER_LIST_0            /*  8b */
	N64_SAMPLER_LIST_0            /* 16b */
	N64_SAMPLER_LIST_0            /* 32b */
	/* ci = 2 */
	N64_SAMPLER_LIST(ci4, N64_FILTER_NONE, 0)      /*  4b */
	N64_SAMPLER_LIST(ci8, N64_FILTER_NONE, 1)      /*  8b */
	N64_SAMPLER_LIST_0            /* 16b */
	N64_SAMPLER_LIST_0            /* 32b */
	/* ia = 3 */
	N64_SAMPLER_LIST(ia4, N64_FILTER_NONE, 0)      /*  4b */
	N64_SAMPLER_LIST(ia8, N64_FILTER_NONE, 1)      /*  8b */
	N64_SAMPLER_LIST(ia16, N64_FILTER_NONE, 2)     /* 16b */
	N64_SAMPLER_LIST_0            /* 32b */
	/* i = 4 */
	N64_SAMPLER_LIST(i4, N64_FILTER_NONE, 0)       /*  4b */
	N64_SAMPLER_LIST(i8, N64_FILTER_NONE, 1)       /*  8b */
	N64_SAMPLER_LIST_0            /* 16b */
	N64_SAMPLER_LIST_0            /* 32b */
};


#define G_CCMUX_COMBINED             0
#define G_CCMUX_TEXEL0               1
#define G_CCMUX_TEXEL1               2
#define G_CCMUX_PRIMITIVE            3
#define G_CCMUX_SHADE                4
#define G_CCMUX_ENVIRONMENT          5

#define G_CCMUX_1                    6
#define G_CCMUX_CENTER               6
#define G_CCMUX_SCALE                6
#define G_CCMUX_CENTER_ALIAS         32
#define G_CCMUX_SCALE_ALIAS          33

#define G_CCMUX_COMBINED_ALPHA       7
#define G_CCMUX_NOISE                7
#define G_CCMUX_K4                   7
#define G_CCMUX_NOISE_ALIAS          34
#define G_CCMUX_K4_ALIAS             35

#define G_CCMUX_TEXEL0_ALPHA         8
#define G_CCMUX_TEXEL1_ALPHA         9
#define G_CCMUX_PRIMITIVE_ALPHA      10
#define G_CCMUX_SHADE_ALPHA          11
#define G_CCMUX_ENV_ALPHA            12
#define G_CCMUX_LOD_FRACTION         13
#define G_CCMUX_PRIM_LOD_FRAC        14
#define G_CCMUX_K5                   15
#define G_CCMUX_0                    31
#define G_CCMUX_0_INITIALIZER        -1

/* alpha values */
#define G_ACMUX_COMBINED             0
#define G_ACMUX_LOD_FRACTION         0
#define G_ACMUX_LOD_FRACTION_ALIAS   8//36

#define G_ACMUX_TEXEL0               1
#define G_ACMUX_TEXEL1               2
#define G_ACMUX_PRIMITIVE            3
#define G_ACMUX_SHADE                4
#define G_ACMUX_ENVIRONMENT          5

#define G_ACMUX_1                    6
#define G_ACMUX_PRIM_LOD_FRAC        6
#define G_ACMUX_PRIM_LOD_FRAC_ALIAS  9//37

#define G_ACMUX_0                    7

/* options available for each setcombine slot */
struct setcombineOptions
{
	int num;
	struct
	{
		int value;
		const char *tech;
		const char *name;
	} list[48];
	char remap[32];
};

struct setcombineVariable
{
	char *name;
	int is_vec3;
	int is_0;
	int is_1;
	int only_a;
	int is_combined;
	int is_texel0;
	int is_texel1;
};

static
char *
sanitize_name(char *name)
{
	static char *str = 0;
	
	if (!str)
		str = malloc(1024);
	
	if (!str)
		exit(EXIT_FAILURE);
	
	strcpy(str, name);
	
	/* replace '.' with '_' */
	for (char *w = str; *w; ++w)
	{
		if (*w == '.')
			*w = '_';
	}
	
	return str;
}

static
char *
make_FC_operand(struct setcombineVariable *var)
{
	static char *buf = 0;
	
	if (!buf)
		buf = malloc(256);
	
	if (!buf)
		exit(EXIT_FAILURE);
	
	if (var->is_0 || var->is_1)
		return var->name;
	
	sprintf(buf, "mat->%s", var->name);
	
	return buf;
}

static
char *
step_func_name(
	struct setcombineVariable *var
	, int is_color
	, char *op_name
)
{
	static char *str = 0;
	
	if (!str)
		str = malloc(1024);
	
	if (!str)
		exit(EXIT_FAILURE);
	
	sprintf(
		str
		, "z64FC_%c_%s_%s"
		, (is_color) ? 'c' : 'a'
		, op_name
		, sanitize_name(var->name)
	);
	
	return str;
}

static
void
declare_step_func(
	struct setcombineVariable *var
	, int is_color
	, char *op
	, char *op_name
)
{
	/* skip uninitialized elements */
	if (!var->name)
		return;
	
	char *in_type = "struct vec4";//(is_color) ? "vec4" : "float";
	char *members = (is_color) ? "xyz" : "\0";
	fprintf(
		stdout
		, "static void %s(struct n64ctx *mat, %s *v) {\n"
		, step_func_name(var, is_color, op_name)
		, in_type
	);
	
	#ifndef NDEBUG
	fprintf(
		stdout
		, "	fprintf(stderr, \"%s\\n\");\n"
		, step_func_name(var, is_color, op_name)
	);
	#endif
	
	/* TODO print mat->%s or something */
	/* alpha, floating point operation */
	if (!*members)
	{
		fprintf(
			stdout
			, "	v->w %s %s;\n"
			, op
			, make_FC_operand(var)
		);
	}
	
	/* operate on each member of frag */
	while(*members)
	{
		/* *v.member += primcolor*/
		fprintf(
			stdout
			, "	v->%c %s %s"
			, *members
			, op
			, make_FC_operand(var)
		);
		
		/* .member */
		if (var->is_vec3)
		{
			fprintf(
				stdout
				, ".%c"
				, *members
			);
		}
		fprintf(
			stdout
			, ";\n"
		);
		
		/* advance to next member */
		members += 1;
	}
	
	/* clamping operations */
	members = (is_color) ? "xyz" : "w";
	while (*members)
	{
		if (*op != '=')
		{
#if (Z64GEN_CLAMPER == Z64GEN_CLAMPER_FABS)
			fprintf(
				stdout
				, "	v->%c = 1.0f - fabs(v->%c - 1.0f);\n"
				, *members
				, *members
			);
#elif (Z64GEN_CLAMPER == Z64GEN_CLAMPER_FMIN_FMAX)
			fprintf(
				stdout
				, "	v->%c = fmin(fmax(v->%c, 0.0f), 1.0f);\n"
				, *members
				, *members
			);
#elif (Z64GEN_CLAMPER == Z64GEN_CLAMPER_DEPENDS)
			if (*op == '-')
				fprintf(
					stdout
					, "	v->%c = fmax(v->%c, 0.0f);\n"
					, *members
					, *members
				);
			if (*op == '+')
				fprintf(
					stdout
					, "	v->%c = fmin(v->%c, 1.0f);\n"
					, *members
					, *members
				);
#endif
		}
		++members;
	}
	
	fprintf(
		stdout
		, "}\n"
	);
}

struct setcombineVariable setcombineColorVariable[] = {
	[G_CCMUX_COMBINED] = { .name = "combined", .is_vec3 = 1, .is_combined = 1 }
	, [G_CCMUX_TEXEL0] = { .name = "texel0", .is_vec3 = 1, .is_texel0 = 1 }
	, [G_CCMUX_TEXEL1] = { .name = "texel1", .is_vec3 = 1, .is_texel1 = 1 }
	, [G_CCMUX_PRIMITIVE] = { .name = "primcolor", .is_vec3 = 1 }
	, [G_CCMUX_SHADE] = { .name = "vertcolor", .is_vec3 = 1 }
	, [G_CCMUX_ENVIRONMENT] = { .name = "envcolor", .is_vec3 = 1 }
	, [G_CCMUX_0] = { .name = "0", .is_0 = 1 }
	
	, [G_CCMUX_TEXEL0_ALPHA] = { .name = "texel0.w", .only_a = 1, .is_texel0 = 1 }
	, [G_CCMUX_TEXEL1_ALPHA] = { .name = "texel1.w", .only_a = 1, .is_texel1 = 1 }
	, [G_CCMUX_PRIMITIVE_ALPHA] = { .name = "primcolor.w", .only_a = 1 }
	, [G_CCMUX_SHADE_ALPHA] = { .name = "vertcolor.w", .only_a = 1 }
	, [G_CCMUX_ENV_ALPHA] = { .name = "envcolor.w", .only_a = 1 }
	
	, [G_CCMUX_LOD_FRACTION] = { .name = "lodfrac" }
	, [G_CCMUX_PRIM_LOD_FRAC] = { .name = "primLodfrac" }
	, [G_CCMUX_K5] = { .name = "k5" }

	/* all of these are [6]; no clue what to do with it */
	, [G_CCMUX_1] = { .name = "1", .is_1 = 1 }
#if 1
	, [G_CCMUX_CENTER_ALIAS] = { .name = "center", .is_vec3 = 1 }
	, [G_CCMUX_SCALE_ALIAS] = { .name = "scale", .is_vec3 = 1 }
#endif

	/* all of these are [7] */
	, [G_CCMUX_COMBINED_ALPHA] = { .name = "combined.w", .only_a = 1, .is_combined = 1 }
#if 1
	, [G_CCMUX_NOISE_ALIAS] = { .name = "noise", .is_vec3 = 1 }
	, [G_CCMUX_K4_ALIAS] = { .name = "k4" }
#endif
};

struct setcombineVariable setcombineAlphaVariable[] = {
	[G_ACMUX_TEXEL0] = { .name = "texel0.w", .is_texel0 = 1 }
	, [G_ACMUX_TEXEL1] = { .name = "texel1.w", .is_texel1 = 1 }
	, [G_ACMUX_PRIMITIVE] = { .name = "primcolor.w" }
	, [G_ACMUX_SHADE] = { .name = "vertcolor.w" }
	, [G_ACMUX_ENVIRONMENT] = { .name = "envcolor.w" }
	, [G_ACMUX_0] = { .name = "0", .is_0 = 1 }

	/* all of these are [0] */
	, [G_ACMUX_COMBINED] = { .name = "combined.w", .is_combined = 1 }
#if 1
	, [G_ACMUX_LOD_FRACTION_ALIAS] = { .name = "lodfrac" }
#endif

	/* all of these are [6] */
	, [G_ACMUX_1] = { .name = "1", .is_1 = 1 }
#if 1
	, [G_ACMUX_PRIM_LOD_FRAC_ALIAS] = { .name = "primLodfrac" }
#endif
};

/* options 0 - 5 are the same for all colors */
#define SETCOMBINECOLOR_0_5\
		, .list[0] = { G_CCMUX_COMBINED, "G_CCMUX_COMBINED", "Result" }\
		, .list[1] = { G_CCMUX_TEXEL0, "G_CCMUX_TEXEL0", "Texture 0" }\
		, .list[2] = { G_CCMUX_TEXEL1, "G_CCMUX_TEXEL1", "Texture 1" }\
		, .list[3] = { G_CCMUX_PRIMITIVE, "G_CCMUX_PRIMITIVE", "Prim Color" }\
		, .list[4] = { G_CCMUX_SHADE, "G_CCMUX_SHADE", "Vertex Shading" }\
		, .list[5] = { G_CCMUX_ENVIRONMENT, "G_CCMUX_ENVIRONMENT", "Env Color" }

/* options 1 - 5 are the same for all alpha options */
#define SETCOMBINEALPHA_1_5\
		, .list[1] = { G_ACMUX_TEXEL0, "G_ACMUX_TEXEL0", "Texel 0" }\
		, .list[2] = { G_ACMUX_TEXEL1, "G_ACMUX_TEXEL1", "Texture 1" }\
		, .list[3] = { G_ACMUX_PRIMITIVE, "G_ACMUX_PRIMITIVE", "Prim Alpha" }\
		, .list[4] = { G_ACMUX_SHADE, "G_ACMUX_SHADE", "Vertex Alpha" }\
		, .list[5] = { G_ACMUX_ENVIRONMENT, "G_ACMUX_ENVIRONMENT", "Env Alpha" }

static
struct setcombineOptions
	setcombineColorA = {
		.num = 9
		SETCOMBINECOLOR_0_5
		, .list[6] = { G_CCMUX_1, "G_CCMUX_1", "White (1.0)" }
		, .list[7] = { G_CCMUX_NOISE, "G_CCMUX_NOISE", "Noise (unemulated)" }
		, .list[8] = { G_CCMUX_0, "G_CCMUX_0", "Black (0.0)" }
		, .remap[G_CCMUX_NOISE] = (G_CCMUX_NOISE_ALIAS + 1)
	}
	, setcombineColorB = {
		.num = 9
		SETCOMBINECOLOR_0_5
		, .list[6] = { G_CCMUX_CENTER, "G_CCMUX_CENTER", "Center (unemulated)" }
		, .list[7] = { G_CCMUX_K4, "G_CCMUX_K4", "K4 (unemulated)" }
		, .list[8] = { G_CCMUX_0, "G_CCMUX_0", "Black (0.0)" }
		, .remap[G_CCMUX_CENTER] = (G_CCMUX_CENTER_ALIAS + 1)
		, .remap[G_CCMUX_K4] = (G_CCMUX_K4_ALIAS + 1)
	}
	, setcombineColorC = {
		.num = 17
		SETCOMBINECOLOR_0_5
		, .list[6] = { G_CCMUX_SCALE, "G_CCMUX_SCALE", "Scale (unemulated)" }
		, .list[7] = { G_CCMUX_COMBINED_ALPHA, "G_CCMUX_COMBINED_ALPHA", "Result (Alpha Channel)" }\
		, .list[8] = { G_CCMUX_TEXEL0_ALPHA, "G_CCMUX_TEXEL0_ALPHA", "Texture 0 (Alpha Channel)" }\
		, .list[9] = { G_CCMUX_TEXEL1_ALPHA, "G_CCMUX_TEXEL1_ALPHA", "Texture 1 (Alpha Channel)" }\
		, .list[10] = { G_CCMUX_PRIMITIVE_ALPHA, "G_CCMUX_PRIMITIVE_ALPHA", "Prim Alpha" }\
		, .list[11] = { G_CCMUX_SHADE_ALPHA, "G_CCMUX_SHADE_ALPHA", "Vertex Alpha" }\
		, .list[12] = { G_CCMUX_ENV_ALPHA, "G_CCMUX_ENV_ALPHA", "Env Alpha" }\
		, .list[13] = { G_CCMUX_LOD_FRACTION, "G_CCMUX_LOD_FRACTION", "Lodfrac (unemulated)" }\
		, .list[14] = { G_CCMUX_PRIM_LOD_FRAC, "G_CCMUX_PRIM_LOD_FRAC", "Prim Lodfrac" }\
		, .list[15] = { G_CCMUX_K5, "G_CCMUX_K5", "K5 (unemulated)" }
		, .list[16] = { G_CCMUX_0, "G_CCMUX_0", "Black (0.0)" }
		, .remap[G_CCMUX_SCALE] = (G_CCMUX_SCALE_ALIAS + 1)
	}
	, setcombineColorD = {
		.num = 8
		SETCOMBINECOLOR_0_5
		, .list[6] = { G_CCMUX_1, "G_CCMUX_1", "White (1.0)" }
		, .list[7] = { G_CCMUX_0, "G_CCMUX_0", "Black (0.0)" }
	}
	, setcombineAlphaA = {
		.num = 8
		, .list[0] = { G_ACMUX_COMBINED, "G_ACMUX_COMBINED", "Combined (Alpha Channel)" }
		SETCOMBINEALPHA_1_5
		, .list[6] = { G_ACMUX_1, "G_ACMUX_1", "1.0" }
		, .list[7] = { G_ACMUX_0, "G_ACMUX_0", "0.0" }
	}
	, setcombineAlphaB = {
		.num = 8
		, .list[0] = { G_ACMUX_COMBINED, "G_ACMUX_COMBINED", "Combined (Alpha Channel)" }
		SETCOMBINEALPHA_1_5
		, .list[6] = { G_ACMUX_1, "G_ACMUX_1", "1.0" }
		, .list[7] = { G_ACMUX_0, "G_ACMUX_0", "0.0" }
	}
	, setcombineAlphaC = {
		.num = 8
		, .list[0] = { G_ACMUX_LOD_FRACTION, "G_ACMUX_LOD_FRACTION", "Lodfrac (unemulated)" }
		SETCOMBINEALPHA_1_5
		, .list[6] = { G_ACMUX_PRIM_LOD_FRAC, "G_ACMUX_PRIM_LOD_FRAC", "Prim Lodfrac" }
		, .list[7] = { G_ACMUX_0, "G_ACMUX_0", "0.0" }
		, .remap[G_ACMUX_LOD_FRACTION] = (G_ACMUX_LOD_FRACTION_ALIAS + 1)
		, .remap[G_ACMUX_PRIM_LOD_FRAC] = (G_ACMUX_PRIM_LOD_FRAC_ALIAS + 1)
	}
	, setcombineAlphaD = {
		.num = 8
		, .list[0] = { G_ACMUX_COMBINED, "G_ACMUX_COMBINED", "Combined (Alpha Channel)" }
		SETCOMBINEALPHA_1_5
		, .list[6] = { G_ACMUX_1, "G_ACMUX_1", "1.0" }
		, .list[7] = { G_ACMUX_0, "G_ACMUX_0", "0.0" }
	};
;

static
char *
declare_step_func_list_item(
	struct setcombineVariable *var
	, int idx
	, int is_color
	, char *op_name
)
{
	/* skip uninitialized elements */
	if (!var->name)
		return 0;
	
	static char *str = 0;
	
	if (!str)
		str = malloc(256);
	
	sprintf(
		str
		, "[%d] = z64FC_%c_%s_%s"
		, idx
		, (is_color) ? 'c' : 'a'
		, op_name
		, sanitize_name(var->name)
	);
	
	return str;
}

static
void
declare_step_func_list(
	struct setcombineVariable *list
	, int count
	, int is_color
	, char *op_name
)
{
	fprintf(
		stdout
		, "void (*z64FC_%c_oplist_%s[])(Z64OPARGS) = {\n"
		, (is_color) ? 'c' : 'a'
		, op_name
	);
	int comma = 0;
	for (struct setcombineVariable *v = list
			; v - list < count
			; ++v, comma = 1) {
		size_t idx = v - list;
		char *test;
		test = declare_step_func_list_item(v, idx, is_color, op_name);
		if (!test)
			continue;
		fprintf(stdout, "\t");
		if (comma)
			fprintf(stdout, ", ");
		fprintf(
			stdout
			, "%s\n"
			, test
		);
		comma = 1;
	}
	fprintf(
		stdout
		, "};\n"
	);
}

static
struct vec4
z64frag(
	struct swr_shaderProgram *prog
	, struct swr_context *ctx
	, struct rs_vertex *frag
)
{
	(void)prog;
	struct vec4 result = {0};
	
	return result;
}

#define Z64FRAG_START(NAME)           \
static                                \
struct vec4                           \
z64frag##NAME(                        \
	struct swr_shaderProgram *prog     \
	, struct swr_context *ctx          \
	, struct rs_vertex *frag           \
)                                     \
{                                     \
	(void)prog;                        \
	struct vec4 result;                \
	struct n64ctx *n64 = ctx->n64ctx; \
	struct n64rast *rast = n64->rast; \
	/* vertex colors */ \
	g_n64ctx->vertcolor = frag->attribs[ATTRIB_COLOR]; \
	/* vertex alpha */ \
	g_n64ctx->vertcolor.w = frag->attribs[ATTRIB_COLOR].w;

#define Z64FRAG_END                                  \
	/*fprintf(stderr, " >> BEGIN MIXING <<\n");*/ \
	for (int i=0; i < rast->oplist_count; ++i)        \
	{                                                 \
		rast->oplist_ptr[i](n64, &result);             \
	}                                                 \
	/*fprintf(stderr, "texel0{%f} primw{%f}\n", n64->texel0.w, n64->primcolor.w);*/ \
	/* clamp result to 0.0 - 1.0 range */ \
	result.x = fmin(fmax(result.x, 0.0f), 1.0f); \
	result.y = fmin(fmax(result.y, 0.0f), 1.0f); \
	result.z = fmin(fmax(result.z, 0.0f), 1.0f); \
	result.w = fmin(fmax(result.w, 0.0f), 1.0f); \
	/*result = (struct vec4){n64->texel0.x+n64->texel1.x, n64->texel0.y+n64->texel1.y, n64->texel0.z+n64->texel1.z, n64->texel0.w+n64->texel1.w };*/ /* tests manual adding */ \
	/*result.x = result.y = result.z = result.w;*/ \
	return result;                                    \
}

static
inline
void
z64frag_h_tx0(struct swr_context *ctx, struct rs_vertex *frag)
{
	struct n64ctx *n64 = ctx->n64ctx;
	struct n64rast *rast = n64->rast;
	
	/* TODO ugly hard-coded texel selection */
	n64->texel0 = (struct vec4){1,1,1,1};
	if (!rast->func_tile_sample[0])
		return;
	assert(rast->func_tile_sample[0]);
	rast->func_tile_sample[0](
		&n64->tile[0]
		, &frag->attribs[ATTRIB_TEX0]
		, &n64->texel0
	);
}

static
inline
void
z64frag_h_tx1(struct swr_context *ctx, struct rs_vertex *frag)
{
	struct n64ctx *n64 = ctx->n64ctx;
	struct n64rast *rast = n64->rast;
	n64->texel1 = (struct vec4){1,1,1,1};
	if (!rast->func_tile_sample[1])
		return;
	assert(rast->func_tile_sample[1]);
	rast->func_tile_sample[1](
		&n64->tile[1]
		, &frag->attribs[ATTRIB_TEX1]
		, &n64->texel1
	);
}

/* add red highlight into result */
static void z64FC_c_add_highlight(struct n64ctx *mat, struct vec4 *v) {
	v->x = 1.0f;
	if (v->y >= 0.50 && v->z >= 0.50)
		v->y = v->z = 0;
}

/* copy current color and alpha into combined */
static void z64FC_b_cpy_combined(struct n64ctx *mat, struct vec4 *v) {
	//fprintf(stderr, "z64FC_b_cpy_combined\n");
	mat->combined = *v;
}

/* copy current color into combined */
static void z64FC_c_cpy_combined(struct n64ctx *mat, struct vec4 *v) {
	//fprintf(stderr, "z64FC_c_cpy_combined\n");
	mat->combined.x = v->x;
	mat->combined.y = v->y;
	mat->combined.z = v->z;
}

/* copy current alpha into combined */
static void z64FC_a_cpy_combined(struct n64ctx *mat, struct vec4 *v) {
	//fprintf(stderr, "z64FC_a_cpy_combined\n");
	mat->combined.w = v->w;
}


/* samples only texel 0 */
Z64FRAG_START(_tx0)
	z64frag_h_tx0(ctx, frag);
Z64FRAG_END

/* samples only texel 1 */
Z64FRAG_START(_tx1)
	z64frag_h_tx1(ctx, frag);
Z64FRAG_END

/* samples both texel0 and texel1 */
Z64FRAG_START(_tx0_tx1)
	z64frag_h_tx0(ctx, frag);
	z64frag_h_tx1(ctx, frag);
Z64FRAG_END

/* samples no texture */
Z64FRAG_START(_notex)
Z64FRAG_END

/* this is a separate function in case I ever decide to
   make options able to be rearranged */
static
int
combineOptionsVarInt(struct setcombineOptions *options, int v)
{
	if (options->remap[v])
		v = options->remap[v] - 1;
	return options->list[v].value;
}

static
struct setcombineVariable *
combineOptionsVar(struct setcombineOptions *options, int v, int is_color)
{
	if (v >= options->num)
		v = options->num - 1;
	
	v = combineOptionsVarInt(options, v);
	
	if (is_color)
		return &setcombineColorVariable[v];
	
	return &setcombineAlphaVariable[v];
}

/* notes regarding a cycle */
struct setcombineCycle
{
	char *code; /* code that was generated */
	int uses_combined;
	int only_combined;
	int uses_texel0;
	int uses_texel1;
};

static
struct setcombineCycle *
setcombineCycle_new(int want_code)
{
	struct setcombineCycle *r = malloc(sizeof(*r));
	
	if (!r)
		return 0;
	
	/* if we don't want code blocks */
	if (!want_code)
	{
		r->code = 0;
		return r;
	}
	
	/* 4kb is plenty for a few lines of code */
	r->code = malloc(4096);
	
	if (!r->code)
		return 0;
	
	return r;
}

static
void
oplist_push(struct n64ctx *ctx, void f(Z64OPARGS))
{
	struct n64rast *mat = ctx->rast;
	mat->oplist[mat->oplist_count] = f;
	mat->oplist_count += 1;
}

static
void
oplist_insert(struct n64rast *mat, void f(Z64OPARGS), int at)
{
	/* copy everything along by 1 */
	memmove(
		mat->oplist + at + 1
		, mat->oplist + at
		, (mat->oplist_count - at) * sizeof(mat->oplist[0])
	);
	
	mat->oplist[at] = f;
	mat->oplist_count += 1;
}

static
void
oplist_omit(struct n64rast *mat, int from, int to)
{
	/* if we are omitting entries at the end, just change count */
	if (to == mat->oplist_count)
	{
		mat->oplist_count = from;
		return;
	}
	/* if we are omitting entries at the beginning, advance ptr */
	if (from == 0)
	{
		mat->oplist_ptr += to;
		mat->oplist_count -= to;
		return;
	}
	/* otherwise, we are omitting entries in the middle */
	memmove(
		mat->oplist + from
		, mat->oplist + to
		, (mat->oplist_count - to) * sizeof(mat->oplist[0])
	);
}

static
void
frag_operation_oplist(
	struct n64ctx *mat
	, int is_color
	, struct setcombineVariable *v
	, int op
)
{
	void (*f)(Z64OPARGS) = 0;
	
	/* if no variable provided, set = default white 1.0 */
	if (!v)
	{
		if (is_color)
			f = z64FC_c_oplist_set[G_CCMUX_1];
		
		else
			f = z64FC_a_oplist_set[G_ACMUX_1];
		
		oplist_push(mat, f);
		return;
	}
	
	if (is_color)
	{
		size_t idx = v - setcombineColorVariable;
		switch (op)
		{
		case '+':
			f = z64FC_c_oplist_add[idx];
			break;
		case '-':
			f = z64FC_c_oplist_sub[idx];
			break;
		case '*':
			f = z64FC_c_oplist_mul[idx];
			break;
		case '=':
			f = z64FC_c_oplist_set[idx];
			break;
		}
	}
	else
	{
		size_t idx = v - setcombineAlphaVariable;
		switch (op)
		{
		case '+':
			f = z64FC_a_oplist_add[idx];
			break;
		case '-':
			f = z64FC_a_oplist_sub[idx];
			break;
		case '*':
			f = z64FC_a_oplist_mul[idx];
			break;
		case '=':
			f = z64FC_a_oplist_set[idx];
			break;
		}
	}
	
	oplist_push(mat, f);
}

static
void
frag_operation(
	struct n64ctx *mat
	, char *dst
	, int is_color
	, char *members
	, struct setcombineVariable *var
	, char *operation
)
{
	/* if not generating C code, must be oplist */
	if (!dst)
	{
		frag_operation_oplist(mat, is_color, var, *operation);
		return;
	}
	/* no variable provided, initialize to default white 1.0 */
	if (!var)
	{
		while (*members)
		{
			char twochar[2] = {*members, '\0'};
			strcat(dst, "\t"FRAG_NAME);
			strcat(dst, ".");
			strcat(dst, twochar);
			strcat(dst, "=1;\n");
			
			/* advance to next member */
			members += 1;
		}
		return;
	}
	/* operate on each member of frag */
	while(*members)
	{
		char twochar[2] = {*members, '\0'};
		strcat(dst, "\t"FRAG_NAME);
		strcat(dst, ".");
		strcat(dst, twochar);
		strcat(dst, operation);
		
		/* ex: frag.member += primcolor.member */
		if (var->is_vec3)
		{
			strcat(dst, var->name);
			strcat(dst, ".");
			strcat(dst, twochar);
		}
		/* ex: frag.member += primcolor.a */
		else
		{
			strcat(dst, var->name);
		}
		strcat(dst, ";\n");
		
		/* advance to next member */
		members += 1;
	}
}
	
static
void
generate_fragment_operations(
	struct n64ctx *mat
	, char *code
	, int is_color
	, struct setcombineVariable *a
	, struct setcombineVariable *b
	, struct setcombineVariable *c
	, struct setcombineVariable *d
)
{
	/* value is initially unset; use this to track if we've set it */
	int has_set = 0;
	char *members = (is_color) ? "xyz" : "w";
	
#if 0
	fprintf(
		stderr
		, "cycle ( %s , %s , %s , %s )\n"
		, a->name
		, b->name
		, c->name
		, d->name
	);
#endif
	
	/* only if we are multiplying by non-zero, do we do (a - b) * c
	 * also, don't evaluate (a - b) if they are the same */
	if (!c->is_0 && a != b)
	{
		/* frag = a */
		frag_operation(mat, code, is_color, members, a, "=");
		has_set = 1;
		/* do subtraction only if the value isn't 0 */
		if (!b->is_0)
		{
			/* frag -= b */
			frag_operation(mat, code, is_color, members, b, "-=");
		}
		/* multiply only if it's a value other than 1 */
		if (!c->is_1)
		{
			/* frag *= c */
			frag_operation(mat, code, is_color, members, c, "*=");
		}
	}
	
	/* only if we are adding something other than 0, do we add */
	if (!d->is_0)
	{
		/* variable still hasn't been set, so set it here */
		if (!has_set)
		{
			/* frag = d */
			frag_operation(mat, code, is_color, members, d, "=");
			has_set = 1;
		}
		else
		{
			/* frag += d */
			frag_operation(mat, code, is_color, members, d, "+=");
		}
	}
	
	/* if we did all these operations and it still hasn't been set, set a default */
	if (!has_set)
	{
		/* initialize to default */
		frag_operation(mat, code, is_color, members, 0, "=");
		has_set = 1;
	}
}

static
void
combineCycleExec(
	struct n64ctx *mat
	, struct setcombineCycle *cycle
	, int is_color
	, int _a
	, int _b
	, int _c
	, int _d
)
{
	char *code;
	struct setcombineVariable *a;
	struct setcombineVariable *b;
	struct setcombineVariable *c;
	struct setcombineVariable *d;
	
	/* zero-initialize the notes */
	code = cycle->code;
	memset(cycle, 0, sizeof(*cycle));
	cycle->code = code;
	
	if (is_color)
	{
		a = combineOptionsVar(&setcombineColorA, _a, is_color);
		b = combineOptionsVar(&setcombineColorB, _b, is_color);
		c = combineOptionsVar(&setcombineColorC, _c, is_color);
		d = combineOptionsVar(&setcombineColorD, _d, is_color);
	}
	else
	{
		a = combineOptionsVar(&setcombineAlphaA, _a, is_color);
		b = combineOptionsVar(&setcombineAlphaB, _b, is_color);
		c = combineOptionsVar(&setcombineAlphaC, _c, is_color);
		d = combineOptionsVar(&setcombineAlphaD, _d, is_color);
	}
	
	/* note cycle uses first cycle's result */
	cycle->uses_combined =
		  a->is_combined
		| b->is_combined
		| c->is_combined
		| d->is_combined
	;
	
	/* note texel usage */
	cycle->uses_texel0 =
		  a->is_texel0
		| b->is_texel0
		| c->is_texel0
		| d->is_texel0
	;
	cycle->uses_texel1 =
		  a->is_texel1
		| b->is_texel1
		| c->is_texel1
		| d->is_texel1
	;
	
	/* these cycle 1 combos yield the same as cycle 0 */
	if (a->is_combined && b->is_0 && c->is_1 && d->is_0)
		cycle->only_combined = 1; /* (combined - 0) * 1 + 0 */
	else if (a->is_1 && b->is_0 && c->is_combined && d->is_0)
		cycle->only_combined = 1; /* (1 - 0) * combined + 0 */
	//else if (a->is_1 && b->is_1 && d->is_combined)
	//	cycle->only_combined = 1; /* (1 - 1) * x + combined */
	//else if (a->is_0 && b->is_0 && d->is_combined)
	//	cycle->only_combined = 1; /* (0 - 0) * x + combined */
	else if (c->is_0 && d->is_combined)
		cycle->only_combined = 1; /* (a - b) * 0 + combined */
	else if (a == b && d->is_combined)
		cycle->only_combined = 1; /* ((a - b)=0) * y + combined */
	
	/* generate C function body and store in code, or, if !code,
	 * it creates an oplist */
	generate_fragment_operations(mat, code, is_color, a, b, c, d);
}

static
void
n64ctxgen(struct n64ctx *ctx)
{
	static struct setcombineCycle *cycle0c = 0;
	static struct setcombineCycle *cycle0a = 0;
	static struct setcombineCycle *cycle1c = 0;
	static struct setcombineCycle *cycle1a = 0;
	struct setcombineBitfield *b = &ctx->combiner.bitfield;
	
	if (!cycle0c)
		cycle0c = setcombineCycle_new(1);
	
	if (!cycle0a)
		cycle0a = setcombineCycle_new(1);
	
	if (!cycle1c)
		cycle1c = setcombineCycle_new(1);
	
	if (!cycle1a)
		cycle1a = setcombineCycle_new(1);
	
	if (!cycle0c || !cycle0a || !cycle1c || !cycle1a)
		exit(EXIT_FAILURE);
	
	/* begin writing function */
	fprintf(
		stdout ,
		"static\n"
		"struct vec4\n"
		"shader_unlit_fragment(\n"
		"	const struct swr_shaderProgram *prog\n"
		"	, const struct swr_context *ctx\n"
		"	, const rs_vertex *frag\n"
		")\n"
		"{\n"
		"	(void)prog;\n"
		"	struct vec4 "FRAG_NAME";\n"
	);
	
	/* execute color cycles */
	combineCycleExec(ctx, cycle0c, 1, b->a0, b->b0, b->c0, b->d0);
	combineCycleExec(ctx, cycle1c, 1, b->a1, b->b1, b->c1, b->d1);
	
	/* execute alpha cycles */
	combineCycleExec(ctx, cycle0a, 0, b->Aa0, b->Ab0, b->Ac0, b->Ad0);
	combineCycleExec(ctx, cycle1a, 0, b->Aa1, b->Ab1, b->Ac1, b->Ad1);
	
	/* if cycle 0 uses `combined` anywhere, declare and initialize it */
	if (cycle0c->uses_combined || cycle0a->uses_combined)
		fprintf(stdout, "\t""struct vec4 combined={1,1,1,1};\n");
	/* declare `combined` as well if we're going to use it */
	else if (!cycle1c->only_combined && cycle1c->uses_combined)
		fprintf(stdout, "\t""struct vec4 combined;\n");
	
	/* write cycle 0 only if cycle 1 actually uses the result */
	if (cycle1c->uses_combined)
		fprintf(stdout, cycle0c->code);
	
	/* write cycle 1 only if it does operations on cycle 0's result, *
	 * or ignores it completely                                      */
	if (!cycle1c->only_combined)
	{
		/* if we are making use of the combined result, combined = outcolor */
		if (cycle1c->uses_combined)
			fprintf(
				stdout ,
				"\t""combined.x = "FRAG_NAME".x;\n"
				"\t""combined.y = "FRAG_NAME".y;\n"
				"\t""combined.z = "FRAG_NAME".z;\n"
			);
		fprintf(stdout, cycle1c->code);
	}
	
	/* now repeat exactly that, but alpha */
	/* write cycle 0 only if cycle 1 actually uses the result */
	if (cycle1a->uses_combined)
		fprintf(stdout, cycle0a->code);
	
	/* write cycle 1 only if it does operations on cycle 0's result, *
	 * or ignores it completely                                      */
	if (!cycle1a->only_combined)
	{
		/* if we are making use of the combined result, combined = outcolor */
		if (cycle1a->uses_combined)
			fprintf(
				stdout ,
				"\t""combined.w = "FRAG_NAME".w;\n"
			);
		fprintf(stdout, cycle1c->code);
	}
	
	/* this is the end of the function */
	fprintf(
		stdout ,
		"	return "FRAG_NAME";\n"
		"}\n"
	);
}


static
inline
void
blend_enable_test(struct n64ctx *ctx)
{
	struct swr_context *swr = ctx->rctx;
	
	/* clear blending bit */
	swr->flags &= ~BLEND_ENABLE;
	
	/* set blending bit */
	if (ctx->othermode.indep.cvg_x_alpha || /*ctx->force_blending_override ||*/ /*ctx->combiner.force_blending ||*/ ctx->othermode.indep.force_bl)
		swr->flags |= BLEND_ENABLE;
}


static
void
n64ctx_generate_oplist(struct n64ctx *ctx)
{
#define QNOTE(A) uses_texel0 |= A->uses_texel0, uses_texel1 |= A->uses_texel1
	
	static struct setcombineCycle *cycle0c = 0;
	static struct setcombineCycle *cycle0a = 0;
	static struct setcombineCycle *cycle1c = 0;
	static struct setcombineCycle *cycle1a = 0;
	int omitted_a = 0;
	int omitted_c = 0;
	int uses_texel0 = 0;
	int uses_texel1 = 0;
	struct setcombineBitfield *b = &ctx->combiner.bitfield;
	struct n64rast *m = ctx->rast;
	//int oplist_divider;
	
	struct
	{
		int before_0c;
		int before_0a;
		int before_1c;
		int before_1a;
	} divider;

	/* zero operations in oplist */
	m->oplist_count = 0;
	m->oplist_ptr = m->oplist;
	
	if (!cycle0c)
		cycle0c = setcombineCycle_new(0);
	
	if (!cycle0a)
		cycle0a = setcombineCycle_new(0);
	
	if (!cycle1c)
		cycle1c = setcombineCycle_new(0);
	
	if (!cycle1a)
		cycle1a = setcombineCycle_new(0);
	
	if (!cycle0c || !cycle0a || !cycle1c || !cycle1a)
		exit(EXIT_FAILURE);
	
	/* execute first cycle */
	divider.before_0c = m->oplist_count;
	combineCycleExec(ctx, cycle0c, 1, b->a0, b->b0, b->c0, b->d0);
	divider.before_0a = m->oplist_count;
	combineCycleExec(ctx, cycle0a, 0, b->Aa0, b->Ab0, b->Ac0, b->Ad0);
	
	/* store where in oplist we change from cycle 0 to cycle 1 */
	//oplist_divider = m->oplist_count;
	
	/* execute second cycle */
	divider.before_1c = m->oplist_count;
	combineCycleExec(ctx, cycle1c, 1, b->a1, b->b1, b->c1, b->d1);
	divider.before_1a = m->oplist_count;
	combineCycleExec(ctx, cycle1a, 0, b->Aa1, b->Ab1, b->Ac1, b->Ad1);
	
	/* alpha is derived from texel, so force blending */
	ctx->combiner.force_blending =
		cycle0a->uses_texel0
		|| cycle1a->uses_texel0
		|| cycle0a->uses_texel1
		|| cycle1a->uses_texel1
	;
	blend_enable_test(ctx);
	
	/* determine whether texel0 and texel1 are used */
	
	/* 1c uses 0c's result */
	if (cycle1c->uses_combined)
	{
		/* 0c is used */
		QNOTE(cycle0c);
		
		/* 1c uses more than just combined result */
		if (!cycle1c->only_combined)
			QNOTE(cycle1c);
	}
	/* only 1c is used */
	else
		QNOTE(cycle1c);
	
	/* 1a uses 0a's result */
	if (cycle1a->uses_combined)
	{
		/* 0a is used */
		QNOTE(cycle0a);
		
		/* 1a uses more than just combined result */
		if (!cycle1a->only_combined)
			QNOTE(cycle1a);
	}
	/* only 1a is used */
	else
		QNOTE(cycle1a);
	
	/* select fragment functions that provide texel 0, *
	 * texel1, etc, based on what is used in combiner. */
	
	if (uses_texel0 && uses_texel1)
		m->func_frag = z64frag_tx0_tx1;
	else if(uses_texel0)
		m->func_frag = z64frag_tx0;
	else if(uses_texel1)
		m->func_frag = z64frag_tx1;
	else
		m->func_frag = z64frag_notex;
	
	/* insert z64FC_b_cpy_combined at divider if cycle 1 
	   uses cycle 0's result, or, if cycle 1 is literally just
	   cycle 0, omit entries after divider, or, if cycle 0
	   is discarded in favor of cycle 1, move everything after
	   divider to front of oplist */
	
	/* if 1c is literally 0c, omit 1c */
	if (cycle1c->only_combined)
	{
		/* 0c is used */
		//QNOTE(cycle0c);
		
		omitted_c = 1;
		/* both 1c and 1a can be omitted */
		if (cycle1a->only_combined)
		{
			/* 0a is used */
			//QNOTE(cycle0a);
			
			m->oplist_count = divider.before_1c;
			//oplist_omit(m, divider.before_1c, m->oplist_count);
			omitted_a = 1;
		}
		/* omit only 1c */
		else
		{
			/* 1a is used */
			//QNOTE(cycle1a);
			
			oplist_omit(m, divider.before_1c, divider.before_1a);
		}
	}
	/* omit only 1a */
	else if (cycle1a->only_combined)
	{
		/* 1c is used */
		//QNOTE(cycle1c);
		
		omitted_a = 1;
		m->oplist_count = divider.before_1a;
		//oplist_omit(m, divider.before_1a, m->oplist_count);
	}
	
	/* if 1c doesn't use 0c's result, omit 0c */
	if (!cycle1c->uses_combined)
	{
		/* 1c is used */
		//QNOTE(cycle1c);
		
		omitted_c = 1;
		/* both 0c and 0a can be omitted */
		if (!cycle1a->uses_combined)
		{
			/* 1a is used */
			//QNOTE(cycle1a);
			
			omitted_a = 1;
			m->oplist_ptr += divider.before_1c;
			m->oplist_count -= divider.before_1c;
		}
		/* omit only 0c */
		else
		{
			/* 1a is used */
			//QNOTE(cycle1a);
			
			m->oplist_ptr += divider.before_0a;
			m->oplist_count -= divider.before_0a;
		}
	}
	/* omit only 0a */
	else if (!cycle1a->uses_combined)
	{
		/* 1a is used */
		//QNOTE(cycle1a);
		
		omitted_a = 1;
		oplist_omit(m, divider.before_0a, divider.before_1c);
	}
	
	/* if c1 and a1 both use the c0 and a0 results,
	 * insert z64FC_b_cpy_combined into oplist */
	if (!omitted_a && !omitted_c)
		oplist_insert(m, z64FC_b_cpy_combined, divider.before_1c);
	/* if only c1 uses the c0 result, insert z64FC_c_cpy_combined */
	else if (!omitted_c)
		oplist_insert(m, z64FC_c_cpy_combined, divider.before_1c);
	/* if only a1 uses the a0 result, insert z64FC_a_cpy_combined */
	else if (!omitted_a)
		oplist_insert(m, z64FC_a_cpy_combined, divider.before_1c);
#undef QNOTE
	
	/* lastly, add red for highlight, if highlighting is desired */
	if (ctx->highlight_on)
		oplist_push(ctx, z64FC_c_add_highlight);
}


#ifndef Z64MATGEN_BUILDING_LIST
static
void
func_triangle(
	struct n64ctx *ctx
	, int v0
	, int v1
	, int v2
)
{
	struct n64vert *vbuf = ctx->vbuf;
	struct n64tile *tile = ctx->tile;
	int vx[] = {v0, v1, v2};
	
	ia_begin(ctx->rctx);

	for (int i=0; i < 3; ++i)
	{
		struct n64vert *v = vbuf + vx[i];
		
		ia_color(
			ctx->rctx
			, v->col.x
			, v->col.y
			, v->col.z
			, v->col.w
		);
		
		/* adding ulS and ulT for texture scrolling */
		ia_texcoord(
			ctx->rctx
			, 0
			, (v->uv.x * tile[0].shiftS_f) + tile[0].ulS
			, (v->uv.y * tile[0].shiftT_f) + tile[0].ulT
		);
		
		ia_texcoord(
			ctx->rctx
			, 1
			, (v->uv.x * tile[1].shiftS_f) + tile[1].ulS
			, (v->uv.y * tile[1].shiftT_f) + tile[1].ulT
		);
		
		ia_vertex(ctx->rctx, v->pos.x, v->pos.y, v->pos.z, 1.0f);
	}

	ia_end(ctx->rctx);
}


static
inline
struct vec4
vec4_normalize3(struct vec4 *v)
{
	float len = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
	return (struct vec4) {
		  .x = v->x / len
		, .y = v->y / len
		, .z = v->z / len
	};
}


static
inline
void
vec4_normalize3_inplace(struct vec4 *v)
{
	float len = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
	v->x /= len;
	v->y /= len;
	v->z /= len;
}


static
inline
void
vec4_normalize2_inplace(struct vec4 *v)
{
	float len = sqrt(v->x * v->x + v->y * v->y);
	v->x /= len;
	v->y /= len;
}


static
inline
float
vec4_dot3(struct vec4 *a, struct vec4 *b)
{
	return a->x * b->x + a->y * b->y + a->z * b->z;
}


static
inline
struct vec4
reflect(struct vec4 *e, struct vec4 *n)
{
	/* return e - 2.0 * vec4_dot3(n, e) * n; */
	/* r = n * dot(n, e) * 2 */
	struct vec4 r = *n;
	float dot_result = vec4_dot3(n, e) * 2;
	r.x *= dot_result;
	r.y *= dot_result;
	r.z *= dot_result;
	/* r = e - r */
	r.x = e->x - r.x;
	r.y = e->y - r.y;
	r.z = e->z - r.z;
	return r;
}


static
void
shader_unlit_vertex(
	const struct swr_shaderProgram *prog
	, const struct swr_context *ctx
	, rs_vertex *v
)
{
	vec4 V = vec4_transform(ctx->modelview, v->attribs[ATTRIB_POS]);
	(void)prog;

	v->attribs[ATTRIB_POS] = vec4_transform(ctx->projection, V);
	v->used = ATTRIB_FLAG_POS | ATTRIB_FLAG_COLOR | ATTRIB_FLAG_TEX0 | ATTRIB_FLAG_TEX1;
	//v->used &= ~(ATTRIB_FLAG_NORMAL|ATTRIB_FLAG_USR0|ATTRIB_FLAG_USR1);
	
	/* shadeless */
	g_n64ctx->vertcolor = (struct vec4){1,1,1,1};
	//g_n64ctx->vertcolor = v->attribs[ATTRIB_COLOR];
}

static
struct vec4
shader_unlit_fragment(
	const struct swr_shaderProgram *prog
	, const struct swr_context *ctx
	, const rs_vertex *frag
)
{
	(void)prog;
	return (struct vec4){1,1,1,1};
}

static struct swr_shaderProgram quick_shader =
	{ shader_unlit_vertex, shader_unlit_fragment }
;

static
void
func_setcombine(
	struct n64ctx *ctx
	, unsigned char *b
)
{
	struct setcombineBitfield *a = &ctx->combiner.bitfield;
	uint32_t hi = /*(b[0]<<24)|*/(b[1]<<16)|(b[2]<<8)|b[3];
	uint32_t lo = (b[4]<<24)|(b[5]<<16)|(b[6]<<8)|b[7];
	
	/* FC opcode */
	a->op = 0xFC;
	
	/* cycle 0 color */
	a->a0 = (hi >> 20) & 0x0F; /* a */
	a->b0 = (lo >> 28) & 0x0F; /* b */
	a->c0 = (hi >> 15) & 0x1F; /* c */
	a->d0 = (lo >> 15) & 0x07; /* d */
	
	/* cycle 1 color */
	a->a1 = (hi >>  5) & 0x0F; /* e */
	a->b1 = (lo >> 24) & 0x0F; /* f */
	a->c1 = (hi >>  0) & 0x1F; /* g */
	a->d1 = (lo >>  6) & 0x07; /* h */
	
	/* cycle 0 alpha */
	a->Aa0 = (hi >> 12) & 0x07; /* z */
	a->Ab0 = (lo >> 12) & 0x07; /* y */
	a->Ac0 = (hi >>  9) & 0x07; /* x */
	a->Ad0 = (lo >>  9) & 0x07; /* w */
	
	/* cycle 1 color */
	a->Aa1 = (lo >> 21) & 0x07; /* v */
	a->Ab1 = (lo >>  3) & 0x07; /* u */
	a->Ac1 = (lo >> 18) & 0x07; /* t */
	a->Ad1 = (lo >>  0) & 0x07; /* s */
	
	/* now build oplist from combiner settings */
	n64ctx_generate_oplist(ctx);
	
	/* now make swrast use the fragment function */
	struct swr_context *swr = ctx->rctx;
	quick_shader.fragment = ctx->rast->func_frag;
	swr->shader = &quick_shader;
}


static
void
func_update_tile_sampler(
	struct n64ctx *ctx
	, struct n64tile *tile
)
{
	if (!tile->fmt && !tile->siz)
		return;
	int idx = tile - ctx->tile;
	struct n64rast *rast = ctx->rast;
	int sampler_idx = 
		(5 * 64) * ctx->use_point_filtering
		+ 64 * tile->fmt
		+ 16 * tile->siz
		+  4 * tile->cmS
		+ tile->cmT
	;
	rast->func_tile_sample[idx] = n64_sampler_array[sampler_idx];
#if 0
	fprintf(stderr, "_____\n");
	fprintf(stderr, "tile idx = %d\n", idx);
	fprintf(stderr, "tile->fmt = %d\n", tile->fmt);
	fprintf(stderr, "tile->siz = %d\n", tile->siz);
	fprintf(stderr, "using func %d (%s)\n", sampler_idx, n64_sampler_array_name(sampler_idx));
	
	fprintf(stderr, "are [%d] = %d x %d not right?\n", idx, tile->width, tile->height);
	/*for (int i=0; i<sizeof(n64_sampler_array)/sizeof(n64_sampler_array[0]); ++i)
		fprintf(stderr, "[%3d] = %p\n", i, n64_sampler_array[i]);
	fprintf(stderr, "%p...\n", n64_sampler_array[sampler_idx]);*/
#endif
}


static
void
func_loadblock(
	struct n64ctx *ctx
	, unsigned char *b
)
{
#if 0
	/* 0xF3 command: load block of texture data pointed to by *
	 *               timg into requested tile descriptor      */
	/* in swrast branch, we just select the appropriate sampling   *
	 * callback based on the format of the data we're working with */
	int tile_idx = (b[4] & 0xF); /* i */
	assert(tile_idx < ( sizeof(ctx->tile) / sizeof(ctx->tile[0])));
	struct n64tile *tile = ctx->tile + tile_idx;
	struct n64rast *rast = ctx->rast;
	unsigned long sampler_idx;
	fprintf(stderr, "tile->siz = %d\n", tile->siz);
	sampler_idx = 
		64 * tile->fmt
		+ 16 * tile->siz
		+  4 * tile->cmS
		+ tile->cmT
	;
	
	fprintf(stderr, "using func %ld\n", sampler_idx);
	fprintf(stderr, "tile idx = %d\n", tile_idx);
	/*for (int i=0; i<sizeof(n64_sampler_array)/sizeof(n64_sampler_array[0]); ++i)
		fprintf(stderr, "[%3d] = %p\n", i, n64_sampler_array[i]);
	fprintf(stderr, "%p...\n", n64_sampler_array[sampler_idx]);*/
	
	rast->func_tile_sample[tile_idx] = n64_sampler_array[sampler_idx];
#endif
}


static
void func_vertex(
	struct n64ctx *n64
	, struct n64vert *vert
)
{
	/* TODO texgen uses hard-coded texture sizes */
	struct swr_context *ctx = n64->rctx;
	if (n64->geometrymode.texgen)
	{
		/* GL_EYE_LINEAR */
	#if 0
		/* try doing some texgen stuff */
		vec4 V = vec4_transform(ctx->modelview, vert->pos);
		struct vec4 e = vec4_normalize3(&V);
		struct vec4 n = vec4_transform(ctx->projection, vert->col);
		vec4_normalize3_inplace(&n);
		
		struct vec4 r = reflect(&e, &n);
		float m = 2 * sqrt(r.x*r.x + r.y*r.y + (r.z+1)*(r.z+1));
		vert->uv.x = r.x / m + 0.5f;
		vert->uv.y = r.y / m + 0.5f;
		vert->uv.x *= 16;
		vert->uv.y *= 16;
	#endif
		struct vec4 n = vec4_transform(ctx->normalmatrix, vert->norm);
		vec4_normalize3_inplace(&n);
		vert->uv.x = (n.x + 1) / 2;
		vert->uv.y = (n.y + 1) / 2;
	#if 0 /* linear? */
		//vec4_normalize2_inplace(&n);
		vert->uv.x = acos(n.x);
		vert->uv.y = acos(n.y);
	#endif
		/* TODO 8 or 16? likely varies with texture dimensions... */
		vert->uv.x *= 16;
		vert->uv.y *= 16;
	}
}


static
void
func_geometrymode(struct n64ctx *ctx)
{
	struct swr_context *swr = ctx->rctx;
	
	/* clear culling bits */
	swr->flags &= ~(CULL_FRONT | CULL_BACK);
	
	/* set culling bits */
	if (ctx->geometrymode.cull_front)
		swr->flags |= CULL_FRONT;
	if (ctx->geometrymode.cull_back)
		swr->flags |= CULL_BACK;
}


static
void
func_othermode(struct n64ctx *ctx)
{
	blend_enable_test(ctx);
}


void
n64rast_use_n64ctx_swrast(struct n64ctx *ctx)
{
	g_n64ctx = ctx;
}


static
void
func_vbuf(
	struct n64ctx *n64
	, int numv
	, int vbidx
)
{
}

struct n64ctx *
n64ctx_new_swrast(void *ctx)
{
	struct n64ctx *nc;
	
	nc = calloc(1, sizeof(*nc));
	
	if (!nc)
		exit(EXIT_FAILURE);
	
	nc->rast = n64rast_new(ctx);
	nc->rctx = ctx;
	
	nc->func_triangle = func_triangle;
	nc->func_setcombine = func_setcombine;
	nc->func_loadblock = func_loadblock;
	nc->func_update_tile_sampler = func_update_tile_sampler;
	nc->func_vertex = func_vertex;
	nc->func_vbuf = func_vbuf;
	nc->func_geometrymode = func_geometrymode;
	nc->func_othermode = func_othermode;
	nc->func_walk = f3dzex_walk;
	
	/* allocate 4096 bytes for white opaque texture; *
	 * 4096 b/c this is the max the N64 will use     */
	nc->blank_texture = malloc(4096);
	if (!nc->blank_texture)
		exit(EXIT_FAILURE);
	memset(nc->blank_texture, 0xFF, 4096);
	
	return nc;
}
#endif /* ndef Z64MATGEN_BUILDING_LIST */


#ifdef Z64MATGEN_BUILDING_LIST
int
main(void)
{
	struct n64ctx m={0};
	m.rast = n64rast_new(0);
	/*m.combiner.word.hi = 0xFC267E04;
	m.combiner.word.lo = 0x1FFCFDF8;*/
	struct setcombineBitfield *b = &m.combiner.bitfield;
	
	/* FC opcode */
	b->op = 0xFC;
	
	/* cycle 0 color */
	b->a0 = G_CCMUX_TEXEL1;
	b->b0 = G_CCMUX_TEXEL0;
	b->c0 = G_CCMUX_ENV_ALPHA;
	b->d0 = G_CCMUX_TEXEL0;
	
	/* cycle 1 color */
	b->a1 = G_CCMUX_COMBINED;
	b->b1 = G_CCMUX_0_INITIALIZER;
	b->c1 = G_CCMUX_SHADE;
	b->d1 = G_CCMUX_0_INITIALIZER;
	
	/* cycle 0 alpha */
	b->Aa0 = G_ACMUX_0;
	b->Ab0 = G_ACMUX_0;
	b->Ac0 = G_ACMUX_0;
	b->Ad0 = G_ACMUX_1;
	
	/* cycle 1 color */
	b->Aa1 = G_ACMUX_0;
	b->Ab1 = G_ACMUX_0;
	b->Ac1 = G_ACMUX_0;
	b->Ad1 = G_ACMUX_COMBINED;
	
	//fprintf(stderr, "%08X %08X\n", m.combiner.word.hi, m.combiner.word.lo);
	//return 0;
	
	//n64ctxgen(&m);
	n64ctx_generate_oplist(&m);
	
	struct swr_context ctx = { .n64ctx = &m };
	
#ifdef Z64MATGEN_BUILDING_LIST
	/* generate functions for operating on colors */
	for (struct setcombineVariable *v = setcombineColorVariable
			; v - setcombineColorVariable < (sizeof(setcombineColorVariable) / sizeof(*v))
			; ++v) {
		declare_step_func(v, 1, "+=", "add");
		declare_step_func(v, 1, "-=", "sub");
		declare_step_func(v, 1, "*=", "mul");
		declare_step_func(v, 1, "=", "set");
	}
	
	/* generate functions for operating on alpha values */
	for (struct setcombineVariable *v = setcombineAlphaVariable
			; v - setcombineAlphaVariable < (sizeof(setcombineAlphaVariable) / sizeof(*v))
			; ++v) {
		declare_step_func(v, 0, "+=", "add");
		declare_step_func(v, 0, "-=", "sub");
		declare_step_func(v, 0, "*=", "mul");
		declare_step_func(v, 0, "=", "set");
	}
	/* generate functions for operating on colors */
	int numColorVariable = (sizeof(setcombineColorVariable) / sizeof(setcombineColorVariable[0]));
	
	int numAlphaVariable = (sizeof(setcombineAlphaVariable) / sizeof(setcombineAlphaVariable[0]));
	
	char *allOps[]={"add", "sub", "mul", "set"};
	
	for (int i=0; i < 4; ++i)
		declare_step_func_list(
			setcombineColorVariable
			, numColorVariable
			, 1
			, allOps[i]
		);
	
	for (int i=0; i < 4; ++i)
		declare_step_func_list(
			setcombineAlphaVariable
			, numAlphaVariable
			, 0
			, allOps[i]
		);

#else
	
	z64frag_notex(0, &ctx, 0);
#endif
	return 0;
}
#endif /* Z64MATGEN_BUILDING_LIST */


void
n64rast_box(struct n64ctx *ctx, struct vec4 *vfirst, struct vec4 *vlast)
{
	struct vec4 *arr[] = {vfirst, vlast};
	const int lut[8][3] = {
		{0, 0, 0}
		, {0, 1, 0}
		, {1, 1, 0}
		, {1, 0, 0}
		, {1, 0, 1}
		, {0, 0, 1}
		, {0, 1, 1}
		, {1, 1, 1}
	};
	const int tri[12][3] = {
		{0, 1, 2}, {2, 3, 0}    /* back   */
		, {5, 7, 6}, {7, 5, 4}  /* front  */
		, {0, 6, 1}, {6, 0, 5}  /* left   */
		, {3, 2, 7}, {7, 4, 3}  /* right  */
		, {6, 2, 1}, {2, 6, 7}  /* top    */
		, {5, 0, 3}, {3, 4, 5}  /* bottom */
	};
	//int t = 0;

	struct swr_context *swr = ctx->rctx;
	swr->shader = shader_internal(SHADER_UNLIT);
	swr->texture_enable[0] = 0;
	swr->textures[0] = NULL;
	swr->flags |= BLEND_ENABLE;
	
	/* clear culling bits */
	swr->flags &= ~(CULL_FRONT | CULL_BACK);

	ia_begin(swr);
	for (int i=0; i<12; ++i)
	{
		ia_color(swr, 1.0f, 1.0f, 0.0f, 0.5f);
		for (int k=0; k<3; ++k)
		{
			struct vec4 vert = {
				.x = arr[lut[tri[i][k]][0]]->x
				, .y = arr[lut[tri[i][k]][1]]->y
				, .z = arr[lut[tri[i][k]][2]]->z
			};
			//fprintf(stdout, "v %f %f %f\n", vert.x, vert.y, vert.z);
			ia_vertex(swr, vert.x, vert.y, vert.z, 1.0f);
		}
		//fprintf(stdout, "f %d %d %d\n", t+1, t+2, t+3);
		//t += 3;
	}
	ia_end(swr);
}

