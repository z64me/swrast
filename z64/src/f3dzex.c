#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "f3dzex.h"

//float oh_dear[16];

/* F3DZEX */
#define HC_ENDDL 0xDF
#define HC_DL    0xDE

/* F3DEX */
//#define HC_ENDDL 0xB8
//#define HC_DL    0x06

#define F3DZEXOP(OPNAME) f3dzex_##OPNAME

#define F3DEXOP(OPNAME) f3dex_##OPNAME

#define N64_IM_FMT_RGBA 0
#define N64_IM_FMT_YUV  1
#define N64_IM_FMT_CI   2
#define N64_IM_FMT_IA   3
#define N64_IM_FMT_I    4

#define N64_IM_SIZ_4b   0
#define N64_IM_SIZ_8b   1
#define N64_IM_SIZ_16b  2
#define N64_IM_SIZ_32b  3

#define DIV_1_4   0.25f
#define DIV_100   0.01f
#define DIV_1000  0.001f
#define DIV_1_255 0.003921569f
#define DIV_1_32  0.03125f
#define DIV_1_128 0.0078125f

#define    FTOFIX32(x)    (long)((x) * (float)0x00010000)
#define    FIX32TOF(x)    ((float)(x) * (1.0f / (float)0x00010000))

#define N64_RSP_TEXTURE_GEN        0b01000000000000000000
#define N64_RSP_TEXTURE_GEN_LINEAR 0b10000000000000000000


static
inline
void
display_vec4(struct vec4 *v)
{
	fprintf(stderr, "\t""%f %f %f %f\n", v->x, v->y, v->z, v->w);
}

static
inline
void
vec4_normalize3_inplace(struct vec4 *v)
{
	float len = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
	v->x = v->x / len;
	v->y = v->y / len;
	v->z = v->z / len;
}


static
inline
struct mat44f
mat44f_identity(void)
{
	struct mat44f mat44 = {0};
	
	mat44.x.x = 1;
	mat44.y.y = 1;
	mat44.z.z = 1;
	mat44.w.w = 1;
	
	return mat44;
}


static
inline
struct mat44f
mat44f_mul(struct mat44f *a, struct mat44f *b)
{
    struct mat44f result;
#define m0   x.x
#define m1   y.x
#define m2   z.x
#define m3   w.x

#define m4   x.y
#define m5   y.y
#define m6   z.y
#define m7   w.y

#define m8   x.z
#define m9   y.z
#define m10  z.z
#define m11  w.z

#define m12  x.w
#define m13  y.w
#define m14  z.w
#define m15  w.w
    result.m0 = a->m0*b->m0 + a->m1*b->m4 + a->m2*b->m8 + a->m3*b->m12;
    result.m1 = a->m0*b->m1 + a->m1*b->m5 + a->m2*b->m9 + a->m3*b->m13;
    result.m2 = a->m0*b->m2 + a->m1*b->m6 + a->m2*b->m10 + a->m3*b->m14;
    result.m3 = a->m0*b->m3 + a->m1*b->m7 + a->m2*b->m11 + a->m3*b->m15;
    result.m4 = a->m4*b->m0 + a->m5*b->m4 + a->m6*b->m8 + a->m7*b->m12;
    result.m5 = a->m4*b->m1 + a->m5*b->m5 + a->m6*b->m9 + a->m7*b->m13;
    result.m6 = a->m4*b->m2 + a->m5*b->m6 + a->m6*b->m10 + a->m7*b->m14;
    result.m7 = a->m4*b->m3 + a->m5*b->m7 + a->m6*b->m11 + a->m7*b->m15;
    result.m8 = a->m8*b->m0 + a->m9*b->m4 + a->m10*b->m8 + a->m11*b->m12;
    result.m9 = a->m8*b->m1 + a->m9*b->m5 + a->m10*b->m9 + a->m11*b->m13;
    result.m10 = a->m8*b->m2 + a->m9*b->m6 + a->m10*b->m10 + a->m11*b->m14;
    result.m11 = a->m8*b->m3 + a->m9*b->m7 + a->m10*b->m11 + a->m11*b->m15;
    result.m12 = a->m12*b->m0 + a->m13*b->m4 + a->m14*b->m8 + a->m15*b->m12;
    result.m13 = a->m12*b->m1 + a->m13*b->m5 + a->m14*b->m9 + a->m15*b->m13;
    result.m14 = a->m12*b->m2 + a->m13*b->m6 + a->m14*b->m10 + a->m15*b->m14;
    result.m15 = a->m12*b->m3 + a->m13*b->m7 + a->m14*b->m11 + a->m15*b->m15;

    return result;
}

static
inline
struct vec4
vec4_mul_mat44f(struct vec4 *v, struct mat44f *mat)
{
	/* TODO can substitute 1 for v->w, but only when safe to assume v->w is 1
	 * (make a separate function for this sometime) */
	return (struct vec4) {
		  .x = v->x * mat->x.x + v->y * mat->y.x + v->z * mat->z.x + 1 * mat->w.x
		, .y = v->x * mat->x.y + v->y * mat->y.y + v->z * mat->z.y + 1 * mat->w.y
		, .z = v->x * mat->x.z + v->y * mat->y.z + v->z * mat->z.z + 1 * mat->w.z
		, .w = v->x * mat->x.w + v->y * mat->y.w + v->z * mat->z.w + 1 * mat->w.w
	};
}

static
inline
struct vec4
vec4_mul_mat44f_upper33(struct vec4 *v, struct mat44f *mat)
{
	/* TODO can substitute 1 for v->w, but only when safe to assume v->w is 1
	 * (make a separate function for this sometime) */
	return (struct vec4) {
		  .x = v->x * mat->x.x + v->y * mat->y.x + v->z * mat->z.x
		, .y = v->x * mat->x.y + v->y * mat->y.y + v->z * mat->z.y
		, .z = v->x * mat->x.z + v->y * mat->y.z + v->z * mat->z.z
		, .w = v->x * mat->x.w + v->y * mat->y.w + v->z * mat->z.w
	};
}

