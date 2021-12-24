/**
 * \file predef.h
 *
 * \brief Forward declerations of structures
 */
#ifndef PREDEF_H
#define PREDEF_H

typedef struct texture texture;
typedef struct swr_framebuffer swr_framebuffer;
typedef struct swr_context swr_context;
typedef struct swr_shaderProgram swr_shaderProgram;
typedef struct rs_vertex rs_vertex;
typedef struct vec4 vec4 __attribute__ ((aligned (16)));
typedef union color4 color4 __attribute__ ((aligned (4)));

#ifndef MATH_INLINE
	#define MATH_INLINE __inline__
#endif

#ifndef MATH_CONST
	#define MATH_CONST __attribute__((const)) __inline__
#endif

#endif /* PREDEF_H */

