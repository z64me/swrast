/**
 * \file context.h
 *
 * \brief Contains the rendering context implementation
 */
#ifndef SWR_CONTEXT_H
#define SWR_CONTEXT_H

#include "predef.h"
#include "config.h"
#include "rasterizer.h"
#include "shader.h"
#include "vector.h"
#include "color.h"

/**
 * \enum COMPARE_FUNCTION
 *
 * \brief Comparison function for depth test
 */
typedef enum {
	COMPARE_ALWAYS = 0x00,		/**< \brief Always pass */
	COMPARE_NEVER = 0x01,		/**< \brief Always fail */
	COMPARE_EQUAL = 0x02,		/**< \brief Pass if equal */
	COMPARE_NOT_EQUAL = 0x03,	/**< \brief Pass if not equal */
	COMPARE_LESS = 0x04,		/**< \brief Pass if less */
	COMPARE_LESS_EQUAL = 0x05,	/**< \brief Pass if less or equal */
	COMPARE_GREATER = 0x06,		/**< \brief Pass if greater */
	COMPARE_GREATER_EQUAL = 0x07	/**< \brief Pass if greater or equal */
} COMPARE_FUNCTION;

/**
 * \enum VERTEX_FORMAT
 *
 * \brief Vertex format flags
 */
typedef enum {
	/** \brief 2 component float position */
	VF_POSITION_F2 = 0x0001,
	/** \brief 3 component float position */
	VF_POSITION_F3 = 0x0002,
	/** \brief 4 component float position */
	VF_POSITION_F4 = 0x0004,
	/** \brief 3 component float normal */
	VF_NORMAL_F3 = 0x0010,

	/** \brief 3 component float color */
	VF_COLOR_F3 = 0x0100,
	/** \brief 4 component float color */
	VF_COLOR_F4 = 0x0200,
	/** \brief 3 component unsigned byte color */
	VF_COLOR_UB3 = 0x0400,
	/** \brief 4 component unsigned byte color */
	VF_COLOR_UB4 = 0x0800,

	/**
	 * \brief 2 component float texture coordinates for texture channel 0
	 */
	VF_TEX0 = 0x1000
} VERTEX_FORMAT;

/**
 * \enum CONTEXT_FLAGS
 *
 * \brief State flags for the rendering context
 */
typedef enum {
	/** \brief Clip out-of-bounds depth values */
	DEPTH_CLIP = 0x0001,
	/** \brief Enable write to depth buffer */
	DEPTH_WRITE = 0x0002,
	/** \brief Enable depth test */
	DEPTH_TEST = 0x0004,
	/** \brief Counter-clock-wise is front facing */
	FRONT_CCW = 0x0008,
	/** \brief Cull front facing triangles */
	CULL_FRONT = 0x0010,
	/** \brief Cull back facing triangles */
	CULL_BACK = 0x0020,
	/** \brief Enable color blending */
	BLEND_ENABLE = 0x0040
} CONTEXT_FLAGS;

/**
 * \struct context
 *
 * \brief A context encapsulates all global state of the rendering pipeline
 */
struct swr_context {
	void *n64ctx;
	/** \brief Immediate mode rendering state */
	struct {
		rs_vertex vertex[3];
		rs_vertex next;
		int current;
		int active;
	} immediate;

	/** \brief The settings of a light */
	struct {
		vec4 ambient;
		vec4 diffuse;
		vec4 specular;
		vec4 position;
		float attenuation_constant;
		float attenuation_linear;
		float attenuation_quadratic;
		int enable;
	} light[MAX_LIGHTS];

	/** \brief The surface material parameters for lighting calculations */
	struct {
		vec4 ambient;
		vec4 diffuse;
		vec4 specular;
		vec4 emission;
		int shininess;
	} material;

	/**
	 * \brief Identifies the actual drawing area on the framebuffer
	 *
	 * The normalized device coordinates after transformation are mapped
	 * to lie within the viewport area.
	 *
	 * \note Do NOT set this directly, use swr_context_set_viewport.
	 */
	struct {
		int x;                  /**< \brief Distance from left */
		int y;                  /**< \brief Distance from top */
		unsigned int width;     /**< \brief Horizontal extents */
		unsigned int height;    /**< \breif Vertical extents */
	} viewport;