static
inline
struct mat44f
mat44f_from_mat44s32(struct mat44s32 *m)
{
#define F FIX32TOF
	struct mat44f mat44 = {
		  { F(m->v[0][0]), F(m->v[0][1]), F(m->v[0][2]), F(m->v[0][3]) }
		, { F(m->v[1][0]), F(m->v[1][1]), F(m->v[1][2]), F(m->v[1][3]) }
		, { F(m->v[2][0]), F(m->v[2][1]), F(m->v[2][2]), F(m->v[2][3]) }
		, { F(m->v[3][0]), F(m->v[3][1]), F(m->v[3][2]), F(m->v[3][3]) }
	};
	
	return mat44;
#undef F
}

static
inline
unsigned int
getw24(unsigned char *b)
{
	return (b[0] << 16) | (b[1] << 8) | b[2];
}


static
inline
unsigned int
getw32(unsigned char *b)
{
	return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}


static
inline
unsigned short
getu16(unsigned char *b)
{
	return (unsigned short)(b[0]<<8) | b[1];
}


static
inline
signed short
gets16(unsigned char *b)
{
	return (signed short)((b[0] << 8) | b[1]);
}


static
inline
signed short
to_s8(unsigned char b)
{
	return (signed char)b;
}


static
inline
float
from_10_5_b(unsigned char *b)
{
	unsigned short raw = (b[0]<<8)|b[1];
	float ten  = (raw >> 5) & 0x3FF;
	float five = raw & 0x1F;
	ten += five * DIV_1_32 * 0.00001;
	if (raw & 0x8000)
		ten *= -1;
	return ten;
}


static
inline
struct mat44s32
mat44s32_from_n64(unsigned char *raw)
{
	struct mat44s32 mat;
	unsigned char *whole = raw;
	unsigned char *frac  = raw + 0x20;
	for (int i=0; i < 4; ++i)
	{
		for (int k=0; k < 4; ++k)
		{
			mat.v[i][k] = (gets16(whole) << 16) | getu16(frac);
			whole += 2;
			frac  += 2;
		}
	}
	return mat;
}


static
inline
void
handle_vertex_color(
	struct n64ctx *ctx
	, struct n64vert *dst
	, unsigned char *rgba
)
{
	struct vec4 *col = &dst->col;
	struct vec4 *norm = &dst->norm;
	*norm = *col = (struct vec4){1,1,1,1};
	col->w = rgba[3] * DIV_1_255;
	
	if (!ctx->use_shading)
		return;
	
	if (ctx->use_vertex_normals)
	{
		norm->x = to_s8(rgba[0]) * DIV_1_128;
		norm->y = to_s8(rgba[1]) * DIV_1_128;
		norm->z = to_s8(rgba[2]) * DIV_1_128;
		/*struct vec3 normal = {
			norm->x
			, norm->y
			, norm->z
		};*/
	
		/* TODO perform lighting calculations here *
		 *      (bake as vertex colors)            */
	}
	else if (ctx->use_vertex_colors)
	{
		col->x = rgba[0] * DIV_1_255;
		col->y = rgba[1] * DIV_1_255;
		col->z = rgba[2] * DIV_1_255;
	}
}


static
inline
unsigned char *
ptr(F3DZEXOPARGS)
{
	int segbyte = b[0];
	unsigned char *seg;
	
	/* if ram pointer, use segment[0] */
	if (segbyte >= 0x80)
	{
		seg = ctx->segment[0];
		
		if (!seg)
			return 0;
		
		unsigned long int rel = (getw32(b) - ctx->segment80);
		
		/* offset exceeds 0xffffff */
		if (rel >> 24)
			return 0;
		
		return seg + rel;
	}
	
	seg = ctx->segment[segbyte];
	
	if (seg)
		return seg + getw24(b+1);
	
	return 0;
}


unsigned char *
f3dzex_ptr(F3DZEXOPARGS)
{
	return ptr(ctx, b);
}

static
void
F3DZEXOP(NOOP)(F3DZEXOPARGS)
{
}


/* F3DEX variant */
static
void
F3DEXOP(VTX)(F3DZEXOPARGS)
{
	/* 0x01 command: fill vertex buffer */
	unsigned char *vdata = ptr(ctx, b+4);
	if (!vdata)
		return;
	
	//vdata += 0x1A20;
	
	struct n64vert *dst = ctx->vbuf;
#if 0
	uint16_t nl = getu16(b+2);
	int n = nl >> 10;
	int l = nl & 0x3FF;
	int numv = (l+1) / 0x10;
#endif
	int numv = b[2] >> 2;
	int vbidx = b[1];
	
	//fprintf(stderr, "%d verts at [%d]\n", numv, vbidx);
	
	/* advance destination to index */
	dst += vbidx;
	
	for(int i=0; i < numv; ++i)
	{
		dst->pos.x = gets16(vdata);
		dst->pos.y = gets16(vdata+2);
		dst->pos.z = gets16(vdata+4);
		
		/* TODO banjo kazooie fix; fixes 64x64 textures, but does it work for other dimensions? */
		dst->uv.x  = gets16(vdata+8) * DIV_1_32 * 0.5f;//from_10_5_b(vdata+8);
		dst->uv.y  = gets16(vdata+10) * DIV_1_32 * 0.5f;//from_10_5_b(vdata+10);
		handle_vertex_color(ctx, dst, vdata+12);
		
		/* transform vertex position if matrix is active */
		if (ctx->matrix.mat)
		{
			/*struct mat44f mat44f = ctx->matrix.mat;
			fprintf(stderr, "matrix:\n");
			display_vec4(&mat44f.x);
			display_vec4(&mat44f.y);
			display_vec4(&mat44f.z);
			display_vec4(&mat44f.w);
			exit(0);*/
			dst->pos = vec4_mul_mat44f(&dst->pos, ctx->matrix.mat);
			
			/*if (ctx->matrix.stack_level == 2)
			{
				memcpy(&ctx->matrix.mat, oh_dear, sizeof(oh_dear));
				dst->pos = vec4_mul_mat44f(&dst->pos, &ctx->matrix.mat);
			}*/
		}
		#if 1 // TODO hardcoded scale for swrast testing
		dst->pos.x *= DIV_100;
		dst->pos.y *= DIV_100;
		dst->pos.z *= DIV_100;
		#endif
		#if 0 // TODO hardcoded scale for swrast testing Banjo
		dst->pos.x *= DIV_1000;
		dst->pos.y *= DIV_1000;
		dst->pos.z *= DIV_1000;
		#endif
		
		ctx->func_vertex(ctx, dst);
		
		++dst;
		vdata += 16;
	}
	
	ctx->func_vbuf(ctx, numv, vbidx);
}


