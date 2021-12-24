#ifndef WINDOW_H
#define WINDOW_H

#include <stddef.h>

#include "framebuffer.h"

typedef struct swr_window swr_window;

#ifdef __cplusplus
extern "C" {
#endif

void *
window_getDrawingSurface(struct swr_window *window);

swr_window *window_create(char *title, size_t width, size_t height);

void window_destroy(swr_window *wnd);

/* Return non-zero if the window is still active, zero if it got closed */
int window_handle_events(swr_window *wnd);

void window_display_framebuffer(swr_window *wnd);

struct swr_framebuffer *window_get_framebuffer(swr_window *wnd);

struct swr_input *
window_get_input(struct swr_window *wnd);

#ifdef __cplusplus
}
#endif

#endif /* WINDOW_H */

