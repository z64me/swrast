#include <swrast/inputassembler.h>
#include <swrast/framebuffer.h>
#include <swrast/rasterizer.h>
#include <swrast/texture.h>
#include <swrast/context.h>
#include <swrast/window.h>
#include <swrast/vector.h>
#include <swrast/color.h>
#include <swrast/input.h>


#include <n64ctx.h>
#include <binding.h>
#include <zui.h>

struct udata {
	struct swr_framebuffer *fb;
	struct swr_window *w;
	void *ctx_surf;
	struct zuiSprite *font;
	struct swr_context ctx;
};

static
int
input(struct n64binding *binding)
{
	struct udata *udata = binding->udata;
	return window_handle_events(udata->w);
}

static
void
clear(
	struct n64binding *binding
	, unsigned char r
	, unsigned char g
	, unsigned char b
	, unsigned char a
)
{
	struct udata *udata = binding->udata;
	swr_framebuffer_clear(udata->fb, r, g, b, a);
	swr_framebuffer_clear_depth(udata->fb, 1.0);
}

static
void
show(
	struct n64binding *binding
)
{
	struct udata *udata = binding->udata;
	window_display_framebuffer(udata->w);
}

static
void
set_modelview(
	struct n64binding *binding
	, float m[16]
)
{
	struct udata *udata = binding->udata;
	swr_context_set_modelview_matrix(&udata->ctx, m);
}

static
float *
depth_ofs(struct n64binding *binding)
{
	struct udata *udata = binding->udata;
	return &udata->ctx.depth_ofs;
}

static
void
cleanup(struct n64binding *binding)
{
	struct udata *udata = binding->udata;
	window_destroy(udata->w);
}

static
int
init(
	struct n64binding *binding
	, char *window_name
	, int window_width
	, int window_height
)
{
	struct udata *udata = binding->udata;
	struct swr_context *ctx = &udata->ctx;
	//struct n64binding_idat *idat = binding->idat;
	
	float far, near, aspect, f, iNF, m[16];
	unsigned int x, y;

	/************* initalization *************/
	udata->w = window_create(window_name, window_width, window_height);

	if (udata->w == NULL)
		return EXIT_FAILURE;
	
	udata->ctx_surf = window_getDrawingSurface(udata->w);
	binding->ctx_surf = udata->ctx_surf;
	
	udata->font = zuiSprite_debugFont();
	
	/* n64input and swr_input are identical, so this is okay */
	binding->input_struct = (struct n64input*)window_get_input(udata->w);

	swr_context_init(ctx);

	/* intialize projection matrix */
	far    = 0.5f;
	near   = 500.0f;
	aspect = (float)window_width / (float)window_height;
	f      = 1.0 / tan(60.0f * (3.14159265359f / 180.0f) * 0.5f);
	iNF    = 1.0 / (near - far);

	m[0]=f/aspect; m[4]=0.0f; m[ 8]= 0.0f;           m[12]=0.0f;
	m[1]=0.0f;     m[5]=f;    m[ 9]= 0.0f;           m[13]=0.0f;
	m[2]=0.0f;     m[6]=0.0f; m[10]= (far+near)*iNF; m[14]=2.0f*far*near*iNF;
	m[3]=0.0f;     m[7]=0.0f; m[11]=-1.0f;           m[15]=0.0f;

	swr_context_set_projection_matrix(ctx, m);

	/* initialize T&L state */
	ctx->depth_test = COMPARE_LESS_EQUAL;
	ctx->flags |= DEPTH_TEST | FRONT_CCW;

	ctx->light[0].enable = 0;
	ctx->light[0].diffuse = vec4_set(1.0f, 1.0f, 1.0f, 1.0f);
	ctx->light[0].specular = vec4_set(1.0f, 1.0f, 1.0f, 1.0f);
	ctx->light[0].attenuation_constant = 1.0f;

	ctx->material.diffuse = vec4_set(0.5f, 0.5f, 0.5f, 1.0f);
	ctx->material.specular = vec4_set(0.5f, 0.5f, 0.5f, 1.0f);
	ctx->material.ambient = vec4_set(0.0f, 0.0f, 0.0f, 1.0f);
	ctx->material.emission = vec4_set(0.0f, 0.0f, 0.0f, 1.0f);
	ctx->material.shininess = 127;
	
	/* default shader */
	ctx->shader = shader_internal(SHADER_UNLIT);

	udata->fb = window_get_framebuffer(udata->w);
	ctx->target = udata->fb;

	swr_context_set_viewport(ctx, 0, 0, window_width, window_height);
	
#if 0 //TODO
	/* for scene explorer right bar */
	if (idat->drawmode == idat->drawmode_scene)
		ctx->draw_area.maxx -= idat->right_pane_w;
#else
	ctx->draw_area.maxx -= 213; /* TODO hardcodes sidebar */
#endif
	
	/* set up appropriate n64ctx */
	binding->n64ctx = n64ctx_new_swrast(ctx);
	
	/* bind to this context */
	n64rast_use_n64ctx_swrast(binding->n64ctx);
	ctx->n64ctx = binding->n64ctx;
	
	return 0;
}

struct n64binding *
n64binding_new_swrast(N64BINDING_NEW_ARGS)
{
	struct n64binding *binding = calloc(1, sizeof(*binding));
	struct udata *udata = calloc(1, sizeof(*udata));
	
	if (!binding || !udata)
		return 0;
	
	binding->type = N64BINDING_SWRAST;
	binding->udata = udata;
	
	/* set up function pointers */
	binding->init  = init;
	binding->clear = clear;
	binding->show  = show;
	binding->input = input;
	binding->set_modelview = set_modelview;
	binding->depth_ofs = depth_ofs;
	binding->cleanup = cleanup;
	
	return binding;
}