static
void
F3DZEXOP(VTX)(F3DZEXOPARGS)
{
	/* 0x01 command: fill vertex buffer */
	unsigned char *vdata = ptr(ctx, b+4);
	if (!vdata)
		return;
	
	struct n64vert *dst = ctx->vbuf;
	int numv = (b[1]<<4)|(b[2]>>4);
	int vbidx = b[3] >> 1;
	
/*da38 0003 0d00 0040
 *0100 500a 0603 15e0
 *da38 0003 0d00 0080
 *0100 f028 0603 1630*/
	
	/* advance destination to index */
	vbidx -= numv;
	assert(vbidx >= 0);
	dst += vbidx;
	
	for(int i=0; i < numv; ++i)
	{
		dst->pos.x = gets16(vdata);
		dst->pos.y = gets16(vdata+2);
		dst->pos.z = gets16(vdata+4);
		
		dst->uv.x  = gets16(vdata+8) * DIV_1_32;//from_10_5_b(vdata+8);
		dst->uv.y  = gets16(vdata+10) * DIV_1_32;//from_10_5_b(vdata+10);
		handle_vertex_color(ctx, dst, vdata+12);
		
		/* transform vertex position if matrix is active */
		if (ctx->matrix.mat)
		{
			/*struct mat44f mat44f = ctx->matrix.mat;
			fprintf(stderr, "matrix:\n");
			display_vec4(&mat44f.x);
			display_vec4(&mat44f.y);
			display_vec4(&mat44f.z);
			display_vec4(&mat44f.w);
			exit(0);*/
			dst->pos = vec4_mul_mat44f(&dst->pos, ctx->matrix.mat);
			
			/*if (ctx->matrix.stack_level == 2)
			{
				memcpy(&ctx->matrix.mat, oh_dear, sizeof(oh_dear));
				dst->pos = vec4_mul_mat44f(&dst->pos, &ctx->matrix.mat);
			}*/
		}
		#if 1 // TODO hardcoded scale for swrast testing
		dst->pos.x *= DIV_100;
		dst->pos.y *= DIV_100;
		dst->pos.z *= DIV_100;
		#endif
		
		ctx->func_vertex(ctx, dst);
		
		++dst;
		vdata += 16;
	}
	
	ctx->func_vbuf(ctx, numv, vbidx);
}


static
void
F3DZEXOP(MODIFYVTX)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(CULLDL)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(BRANCH_Z)(F3DZEXOPARGS)
{
	/* 0x04 command: branch to other display list */
	unsigned char rdp[4] = {
		ctx->rdp.hi>>24
		, (ctx->rdp.hi>>16)&0xFF
		, (ctx->rdp.hi>> 8)&0xFF
		, (ctx->rdp.hi>> 0)&0xFF
	};
	
	if (ctx->branches_are_DLs)
	{
		unsigned char *call;
		call = ptr(ctx, rdp);
		
		if (call)
			ctx->func_walk(ctx, call);
	}
	else
		ctx->force_branch = ptr(ctx, rdp);
}


static
void
F3DZEXOP(TRI1)(F3DZEXOPARGS)
{
	/* 0x05 command: draw one triangle */
	ctx->func_triangle(ctx, b[1]>>1, b[2]>>1, b[3]>>1);
}


/* f3dex equivalent */
static
void
F3DEXOP(TRI1)(F3DZEXOPARGS)
{
	/* 0x05 command: draw one triangle */
	ctx->func_triangle(ctx, b[5]>>1, b[6]>>1, b[7]>>1);
}


static
void
F3DZEXOP(TRI2)(F3DZEXOPARGS)
{
	/* 0x06 command: draw two triangles */
	ctx->func_triangle(ctx, b[1]>>1, b[2]>>1, b[3]>>1);
	ctx->func_triangle(ctx, b[5]>>1, b[6]>>1, b[7]>>1);
}


static
void
F3DZEXOP(QUAD)(F3DZEXOPARGS)
{
	/* 0x07 command: draw a quad; functionally equivalent to TRI2 */
	ctx->func_triangle(ctx, b[1]>>1, b[2]>>1, b[3]>>1);
	ctx->func_triangle(ctx, b[5]>>1, b[6]>>1, b[7]>>1);
}


static
void
F3DZEXOP(LINE3D)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(SPECIAL_3)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(SPECIAL_2)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(SPECIAL_1)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(DMA_IO)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(TEXTURE)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(POPMTX)(F3DZEXOPARGS)
{
	/* 0xD8 command: pop matrix */
	int num = getw32(b+4) / 64;
	//fprintf(stderr, "pop %d matrices\n", num);
	//num = 2;
	if (num > ctx->matrix.stack_level)
	{
		fprintf(
			stderr
			, "[!] matrix stack underflow: popping %d off stack of size %d!\n"
			, num
			, ctx->matrix.stack_level
		);
		exit(EXIT_FAILURE);
	}
	if (ctx->matrix.stack_level == num)
		ctx->matrix.mat = 0;
	else
		ctx->matrix.mat -= num;
	ctx->matrix.stack_level -= num;
}

/* F3DEX specific */
static
void
F3DEXOP(CLEARGEOMETRYMODE)(F3DZEXOPARGS)
{
}