	/**
	 * \brief The actual drawing area, computed from the viewport
	 *        by swr_context_set_viewport
	 */
	struct {
		int minx;
		int miny;
		int maxx;
		int maxy;
	} draw_area;

	struct {
		int index;
		rs_vertex vtx;
	} post_tl_cache[MAX_INDEX_CACHE];
	
	/* input structure */
	struct swr_input *input;

	int flags;      /**< \brief A set of CONTEXT_FLAGS */

	/** \brief Depth test comparison function */
	COMPARE_FUNCTION depth_test;

	/** \brief Non-zero to enable a texture layer, zero to disable */
	int texture_enable[MAX_TEXTURES];

	/** \brief Pointer to textures for different texture layers */
	texture *textures[MAX_TEXTURES];

	/** \brief Model-View matrix used by T&L stage */
	float modelview[16];

	/** \brief Projection matrix used by T&L stage */
	float projection[16];

	/** \brief Normal matrix computed from modelview matrix */
	float normalmatrix[16];

	/** \brief Input assembler vertex format */
	int vertex_format;

	/** \brief Vertex buffer for input assember */
	void *vertexbuffer;

	/** \brief Index buffer for input assembler */
	unsigned short *indexbuffer;

	/** \brief Which shader program to use */
	const struct swr_shaderProgram *shader;

	/** \brief Depth value that minimum distance is mapped to */
	float depth_near;

	/** \brief Depth value that maximum distance is mapped to */
	float depth_far;
	
	/** \brief Offset depth testing values (used for decals) */
	float depth_ofs;

	/** \brief Frame buffer that the rasterizer draws to */
	struct swr_framebuffer *target;

	/** \brief Color mask determined from flags */
	color4 colormask;
};

static MATH_CONST int depth_test(const struct swr_context* ctx,
				const float z, const float ref)
{
	int depthtest[8];

	if (ctx->flags & DEPTH_TEST) {
		float z1 = z + ctx->depth_ofs;
		depthtest[COMPARE_ALWAYS] = 1;
		depthtest[COMPARE_NEVER] = 0;
		depthtest[COMPARE_LESS] = z1 < ref;
		depthtest[COMPARE_GREATER] = z1 > ref;
		depthtest[COMPARE_NOT_EQUAL] = depthtest[COMPARE_LESS] |
						depthtest[COMPARE_GREATER];
		depthtest[COMPARE_EQUAL] = !depthtest[COMPARE_NOT_EQUAL];
		depthtest[COMPARE_LESS_EQUAL] = depthtest[COMPARE_EQUAL] |
						depthtest[COMPARE_LESS];
		depthtest[COMPARE_GREATER_EQUAL] = depthtest[COMPARE_EQUAL] |
		depthtest[COMPARE_GREATER];

		if( !depthtest[ctx->depth_test] )
			return 0;
	}

	if ((ctx->flags & DEPTH_CLIP) &&
		(z > ctx->depth_far || z < ctx->depth_near)) {
		return 0;
	}
	return 1;
}

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize a context object
 *
 * \memberof context
 *
 * \param ctx A pointer to a context
 */
void swr_context_init(struct swr_context *ctx);

/**
 * \brief Set the currently active model view matrix and update the
 *        normal matrix
 *
 * \memberof context
 *
 * \param ctx A pointer to a context
 * \param f   A pointer to an array of 16 float values, representing a 4x4
 *            matrix, stored in column-major order
 */
void swr_context_set_modelview_matrix(struct swr_context *ctx, float *f);

/**
 * \brief Set the currently active projection matrix
 *
 * \memberof context
 *
 * \param ctx A pointer to a context
 * \param f   A pointer to an array of 16 float values, representing a 4x4
 *            matrix, stored in column-major order
 */
void swr_context_set_projection_matrix(struct swr_context *ctx, float *f);

/**
 * \brief Configure viewport mapping
 *
 * \memberof context
 *
 * \note When this function is called, a vaild framebuffer MUST be bound
 *
 * \param ctx    A pointer to a context
 * \param x      The distance from the left of the framebuffer
 * \param y      The distance from the top of the framebuffer
 * \param width  The horizontal extents of the viewport
 * \param height The vertical extents of the viewport
 */
void swr_context_set_viewport(struct swr_context *ctx, int x, int y,
			unsigned int width, unsigned int height);

#ifdef __cplusplus
}
#endif

#endif /* SWR_CONTEXT_H */

