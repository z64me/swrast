#include <swrast/window.h>
#include <swrast/input.h>

#include <SDL/SDL.h>
#include <stdint.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

struct swr_window {
	SDL_Surface *screen;
	struct swr_framebuffer fb;
	struct swr_input input;
	uint32_t last_ticks;
};

void *
window_getDrawingSurface(struct swr_window *window)
{
	return window->screen;
}

struct swr_window *
window_create(char *title, size_t width, size_t height)
{
	struct swr_window *win = calloc(1, sizeof(*win));

	if (!win)
		return NULL;
	
	win->screen = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE);
	
	if (!win->screen)
		goto failure;
	
	SDL_WM_SetCaption(title, 0);

	if (!swr_framebuffer_init_color(&win->fb, width, height, win->screen->pixels))
		goto failure;
	
	win->last_ticks = SDL_GetTicks();
	
	return win;
	
failure:
	if (win)
		free(win);
	
	return NULL;
}

void
window_destroy(struct swr_window *wnd)
{
	SDL_FreeSurface(wnd->screen);
	free(wnd);
	SDL_Quit();
}

int
window_handle_events(struct swr_window *wnd)
{
	SDL_Event event;
	struct swr_input *input = &wnd->input;
	int t;
	
	/* reset leftclick after two frames */
	if (input->mouse.button.leftclick)
	{
		input->mouse.button.leftclick += 1; /* increment */
		input->mouse.button.leftclick &= 1; /* reset after 2 */
	}
	
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			return 0;
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			t = (event.type == SDL_MOUSEBUTTONDOWN) ? 1 : 0;
			t &= (event.button.state == SDL_PRESSED) ? 1 : 0;
			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT:
				input->mouse.button.left = t;
				/* leftclick does not get reset here, is separate */
				if(t)
					input->mouse.button.leftclick = 1;
				break;
			case SDL_BUTTON_MIDDLE:
				input->mouse.button.middle = t;
				break;
			case SDL_BUTTON_RIGHT:
				input->mouse.button.right = t;
				break;
			}
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			t = (event.type == SDL_KEYDOWN) ? 1 : 0;
			switch (event.key.keysym.sym)
			{
			case SDLK_LCTRL:
				input->key.lctrl = t;
				break;
			case SDLK_w:
				input->key.w = t;
				break;
			case SDLK_a:
				input->key.a = t;
				break;
			case SDLK_s:
				input->key.s = t;
				break;
			case SDLK_d:
				input->key.d = t;
				break;
			default:
				break;
			}
			break;
		case SDL_MOUSEMOTION:
			input->mouse.pos.x = event.motion.x;
			input->mouse.pos.y = event.motion.y;
			break;
		}
	}
	
	return 1;
}

void
window_display_framebuffer(struct swr_window *wnd)
{
	struct timespec tim;
	struct timespec tim2;
	
	SDL_Flip(wnd->screen);

	/* TODO use SDL for this */
	/* wait for ~16.666 ms -> ~60 fps */
	tim.tv_sec = 0;
	tim.tv_nsec = 16666666L;
	tim.tv_nsec *= 3; /* 20 FPS hack */
	nanosleep(&tim, &tim2);
	
	/* delta time calculation */
	struct swr_input *input = &wnd->input;
	uint32_t ticks = SDL_GetTicks();
	input->delta_time_sec = ticks - wnd->last_ticks;
	input->delta_time_sec *= 0.001;
	wnd->last_ticks = ticks;
}

struct swr_framebuffer *
window_get_framebuffer(struct swr_window *wnd)
{
	return &wnd->fb;
}

struct swr_input *
window_get_input(struct swr_window *wnd)
{
	return &wnd->input;
}