/* F3DEX specific */
static
void
F3DEXOP(SETGEOMETRYMODE)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(GEOMETRYMODE)(F3DZEXOPARGS)
{
	/* 0xD9 command: update RSP geometry mode */
	uint32_t clearbits = ~(getw32(b) & 0xFFFFFF);
	uint32_t setbits   = getw32(b+4);
	uint32_t cur = ctx->geometrymode.current;
	/* TODO cur = 0 isn't correct, but works for now... */
	cur = 0;
	cur = (cur & ~clearbits) | setbits;
	ctx->geometrymode.current = cur;
	
	if (clearbits & N64_G_SHADE)
	{
		ctx->use_shading = 0;
	}
	if (setbits & N64_G_SHADE)
	{
		ctx->use_shading = 1;
	}
	if (clearbits & N64_G_LIGHTING)
	{
		ctx->use_vertex_colors = 1;
		ctx->use_vertex_normals = 0;
	}
	if (setbits & N64_G_LIGHTING)
	{
		ctx->use_vertex_colors = 0;
		ctx->use_vertex_normals = 1;
	}
	
	ctx->geometrymode.texgen =
		(cur & (N64_RSP_TEXTURE_GEN | N64_RSP_TEXTURE_GEN_LINEAR))
		>> 18
	;
	
	ctx->geometrymode.cull_front = (cur >>  9) & 1;
	ctx->geometrymode.cull_back  = (cur >> 10) & 1;
	
	ctx->func_geometrymode(ctx);
	
	//fprintf(stderr, "texgen = %d\n", ctx->geometrymode.texgen);
}


static
void
F3DZEXOP(MTX)(F3DZEXOPARGS)
{
	/* 0xDA command: does a matrix operation */
	unsigned char *mp;
	mp = ptr(ctx, b + 4);
	if (!mp)
		return;

#define N64_G_MTX_NOPUSH      0x00
#define N64_G_MTX_PUSH        0x01
#define N64_G_MTX_MUL         0x00
#define N64_G_MTX_LOAD        0x02
#define N64_G_MTX_MODELVIEW   0x00
#define N64_G_MTX_PROJECTION  0x04
	
	int params = b[3] ^ N64_G_MTX_PUSH;
	struct mat44f *transforming = ctx->matrix.mat;
	
	/* push matrix on stack */
	if (params & N64_G_MTX_PUSH)
	{
		ctx->matrix.stack_level += 1;
		ctx->matrix.mat = ctx->matrix.stack + ctx->matrix.stack_level;
	
		if (ctx->matrix.stack_level >= N64CTX_MATRIX_STACK_MAX)
		{
			fprintf(stderr, "[!] matrix stack overflow: pushed too many matrices!\n");
			exit(EXIT_FAILURE);
		}
	}
	else if (!ctx->matrix.mat || !ctx->matrix.stack)
	{
		ctx->matrix.mat = ctx->matrix.stack+1;
		*ctx->matrix.mat = mat44f_identity();
	}
	
	struct mat44s32 mat44s32 = mat44s32_from_n64(mp);
	struct mat44f mat44f = mat44f_from_mat44s32(&mat44s32);
	
	if (transforming)
	{
		*ctx->matrix.mat = mat44f_mul(transforming, &mat44f);
	}
	else
		*ctx->matrix.mat = mat44f;
}


static
void
F3DZEXOP(MOVEWORD)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(MOVEMEM)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(LOAD_UCODE)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(DL)(F3DZEXOPARGS)
{
	/* 0xDE command: display another display list */
	unsigned char *call;
	call = ptr(ctx, b + 4);
	
	if (call)
		ctx->func_walk(ctx, call);
}


static
void
F3DZEXOP(ENDDL)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(SPNOOP)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(RDPHALF_1)(F3DZEXOPARGS)
{
	/* 0xE1 command: sets higher half of generic RDP word */
	ctx->rdp.hi = getw32(b + 4);
}


static
void
F3DZEXOP(SETOTHERMODE_L)(F3DZEXOPARGS)
{
	/* 0xE2 command: set other mode, lower word */
	int length = 1 + b[3];
	int shift = 32 - length - b[2];
	uint32_t data = getw32(b+4);
	ctx->othermode.lo = ctx->othermode.lo & ~(((1 << length) - 1) << shift) | data;
	ctx->othermode.alphacompare = ctx->othermode.lo & 3;
	ctx->othermode.indep.v = (ctx->othermode.lo>>3) & 0x1FFF;
	ctx->othermode.indep.cvg_x_alpha = (ctx->othermode.indep.v>>9)&1;
	ctx->othermode.indep.alpha_cvg_sel = (ctx->othermode.indep.v>>10)&1;
	ctx->othermode.indep.force_bl = (ctx->othermode.indep.v>>11)&1;
	ctx->othermode.indep.zmode = (ctx->othermode.indep.v>>7)&3;
	if (ctx->othermode.indep.zmode == N64_ZMODE_INTER)
		ctx->othermode.indep.zmode = N64_ZMODE_OPA;
	//fprintf(stderr, "zmode = %d\n", ctx->othermode.indep.zmode);
	//fprintf(stderr, "%08X\n", (ctx->othermode.indep.v>>11)&1);
	ctx->func_othermode(ctx);
}


static
void
F3DZEXOP(SETOTHERMODE_H)(F3DZEXOPARGS)
{
	/* 0xE3 command: set other mode, upper word */
	int length = 1 + b[3];
	int shift = 32 - length - b[2];
	uint32_t data = getw32(b+4);
	ctx->othermode.hi = ctx->othermode.hi & ~(((1 << length) - 1) << shift) | data;
	ctx->func_othermode(ctx);
}


static
void
F3DZEXOP(TEXRECT)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(TEXRECTFLIP)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(RDPLOADSYNC)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(RDPPIPESYNC)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(RDPTILESYNC)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(RDPFULLSYNC)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(SETKEYGB)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(SETKEYR)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(SETCONVERT)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(SETSCISSOR)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(SETPRIMDEPTH)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(RDPSETOTHERMODE)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(LOADTLUT)(F3DZEXOPARGS)
{
	/* 0xF0 command: load palette */
	/* TODO this hack works, but is it actually a good way to emulate? */
	ctx->palette = ctx->timg.imgaddr;
}


static
void
F3DZEXOP(RDPHALF_2)(F3DZEXOPARGS)
{
	/* 0xF1 command: sets lower half of generic RDP word */
	ctx->rdp.lo = getw32(b + 4);
}


