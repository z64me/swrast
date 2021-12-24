#include <swrast/framebuffer.h>
#include <swrast/color.h>

#include <stdlib.h>

int swr_framebuffer_init_color(
	struct swr_framebuffer *fb
	, unsigned int width
	, unsigned int height
	, void *color
)
{
	fb->width = width;
	fb->height = height;

	fb->depth = malloc(width * height * sizeof(float));

	if (!fb->depth)
		return 0;

	fb->color = color;

	if (!fb->color)
		return 0;
	
	return 1;
}

int swr_framebuffer_init(struct swr_framebuffer *fb, unsigned int width, unsigned int height)
{
	fb->width = width;
	fb->height = height;

	fb->color = malloc(width * height * 4);

	if (!fb->color)
		return 0;

	fb->depth = malloc(width * height * sizeof(float));

	if (!fb->depth) {
		free(fb->color);
		return 0;
	}
	return 1;
}

void swr_framebuffer_cleanup(struct swr_framebuffer *fb)
{
	free(fb->depth);
	free(fb->color);
}

void swr_framebuffer_clear(struct swr_framebuffer *fb, int r, int g, int b, int a)
{
	color4 *ptr = fb->color, val = color_set(r, g, b, a);
	unsigned int i, count = fb->height * fb->width;

	for (i = 0; i < count; ++i)
		*(ptr++) = val;
}

void swr_framebuffer_clear_depth(struct swr_framebuffer *fb, float value)
{
	unsigned int i, count = fb->height * fb->width;
	float *ptr;

	value = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);

	for (ptr = fb->depth, i = 0; i < count; ++i, ++ptr)
		*ptr = value;
}
