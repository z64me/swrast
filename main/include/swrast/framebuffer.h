/**
 * \file framebuffer.h
 *
 * \brief Contains a frame buffer object implementation
 */
#ifndef SWR_FRAMEBUFFER_H
#define SWR_FRAMEBUFFER_H

#include "predef.h"
#include "config.h"

/**
 * \struct swr_framebuffer
 *
 * \brief Holds the data of a frame buffer
 */
struct swr_framebuffer {
	color4 *color;		/**< \brief Color buffer scan line data */
	float *depth;		/**< \brief Depth buffer scan line data */
	int width;		/**< \brief Frame buffer width in pixels */
	int height;		/**< \brief Frame buffer height in pixels */
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize an uninitialized frame buffer object
 *        (32 bpp RGBA + 32 bit depth buffer)
 *
 * \memberof swr_framebuffer
 *
 * \param fb     A pointer to an uninitialized frame buffer object
 * \param width  The width of the frame buffer in pixels
 * \param height The height of the frame buffer in pixels
 *
 * \return Non-zero on success, zero on failure
 */
int swr_framebuffer_init(struct swr_framebuffer *fb,
			unsigned int width, unsigned int height);

int swr_framebuffer_init_color(
	struct swr_framebuffer *fb
	, unsigned int width
	, unsigned int height
	, void *color
);

/**
 * \brief Destroy a frame buffer object and free its resources
 *
 * \memberof swr_framebuffer
 *
 * \param fb A pointer to a frame buffer structure
 */
void swr_framebuffer_cleanup(struct swr_framebuffer *fb);

/**
 * \brief Clear the color buffer of a frame buffer object
 *
 * \memberof swr_framebuffer
 *
 * \param fb A pointer to a frame buffer structure
 * \param r  The red component of the clear color
 * \param g  The green component of the clear color
 * \param b  The blue component of the clear color
 * \param a  The alpha component of the clear color
 */
void swr_framebuffer_clear(struct swr_framebuffer *fb, int r, int g, int b, int a);

/**
 * \brief Clear the depth buffer of a frame buffer object
 *
 * \memberof swr_framebuffer
 *
 * \param fb    A pointer to a frame buffer structure
 * \param value The value to write into the depth buffer
 */
void swr_framebuffer_clear_depth(struct swr_framebuffer *fb, float value);

#ifdef __cplusplus
}
#endif

#endif /* SWR_FRAMEBUFFER_H */