static
void
F3DZEXOP(SETTILESIZE)(F3DZEXOPARGS)
{
	/* 0xF2 command: set size and u/v offset of tile */
	int idx = b[4] & 0xF; /* i */
	struct n64tile *tile = ctx->tile + idx;
	uint32_t hi = getw32(b) & 0xFFFFFF;
	uint32_t lo = getw32(b+4);
	
	/* unsigned fixed-point 10.2 format range 0 <= x <= 1023.75 */
	/* aka divide the number by (1 << 2) */
#if 0
	tile->ulS = DIV_1_4 * ( (hi >> 12) & 0xFFF ); /* s */
	tile->ulT = DIV_1_4 * ( (hi >>  0) & 0xFFF ); /* t */
	tile->lrS = DIV_1_4 * ( (lo >> 12) & 0xFFF ); /* u */
	tile->lrT = DIV_1_4 * ( (lo >>  0) & 0xFFF ); /* v */
#else
	tile->ulS = ( (hi >> 12) & 0xFFF ); /* s */
	tile->ulT = ( (hi >>  0) & 0xFFF ); /* t */
	tile->lrS = ( (lo >> 12) & 0xFFF ); /* u */
	tile->lrT = ( (lo >>  0) & 0xFFF ); /* v */
#endif
	
	/* this is hackish stuff for determinig dimensions */
	int width  = ((tile->lrS - tile->ulS) >> 2) + 1;//( ( lo >> 12) & 0xFFF ) / 4 + 1;
	int height = ((tile->lrT - tile->ulT) >> 2) + 1;//( ( lo >>  0) & 0xFFF ) / 4 + 1;
	
	/* use only the first F2 after the F5 for dimensions */
	if (tile->f2 == 0)
	{
		tile->width = width;
		tile->height = height;
	}
	
	/* aka `range_clamp`; the texture is allowed to repeat
	 * until it reaches this value, at which point it is
	 * clamped to the width/height set above */
	tile->clamp_w = width;
	tile->clamp_h = height;
	
	tile->f2 += 1;
	
	//fprintf(stderr, "are %d x %d not right?\n", tile->width, tile->height);
	//fprintf(stderr, "tile[%d] : %d x %d\n", idx, tile->width, tile->height);
}


static
void
F3DZEXOP(LOADBLOCK)(F3DZEXOPARGS)
{
	/* 0xF3 command: load block of texture data pointed to by *
	 *               timg into requested tile descriptor      */
	/* TODO not completely understood; in fact, it makes huge *
	 *      assumptions and just converts the current texture *
	 *      to rgba8888 format for use in the rasterizer      */
#if 0
	struct n64tile *tile = ctx->tile + (b[4] & 0xF); /* i */
	
	if (!ctx->timg.imgaddr)
	{
		tile->sampler = 
		return;
	}
#else
	struct n64tile *tile = ctx->tile + (b[4] & 0xF); /* i */
	tile->pix = ctx->timg.imgaddr;
	
	ctx->func_loadblock(ctx, b);
#endif
}


/* F3DEX version */
static
void
F3DEXOP(LOADBLOCK)(F3DZEXOPARGS)
{
	/* 0xF3 command: load block of texture data pointed to by *
	 *               timg into requested tile descriptor      */
	/* TODO not completely understood; in fact, it makes huge *
	 *      assumptions and just converts the current texture *
	 *      to rgba8888 format for use in the rasterizer      */
#if 0
	struct n64tile *tile = ctx->tile + (b[4] & 0xF); /* i */
	
	if (!ctx->timg.imgaddr)
	{
		tile->sampler = 
		return;
	}
#else
	struct n64tile *tile = ctx->tile + (b[4] & 0xF); /* i */
	
	/* TODO f3dex version operates on tile 0 directly; fixes
	 * issues with palette being updated but not texture; is
	 * this safe? */
	//tile = ctx->tile;
	tile->pix = ctx->timg.imgaddr;
	//*ctx->tile = *tile;
	
	ctx->func_loadblock(ctx, b);
#endif
}


static
void
F3DZEXOP(LOADTILE)(F3DZEXOPARGS)
{
	/* 0xF4 command: load rectangular portion of timg into tile */
	/* TODO not yet implemented, and not completely understood; *
	 *      is this for cropping a texture and then loading it? */
}


static
float
shift_to_multiplier(const int shift)
{
	/* how many bits to shift texture coordinates       *
	 * if in range  1 <= n <= 10, texcoord >>= n        *
	 * if in range 11 <= n <= 15, texcoord <<= (16 - n) */
	if (!shift)
		return 1;
	
	/* right shift; division by 2 per bit */
	if (shift < 11)
	{
		return 1.0f / pow(2, shift);
	}
	
	/* left shift; multiplication by 2 per bit */
	return pow(2, 16 - shift);
}


static
void
F3DZEXOP(SETTILE)(F3DZEXOPARGS)
{
	/* 0xF5 command: set many tile descriptor properties */
	int idx = b[4] & 7;
	struct n64tile *tile = ctx->tile + idx;
	uint32_t hi = getw24(b+1);
	uint32_t lo = getw32(b+4);
	
	if (idx < 7)
	{
		struct n64tile *load = ctx->tile + 7;
		tile->pix = load->pix;
	}
	
	/* lo */
	tile->shiftS  = (lo >>  0) & 0xF;   /* u */
	tile->shiftT  = (lo >> 10) & 0xF;   /* s */
	
	tile->maskS   = (lo >>  4) & 0xF;   /* b */
	tile->maskT   = (lo >> 14) & 0xF;   /* a */
	
	tile->cmS     = (lo >>  8) & 0x3;   /* d */
	tile->cmT     = (lo >> 18) & 0x3;   /* c */
	
	tile->palette = (lo >> 20) & 0xF;   /* p */
	
	/* hi */
	tile->line    = (hi >>  9) & 0x3FF; /* n */
	tile->tmem    = (hi >>  0) & 0x1FF; /* m */
	tile->siz     = (hi >> 19) & 0x3;   /* i */
	tile->fmt     = (hi >> 21) & 0x7;   /* f */
	
	/* custom stuff */
	/*if (tile->siz == 0)
		tile->pitch = tile->width / 2;
	else if (tile->siz == 3)
		tile->pitch = tile->width * 4;
	else
		tile->pitch = tile->width * tile->siz;*/
	tile->pitch = tile->line * 8;
	
	/* rgba32/rgba8888 hack TODO does it work on everything? */
	if (tile->fmt == N64_IM_FMT_RGBA && tile->siz == N64_IM_SIZ_32b)
		tile->pitch *= 2;
	
	tile->shiftS_f = shift_to_multiplier(tile->shiftS);
	tile->shiftT_f = shift_to_multiplier(tile->shiftT);
	
	/* reset F2 command counter to 0 on each F5 */
	tile->f2 = 0;
	
	ctx->func_update_tile_sampler(ctx, tile);
}


static
void
F3DZEXOP(FILLRECT)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(SETFILLCOLOR)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(SETFOGCOLOR)(F3DZEXOPARGS)
{
	/* 0xF8 command: set fog color/alpha */
	ctx->fogcolor.x = DIV_1_255 * b[4];
	ctx->fogcolor.y = DIV_1_255 * b[5];
	ctx->fogcolor.z = DIV_1_255 * b[6];
	ctx->fogcolor.w = DIV_1_255 * b[7];
}


static
void
F3DZEXOP(SETBLENDCOLOR)(F3DZEXOPARGS)
{
	/* 0xF9 command: set blend color/alpha */
	ctx->blendcolor.x = DIV_1_255 * b[4];
	ctx->blendcolor.y = DIV_1_255 * b[5];
	ctx->blendcolor.z = DIV_1_255 * b[6];
	ctx->blendcolor.w = DIV_1_255 * b[7];
}


static
void
F3DZEXOP(SETPRIMCOLOR)(F3DZEXOPARGS)
{
	/* 0xFA command: set primitive color/alpha, minlevel, and lodfrac */
	ctx->primcolor.x = DIV_1_255 * b[4];
	ctx->primcolor.y = DIV_1_255 * b[5];
	ctx->primcolor.z = DIV_1_255 * b[6];
	ctx->primcolor.w = DIV_1_255 * b[7];
	
	ctx->minlevel    = DIV_1_255 * b[2];
	ctx->lodfrac     = DIV_1_255 * b[3];
}


static
void
F3DZEXOP(SETENVCOLOR)(F3DZEXOPARGS)
{
	/* 0xFB command: set environment color/alpha */
	ctx->envcolor.x = DIV_1_255 * b[4];
	ctx->envcolor.y = DIV_1_255 * b[5];
	ctx->envcolor.z = DIV_1_255 * b[6];
	ctx->envcolor.w = DIV_1_255 * b[7];
}


static
void
F3DZEXOP(SETCOMBINE)(F3DZEXOPARGS)
{
	/* 0xFC command: set combiner settings */
	ctx->func_setcombine(ctx, b);
}


static
void
F3DZEXOP(SETTIMG)(F3DZEXOPARGS)
{
	/* 0xFD command: set timg (texture image) settings */
	struct n64timg *timg = &ctx->timg;
	uint32_t hi = getw32(b);
	
	timg->imgaddr = ptr(ctx, b + 4);
	timg->fmt     = (hi >> (16+5)) & 0x7;
	timg->siz     = (hi >> (16+3)) & 0x3;
	timg->width   = (hi & 0xFFF) + 1;
	
	/* guarantee it will at least point to blank data */
	if (!timg->imgaddr)
		timg->imgaddr = ctx->blank_texture;
}


static
void
F3DZEXOP(SETZIMG)(F3DZEXOPARGS)
{
}


static
void
F3DZEXOP(SETCIMG)(F3DZEXOPARGS)
{
}



static
void
(*microcode_op[256])(F3DZEXOPARGS);


static
void
(*f3dzex_op[256])(F3DZEXOPARGS) = {
	  [0x00] = F3DZEXOP(NOOP)
	, [0x01] = F3DZEXOP(VTX)
	, [0x02] = F3DZEXOP(MODIFYVTX)
	, [0x03] = F3DZEXOP(CULLDL)
	, [0x04] = F3DZEXOP(BRANCH_Z)
	, [0x05] = F3DZEXOP(TRI1)
	, [0x06] = F3DZEXOP(TRI2)
	, [0x07] = F3DZEXOP(QUAD)
	, [0x08] = F3DZEXOP(LINE3D)
	, [0xD3] = F3DZEXOP(SPECIAL_3)
	, [0xD4] = F3DZEXOP(SPECIAL_2)
	, [0xD5] = F3DZEXOP(SPECIAL_1)
	, [0xD6] = F3DZEXOP(DMA_IO)
	, [0xD7] = F3DZEXOP(TEXTURE)
	, [0xD8] = F3DZEXOP(POPMTX)
	, [0xD9] = F3DZEXOP(GEOMETRYMODE)
	, [0xDA] = F3DZEXOP(MTX)
	, [0xDB] = F3DZEXOP(MOVEWORD)
	, [0xDC] = F3DZEXOP(MOVEMEM)
	, [0xDD] = F3DZEXOP(LOAD_UCODE)
	, [0xDE] = F3DZEXOP(DL)
	, [0xDF] = F3DZEXOP(ENDDL)
	, [0xE0] = F3DZEXOP(SPNOOP)
	, [0xE1] = F3DZEXOP(RDPHALF_1)
	, [0xE2] = F3DZEXOP(SETOTHERMODE_L)
	, [0xE3] = F3DZEXOP(SETOTHERMODE_H)
	, [0xE4] = F3DZEXOP(TEXRECT)
	, [0xE5] = F3DZEXOP(TEXRECTFLIP)
	, [0xE6] = F3DZEXOP(RDPLOADSYNC)
	, [0xE7] = F3DZEXOP(RDPPIPESYNC)
	, [0xE8] = F3DZEXOP(RDPTILESYNC)
	, [0xE9] = F3DZEXOP(RDPFULLSYNC)
	, [0xEA] = F3DZEXOP(SETKEYGB)
	, [0xEB] = F3DZEXOP(SETKEYR)
	, [0xEC] = F3DZEXOP(SETCONVERT)
	, [0xED] = F3DZEXOP(SETSCISSOR)
	, [0xEE] = F3DZEXOP(SETPRIMDEPTH)
	, [0xEF] = F3DZEXOP(RDPSETOTHERMODE)
	, [0xF0] = F3DZEXOP(LOADTLUT)
	, [0xF1] = F3DZEXOP(RDPHALF_2)
	, [0xF2] = F3DZEXOP(SETTILESIZE)
	, [0xF3] = F3DZEXOP(LOADBLOCK)
	, [0xF4] = F3DZEXOP(LOADTILE)
	, [0xF5] = F3DZEXOP(SETTILE)
	, [0xF6] = F3DZEXOP(FILLRECT)
	, [0xF7] = F3DZEXOP(SETFILLCOLOR)
	, [0xF8] = F3DZEXOP(SETFOGCOLOR)
	, [0xF9] = F3DZEXOP(SETBLENDCOLOR)
	, [0xFA] = F3DZEXOP(SETPRIMCOLOR)
	, [0xFB] = F3DZEXOP(SETENVCOLOR)
	, [0xFC] = F3DZEXOP(SETCOMBINE)
	, [0xFD] = F3DZEXOP(SETTIMG)
	, [0xFE] = F3DZEXOP(SETZIMG)
	, [0xFF] = F3DZEXOP(SETCIMG)
};


static
void
(*f3dex_op[256])(F3DZEXOPARGS) = {
	  [0x00] = F3DZEXOP(NOOP)
	, [0x01] = F3DZEXOP(MTX)
	, [0x02] = F3DZEXOP(NOOP)
	, [0x03] = F3DZEXOP(MOVEMEM)
	, [0x04] = F3DEXOP(VTX)
	, [0x05] = F3DZEXOP(NOOP)
	, [0x06] = F3DZEXOP(DL)
	
	, [0xAF] = F3DZEXOP(LOAD_UCODE)
	, [0xB1] = F3DZEXOP(TRI2)
	, [0xB2] = F3DZEXOP(MODIFYVTX)
	, [0xB3] = F3DZEXOP(RDPHALF_2)
	, [0xB4] = F3DZEXOP(RDPHALF_1)
	, [0xB5] = F3DZEXOP(QUAD)
	
	, [0xB6] = F3DEXOP(CLEARGEOMETRYMODE)
	, [0xB7] = F3DEXOP(SETGEOMETRYMODE)
	
	, [0xB8] = F3DZEXOP(ENDDL)
	, [0xB9] = F3DZEXOP(SETOTHERMODE_L)
	, [0xBA] = F3DZEXOP(SETOTHERMODE_H)
	, [0xBB] = F3DZEXOP(TEXTURE)
	, [0xBC] = F3DZEXOP(MOVEWORD)
	, [0xBD] = F3DZEXOP(POPMTX)
	, [0xBE] = F3DZEXOP(CULLDL)
	, [0xBF] = F3DEXOP(TRI1)
	
	, [0xC0] = F3DZEXOP(NOOP)
	
	, [0xE4] = F3DZEXOP(TEXRECT)
	, [0xE5] = F3DZEXOP(TEXRECTFLIP)
	, [0xE6] = F3DZEXOP(RDPLOADSYNC)
	, [0xE7] = F3DZEXOP(RDPPIPESYNC)
	, [0xE8] = F3DZEXOP(RDPTILESYNC)
	, [0xE9] = F3DZEXOP(RDPFULLSYNC)
	, [0xEA] = F3DZEXOP(SETKEYGB)
	, [0xEB] = F3DZEXOP(SETKEYR)
	, [0xEC] = F3DZEXOP(SETCONVERT)
	, [0xED] = F3DZEXOP(SETSCISSOR)
	, [0xEE] = F3DZEXOP(SETPRIMDEPTH)
	, [0xEF] = F3DZEXOP(RDPSETOTHERMODE)
	
	, [0xF0] = F3DZEXOP(LOADTLUT)
	, [0xF2] = F3DZEXOP(SETTILESIZE)
	, [0xF3] = F3DEXOP(LOADBLOCK)
	, [0xF4] = F3DZEXOP(LOADTILE)
	, [0xF5] = F3DZEXOP(SETTILE)
	, [0xF6] = F3DZEXOP(FILLRECT)
	, [0xF7] = F3DZEXOP(SETFILLCOLOR)
	, [0xF8] = F3DZEXOP(SETFOGCOLOR)
	, [0xF9] = F3DZEXOP(SETBLENDCOLOR)
	, [0xFA] = F3DZEXOP(SETPRIMCOLOR)
	, [0xFB] = F3DZEXOP(SETENVCOLOR)
	, [0xFC] = F3DZEXOP(SETCOMBINE)
	, [0xFD] = F3DZEXOP(SETTIMG)
	, [0xFE] = F3DZEXOP(SETZIMG)
	, [0xFF] = F3DZEXOP(SETCIMG)
};


void
f3dzex_exec(F3DZEXOPARGS)
{
	if (microcode_op[*b])
		microcode_op[*b](ctx, b);
}


void
f3dtree_addChild(struct f3dtree *tree, struct f3dtree *child)
{
	/* keep track of parent */
	child->parent = tree;
	
	/* no child currently registered to current tree */
	if (!tree->child)
		tree->child = child;
	/* current tree already has a child */
	else
	{
		/* so find the last child (will have no sibling) */
		struct f3dtree *walk = tree->child;
		while (walk->next)
			walk = walk->next;
		/* and register the new child as its sibling */
		walk->next = child;
	}
}


void
f3dtree_register(struct n64ctx *ctx, struct f3dtree *child)
{
	if (!child)
		return;
	if (!ctx->f3dtree)
	{
		ctx->f3dtree = child;
		child->name = "MAIN";
		return;
	}
	f3dtree_addChild(ctx->f3dtree, child);
}


struct f3dtree *
f3dtree_search_data(struct f3dtree *tree, unsigned char *data)
{
	if (!tree)
		return 0;
	
	while (tree)
	{
		if (tree->data == data)
			return tree;
		
		if (tree->child)
		{
			struct f3dtree *child_result;
			child_result = f3dtree_search_data(tree->child, data);
			if (child_result)
				return child_result;
		}
		
		tree = tree->next;
	}
	
	return 0;
}


struct f3dtree *
f3dtree_new(void *calloc(size_t nmemb, size_t size))
{
	struct f3dtree *tree = calloc(1, sizeof(*tree));
	if (!tree)
		return 0;
	tree->is_expanded = 1;
	return tree;
}


struct f3dtree *
f3dzex_get_tree(F3DZEXOPARGS, void *calloc(size_t nmemb, size_t size))
{
	if (!b)
		return 0;
	struct f3dtree *tree = calloc(1, sizeof(*tree));
	if (!tree)
		return 0;
	
	tree->is_expanded = 1;
	
	/* back up current opcode */
	memcpy(tree->og_data, b, 8);
	
	/* back up current offset */
	tree->data = b;
	tree->name = "DL";//"PTR";
	
	for( int walking = 1; walking; b += 8)
	{
		unsigned char *call = 0;
		char *call_type = 0;
		unsigned char tu8[4];
		uint32_t call_dlofs;
		
		struct f3dtree *child;
		
		/* keep track of current location */
		ctx->b = b;
		
		switch (*b)
		{
		case 0x01:
			/* 0x01 command: propagate vertex buffer */
			microcode_op[*b](ctx, b);
			break;
		case 0x03:
			/* 0x03 command: rectangular culling volume */
			child = f3dtree_new(calloc);
			if (!child)
				return 0;
			child->name = "CULL";
			child->type = F3DTREE_CULLDL;
			child->data = b;
			//child->dlofs = tree->dlofs + (b - tree->data);
			child->raw.culldl.vfirst = ctx->vbuf[getu16(b+2)>>1].pos;
			child->raw.culldl.vlast  = ctx->vbuf[getu16(b+6)>>1].pos;
			/*fprintf(
				stderr
				, "vfirst = {%f, %f, %f}\n""vlast = {%f, %f, %f}\n"
				, child->raw.culldl.vfirst.pos.x
				, child->raw.culldl.vfirst.pos.y
				, child->raw.culldl.vfirst.pos.z
				, child->raw.culldl.vlast.pos.x
				, child->raw.culldl.vlast.pos.y
				, child->raw.culldl.vlast.pos.z
			);*/
			f3dtree_addChild(tree, child);
			break;
		case 0xE1:
			/* 0xE1 command: sets higher half of generic RDP word */
			ctx->rdp.hi = getw32(b + 4);
			break;
		case 0x04: {
			tu8[0] = (ctx->rdp.hi >> 24) & 0xFF;
			tu8[1] = (ctx->rdp.hi >> 16) & 0xFF;
			tu8[2] = (ctx->rdp.hi >>  8) & 0xFF;
			tu8[3] = (ctx->rdp.hi >>  0) & 0xFF;
			call_dlofs = ctx->rdp.hi;
			call = ptr(ctx, tu8);
			call_type = "BRANCH";
			break;
		}
		case HC_DL:
			call_dlofs = getw32(b+4);
			call = ptr(ctx, b + 4);
			call_type = "DL";
			if (b[1] == 0x01)
				walking = 0;
			break;
		case HC_ENDDL:
			walking = 0;
		}
			
		if (call)
		{
			struct f3dtree *child;
			child = f3dzex_get_tree(ctx, call, calloc);
			
			/* something went wrong */
			if (!child)
				return 0;
			
			child->name = call_type;
			child->dlofs = call_dlofs;
			
			f3dtree_addChild(tree, child);
		}
	}
	
	return tree;
}


void
f3dzex_walk(F3DZEXOPARGS)
{
	int highlighting = (b == ctx->highlight_me);
	if (highlighting)
		ctx->highlight_on = 1;
	++ctx->walk_nesting_level;
	//fprintf(stderr, ">>> WALK BEGIN <<<\n");
	for( ; *b != HC_ENDDL; b += 8)
	{
		if (!microcode_op[*b])
			continue;
		
		/* keep track of current location */
		ctx->b = b;
		
		//if( getw32(b+4) == 0x0007C07C)
		//	continue;
		
		//fprintf(stderr, "%08X %08X\n", getw32(b), getw32(b+4));
		
		/* testing purposes only */
		/*if (*b == 0xF0)
		{	fprintf(stderr, "--------\n");
			break;}*/
		
		microcode_op[*b](ctx, b);
		
		/* do not continue with this display list: it must branch */
		if (ctx->force_branch)
			break;
		
		/* this byte combination functions as ENDDL */
		if (*b == HC_DL && b[1] == 0x01)
			break;
	}
	--ctx->walk_nesting_level;
	if (!ctx->walk_nesting_level && ctx->force_branch)
	{
		while (ctx->force_branch)
		{
			unsigned char *force_branch = ctx->force_branch;
			ctx->force_branch = 0;
			f3dzex_walk(ctx, force_branch);
		}
	}
	if (highlighting)
		ctx->highlight_on = 0;
}


void
f3dzex_walk_zmode(F3DZEXOPARGS)
{
	int highlighting = (b == ctx->highlight_me);
	if (highlighting)
		ctx->highlight_on = 1;
	++ctx->walk_nesting_level;
	//fprintf(stderr, ">>> WALK BEGIN <<<\n");
	for( ; *b != HC_ENDDL; b += 8)
	{
		/* TODO skip more than just triangle commands */
		if (ctx->othermode.indep.zmode != ctx->only_zmode && (*b == 0x05 || *b==0x06))
			continue;
		
		if (!microcode_op[*b])
			continue;
		
		/* keep track of current location */
		ctx->b = b;
		
		//if( getw32(b+4) == 0x0007C07C)
		//	continue;
		
		//fprintf(stderr, "%08X %08X\n", getw32(b), getw32(b+4));
		
		/* testing purposes only */
		/*if (*b == 0xF0)
		{	fprintf(stderr, "--------\n");
			break;}*/
		
		microcode_op[*b](ctx, b);
		
		/* do not continue with this display list: it must branch */
		if (ctx->force_branch)
			break;
		
		/* this byte combination functions as ENDDL */
		if (*b == HC_DL && b[1] == 0x01)
			break;
	}
	--ctx->walk_nesting_level;
	if (!ctx->walk_nesting_level && ctx->force_branch)
	{
		while (ctx->force_branch)
		{
			unsigned char *force_branch = ctx->force_branch;
			ctx->force_branch = 0;
			f3dzex_walk_zmode(ctx, force_branch);
		}
	}
	if (highlighting)
		ctx->highlight_on = 0;
}


void
f3dzex_select_microcode(enum n64microcode microcode)
{
	switch (microcode)
	{
		case N64MICRO_F3DZEX:
			memcpy(microcode_op, f3dzex_op, sizeof(microcode_op));
			break;
		
		case N64MICRO_F3DEX:
			memcpy(microcode_op, f3dex_op, sizeof(microcode_op));
			break;
	}
}

