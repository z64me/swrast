#include <stdio.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* for magic reloading */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

static int do_billboards_0x01 = 0;

int single_display_test = 0;

#define SWRAST 0
#define RAYLIB 1

#if !defined(BINDING)
	#error Please compile with -DBINDING=X (RAYLIB or SWRAST)
#endif

#if (BINDING != RAYLIB && BINDING != SWRAST)
	#error Please compile with -DBINDING=X (RAYLIB or SWRAST)
#endif

/* makes widescreen when view width is 320 * 2 */
#define RIGHT_PANE_W 213//256

#define WIDTH_VIEW (320*2)
#define HEIGHT_VIEW (240*2)

#define WIDTH (WIDTH_VIEW+RIGHT_PANE_W)
#define HEIGHT HEIGHT_VIEW

#define RIGHT_PANE_X (WIDTH - RIGHT_PANE_W)
#define RIGHT_PANE_INFOBAR_H (8*4)
#define RIGHT_PANE_INFOBAR_Y (HEIGHT - RIGHT_PANE_INFOBAR_H)

#include <n64ctx.h>
#include <binding.h>
#include <f3dzex.h>
#include <zui.h>

#include <invert4x4_c.h>

#define ARCBALL_CAMERA_IMPLEMENTATION
#include <arcball_camera.h>

#define FLYTHROUGH_CAMERA_IMPLEMENTATION
#include <flythrough_camera.h>

static struct n64binding *n64binding = 0;
static struct n64input *input = 0;
static struct n64ctx *n64ctx = 0;

static void *zuifont = 0;
static void *ctx_surf = 0;

unsigned char *zobj = 0;
unsigned char *zmap = 0;
unsigned char *zscene = 0;
char *zscene_fn = 0;
char *zmap_fn = 0;
enum drawmode {
	DRAWMODE_OBJECT
	, DRAWMODE_SCENE
};
enum drawmode drawmode = DRAWMODE_OBJECT;
size_t zobj_size;
int ofs;

/* list of DLs to draw in order */
static
struct DL
{
	float  x;
	float  y;
	float  z;
	int    ofs;
} DL_list[256] = {0};
static
unsigned int DL_list_count = 0;

struct culler {
	struct vec4       color;
	struct vec4       pos;
	union {
		struct {
			float radius;
		} sphere;
		struct {
			struct vec4 vfirst;
			struct vec4 vlast;
		} box;
	} data;
	enum f3dtreeType  type;
	struct f3dtree   *f3dtree;
};

static
struct {
	struct culler *culler;
	struct culler *culler_selected;
	struct culler  culler_temp;
	int culler_max;
	int culler_num;
	char show_pointer_cullers;
	char show_inline_cullers;
} global = {0};


static
inline
short int
gets16(unsigned char *b)
{
	short int v = (b[0]<<8)|b[1];
	return v;
}



static
inline
struct culler *
culler_push(void)
{
	if (global.culler_num+1 >= global.culler_max)
	{
		if (!global.culler_max)
			global.culler_max = 4096;
		else
		{
			fprintf(stderr, "[!] memory error!\n");
			exit(EXIT_FAILURE);
			global.culler_max *= 2;
		}
		global.culler =
			realloc(
				global.culler
				, sizeof(*global.culler) * global.culler_max
			)
		;
		if (!global.culler)
		{
			fprintf(stderr, "[!] memory error!\n");
			exit(EXIT_FAILURE);
		}
	}
	global.culler_num += 1;
	return &global.culler[global.culler_num - 1];
}


static
inline
struct culler *
culler_push_sphere(int x, int y, int z, int r)
{
	struct culler *culler = culler_push();
	float scale = 0.01f; /* TODO hard-coded scale is bad */
	culler->type = F3DTREE_CULLSPHERE;
	culler->pos.x = scale * x;
	culler->pos.y = scale * y;
	culler->pos.z = scale * z;
	culler->data.sphere.radius = scale * r;
	return culler;
}


static
inline
unsigned int
getw32(unsigned char *b)
{
	return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}


/* keep trying to open file until successful, then keep
 * trying to read file into buffer until successful */
static
inline
void
persistent_readfile(char *fn, unsigned char *dest)
{
	FILE *fp = 0;
	size_t bytes;
	
	while (!fp)
		fp = fopen(fn, "rb");
	
	fseek(fp, 0, SEEK_END);
	bytes = ftell(fp);
	
	while (1)
	{
		fseek(fp, 0, SEEK_SET);
		if (fread(dest, 1, bytes, fp) == bytes)
			break;
	}
	
	fclose(fp);
}

static
void
showargs(char *errmsg)
{
#define TAB "   "
	fprintf(stderr,
		"\n""A few command line examples you can learn from:\n\n"
		
		TAB "Mirror Shield\n"
		TAB TAB"zzviewer-rrw -z \"object_link_majora.zobj\" 0x16480\n\n"
		
		TAB "Master Sword\n"
		TAB TAB "zzviewer-rrw -r0x04 \"gameplay_keep.zobj\" -z \"object_link_boy.zobj\" 0x21F78\n\n"
		
		
		TAB "Deku Tree\n"
		TAB TAB "zzviewer-rrw -s \"ydan_scene.zscene\" \"ydan_room_0.zscene\"\n\n"
		
		TAB "Hylian Guard (full skeleton)\n"
		TAB TAB "zzviewer-rrw -z1 \"object_sd.zobj\" \"0xA6F8(0,0,0)|0xB3D0(479,-81,417)|0xB240(1631,-81,417)|0xB158(3165,-81,417)|0xAAC0(958,-162,0)|0xA930(2110,-162,0)|0xA848(3644,-162,0)|0xA440(0,0,0)|0xB8C0(1300,0,850)|0xB730(2600,0,850)|0xB520(3550,0,850)|0xB050(2600,0,-20)|0xAEC0(4000,0,-20)|0xAC10(4850,0,-20)|0xA030(4800,0,-20)\"\n\n"
		
		" == Extra Notes ==\n"
		"You can append '+0xOffset' onto a ram segment to begin at 'offset' bytes\n"
		"within the file when loading it. This is useful when loading Link's mouth\n"
		"texture into segment 0x09, like so: -r0x09+0x4000 \"object_link_boy.zobj\"\n\n"
		
		"Lastly, you can append 'r' onto a ram segment to tell zzviewer-rrw to reload\n"
		"the file whenever changes are detected, like so -r0x04r \"gameplay_keep.zobj\"\n"
		"(Do not increase the file-size or rearrange data when using this feature.)\n\n"
		
		" == Camera Controls ==\n"
		"When viewing a map, you can rotate the camera by left-click-dragging\n"
		"within the viewport. The WASD keys move you through the scene.\n\n"
		
		"When viewing an object, you zoom in and out by right-click-dragging\n"
		"up or down within the viewport. Left-click-drag to rotate the object.\n"
   );
	if (errmsg)
		fprintf(stderr, errmsg);
	exit(EXIT_FAILURE);
}

static
unsigned char *
file_load(char const *fn, size_t *sz)
{
	if (!fn || !sz)
		return 0;
	
	FILE *fp = fopen(fn, "rb");
	unsigned char *raw;
	
	if (!fp)
		return 0;
	
	fseek(fp, 0, SEEK_END);
	
	*sz = ftell(fp);
	
	if (!sz)
		goto close_and_fail;
	
	raw = malloc(*sz);
	
	if (!raw)
		goto close_and_fail;
	
	fseek(fp, 0, SEEK_SET);
	
	if (fread(raw, 1, *sz, fp) != *sz)
		goto close_and_free;
	
	fclose(fp);
	
	return raw;
	
close_and_free:
	free(raw);
close_and_fail:
	fclose(fp);
	return 0;
}


static
void
do_dlist(unsigned char *dl)
{
	dl = f3dzex_ptr(n64ctx, dl);
	if (!dl)
		return;
	/* zmode magic */
	f3dzex_walk_zmode(n64ctx, dl);
}


static
inline
int
point_in_rect(int px, int py, int rx, int ry, int rw, int rh)
{
	if (px < rx)
		return 0;
	if (py < ry)
		return 0;
	if (px > rx + rw)
		return 0;
	if (py > ry + rh)
		return 0;
	return 1;
}


static
inline
int
ui_option(char *str, char *on, int x, int y, int row)
{
	char ok[64];
	int changed = 0;
#if 1 // TODO needs to be adapted to new binding setup
	sprintf(ok, "[ ] %s", str);
	
	/* test mouse click for selecting item */
	if (input->mouse.button.leftclick == 1)
	{
		if (point_in_rect(
			input->mouse.pos.x, input->mouse.pos.y
			, x, y + row * 8, 8*3, 8))
		{
			input->mouse.button.left = 0;
			*on = !*on;
			changed = 1;
		}
	}
	
	if (*on)
		ok[1] = 'x';
	
	zuiSprite_debugFont_draw(
		ctx_surf
		, zuifont
		, ok
		, x
		, y + row * 8
	);
#endif
	
	return changed;
}


static
inline
void
camera_arcball(float m[16])
{
	/* camera magic */
	static float pos[3] = { 0.0f, 0.0f, 3.0f };
	static float target[3] = { 0.0f, 0.0f, 0.0f };

	// initialize "up" to be tangent to the sphere!
	// up = cross(cross(look, world_up), look)
	static float up[3];
	static int up_initialized = 0;
	if (!up_initialized)
	{
		up_initialized = 1;
		float look[3] = { target[0] - pos[0], target[1] - pos[1], target[2] - pos[2] };
		float look_len = sqrtf(look[0] * look[0] + look[1] * look[1] + look[2] * look[2]);
		look[0] /= look_len;
		look[1] /= look_len;
		look[2] /= look_len;

		float world_up[3] = { 0.0f, 1.0f, 0.0f };

		float across[3] = {
			look[1] * world_up[2] - look[2] * world_up[1],
			look[2] * world_up[0] - look[0] * world_up[2],
			look[0] * world_up[1] - look[1] * world_up[0],
		};

		up[0] = across[1] * look[2] - across[2] * look[1];
		up[1] = across[2] * look[0] - across[0] * look[2];
		up[2] = across[0] * look[1] - across[1] * look[0];
		
		float up_len = sqrtf(up[0] * up[0] + up[1] * up[1] + up[2] * up[2]);
		up[0] /= up_len;
		up[1] /= up_len;
		up[2] /= up_len;
	}
	
	static struct {
		int x;
		int y;
	} oldcursor = {0}, cursor = {0};
	
	cursor.x = input->mouse.pos.x;
	cursor.y = input->mouse.pos.y;
	
	if (input->mouse.button.right)
		input->mouse.wheel.dy = oldcursor.y - cursor.y;
	else
		input->mouse.wheel.dy = 0;

	arcball_camera_update(
		pos, target, up, m
		, input->delta_time_sec
		, 0.1f // zoom per tick
		, 0.001f // pan speed
		, 3.0f // rotation multiplier
		, (WIDTH_VIEW), (HEIGHT_VIEW) // screen (window) size
		, oldcursor.x, cursor.x
		, oldcursor.y, cursor.y
		, 0//input->mouse.button.middle
		, input->mouse.button.left
		, input->mouse.wheel.dy
		, 0
	);
	
	oldcursor = cursor;
}


static
inline
void
camera_flythrough(float m[16])
{
	/* camera magic */
	static float pos[3] = { 0.0f, 0.0f, 0.0f };
	static float look[3] = {0, 0, -1};
	const float up[3] = {0, 1, 0};
	
	static struct {
		int x;
		int y;
	} oldcursor = {0}, cursor = {0};
	
	cursor.x = input->mouse.pos.x;
	cursor.y = input->mouse.pos.y;
	
	if (input->mouse.button.right)
		input->mouse.wheel.dy = oldcursor.y - cursor.y;
	else
		input->mouse.wheel.dy = 0;
	
	int activated = input->mouse.button.left;
	
	flythrough_camera_update(
		pos, look, up, m
		, input->delta_time_sec
		, 4.0f * (input->key.lshift ? 2.0f : 1.0f)
		, 0.5f * activated
		, 80.0f
		, cursor.x - oldcursor.x, cursor.y - oldcursor.y
		, input->key.w
		, input->key.a
		, input->key.s
		, input->key.d
		, 0//input->key.space
		, 0//input->key.lctrl
		, 0
	);
	
	oldcursor = cursor;
}


static
inline
void
box(struct vec4 *vfirst, struct vec4 *vlast)
{
#if 1 //TODO we need a way to abstract this between bindings
	n64rast_box(n64ctx, vfirst, vlast);
#endif
}


static
inline
void
scene_boxes(void)
{
	//for (int i=0; i < global.culler_num; ++i)
	{
		struct culler *culler = global.culler_selected;//&global.culler[i];
		
		if (!culler)
			return;
		
		if (culler->type == F3DTREE_CULLSPHERE)
		{
			struct vec4 vfirst = {
				culler->pos.x - culler->data.sphere.radius
				, culler->pos.y - culler->data.sphere.radius
				, culler->pos.z - culler->data.sphere.radius
				, 0
			};
			struct vec4 vlast = {
				culler->pos.x + culler->data.sphere.radius
				, culler->pos.y + culler->data.sphere.radius
				, culler->pos.z + culler->data.sphere.radius
				, 0
			};
			
			box(&vfirst, &vlast);
		}
		else if(culler->type == F3DTREE_CULLDL)
		{
			box(
				&culler->data.box.vfirst
				, &culler->data.box.vlast
			);
		}
	}
}


static
inline
void
scene_print_tree(struct f3dtree *tree)
{
	static int level = 0;
	++level;
	while (tree)
	{
		for (int i=0; i < level; ++i)
			fprintf(stderr, " ");
		fprintf(stderr, "- %s\n", tree->name);
		if (tree->child)
			scene_print_tree(tree->child);
		tree = tree->next;
	}
	--level;
}


static
inline
void
scene_print_tree_zui(struct f3dtree *tree)
{
#if 1 // TODO must adapt to binding setup
	static int level = 0;
	static int yofs = 0;
	if (!tree->parent)
		yofs = 1;
	++level;
	while (tree)
	{
		char with_expanding[64];
		with_expanding[0] = '\0';
		
		int x = level * 8;
		int y = yofs * 8;
		yofs += 1;
		
		/* test mouse click for expanding, only if expandable */
		if (tree->child && input->mouse.button.leftclick == 1)
		{
			if (point_in_rect(
					input->mouse.pos.x, input->mouse.pos.y
					, (RIGHT_PANE_X + x)
					, (0 + y)
					, 8
					, 8
				))
			{
				input->mouse.button.left = 0;
				tree->is_expanded = !tree->is_expanded;
			}
		}
		
		/* highlight and click-test rectangle */
		struct {
			int x, y, w, h;
		} rect;
		
		/* initialize rectangle b/c it's needed */
		if ((input->mouse.button.leftclick == 1 || input->key.lctrl) || n64ctx->highlight_me == tree->data)
		{
			rect.x = RIGHT_PANE_X + x + 8;
			rect.y = y;
			rect.w = 16 * 8;//(strlen(tree->name)+2) * 8;
			rect.h = 8;
		}
		
		/* test mouse click for selecting item */
		if (input->mouse.button.leftclick == 1 || input->key.lctrl)
		{
			if (point_in_rect(
				input->mouse.pos.x, input->mouse.pos.y
				, rect.x, rect.y, rect.w, rect.h))
			{
				input->mouse.button.left = 0;
				
				/* deselect things that are already selected; note
				 * that this is only for left-click behavior  */
				if (input->mouse.button.leftclick && n64ctx->highlight_me == tree->data)
				{
					n64ctx->highlight_me = 0;
					global.culler_selected = 0;
				}
				
				/* select things otherwise */
				else
				{
					n64ctx->highlight_me = tree->data;
					if (tree->type == F3DTREE_CULLSPHERE)
						global.culler_selected = tree->data;
					else if(tree->type == F3DTREE_CULLDL)
					{
						global.culler_selected = &global.culler_temp;
						
						/* initialize with settings */
						struct culler *culler = global.culler_selected;
						memset(culler, 0, sizeof(*culler));
						culler->type = F3DTREE_CULLDL;
						culler->data.box.vfirst = tree->raw.culldl.vfirst;
						culler->data.box.vlast  = tree->raw.culldl.vlast;
					}
					else
						global.culler_selected = 0;
				}
			}
		}
		
		/* draw a highlight */
		if (tree->data && n64ctx->highlight_me == tree->data)
		{
			zui_rect(ctx_surf, rect.x, rect.y, rect.w, rect.h, 0xFF202020);
		}
		
		/* display appropriate graphic for clicking */
		if (!tree->child)
			strcat(with_expanding, "  ");
		else if (tree->is_expanded)
			strcat(with_expanding, "- ");
		else
			strcat(with_expanding, "+ ");
		strcat(with_expanding, tree->name);
		
		zuiSprite_debugFont_draw(
			ctx_surf
			, zuifont
			, with_expanding
			, RIGHT_PANE_X + x
			, 0 + y
		);
		if (tree->is_expanded && tree->child)
			scene_print_tree_zui(tree->child);
		tree = tree->next;
	}
	--level;
#endif
}


static
inline
void
scene_preprocess(void)
{
	if (drawmode != DRAWMODE_SCENE)
		return;
	/* in scenes, envcolor usually defaults 80808080 */
	n64ctx->envcolor = (struct vec4){0.5f,0.5f,0.5f,0.5f};
	unsigned char *room = zmap;
	
	/* create tree head */
	n64ctx->f3dtree = f3dtree_new(calloc);
	if (!n64ctx->f3dtree)
		exit(EXIT_FAILURE);
	n64ctx->f3dtree->name = "MAIN";
	
	/* walk header */
	for (room = zmap; *room != 0x14; room += 8)
	{
		if (*room == 0x0A)
		{
			unsigned char *header = f3dzex_ptr(n64ctx, room+4);
			if (!header)
				return;
			/* mesh header types */
			if (*header == 0)
			{
				int count = header[1];
				unsigned char *start = f3dzex_ptr(n64ctx, header+4);
				if (!start)
					return;
				for (int count = header[1]; count; --count)
				{
					struct f3dtree *tree;
					tree = f3dzex_get_tree(n64ctx, f3dzex_ptr(n64ctx, start), calloc);
					if (tree)
						tree->dlofs = getw32(start);
					f3dtree_register(n64ctx, tree);
					tree = f3dzex_get_tree(n64ctx, f3dzex_ptr(n64ctx, start+4), calloc);
					if (tree)
						tree->dlofs = getw32(start+4);
					f3dtree_register(n64ctx, tree);
					start += 8;
				}
			}
			else if (*header == 1)
			{
				fprintf(stderr, "[!] type 1 header unsupported!\n");
				exit(EXIT_FAILURE);
			}
			else if (*header == 2)
			{
				int count = header[1];
				unsigned char *start = f3dzex_ptr(n64ctx, header+4);
				if (!start)
					return;
				for (int count=header[1]; count; --count)
				{
					/* create a tree for the culler */
					struct f3dtree *tree;
					struct f3dtree *child;
					struct culler *culler;
					culler = culler_push_sphere(
						gets16(start)
						, gets16(start+2)
						, gets16(start+4)
						, gets16(start+6)
					);
					tree = f3dtree_new(calloc);
					if (!tree)
						exit(EXIT_FAILURE);
					tree->name = "CULL";
					tree->type = F3DTREE_CULLSPHERE;
					tree->data = culler;
					culler->f3dtree = tree;
					f3dtree_register(n64ctx, tree);
					
					child = f3dzex_get_tree(n64ctx, f3dzex_ptr(n64ctx, start+8), calloc);
					if (child)
					{
						child->dlofs = getw32(start+8);
						f3dtree_addChild(tree, child);
					}
					child = f3dzex_get_tree(n64ctx, f3dzex_ptr(n64ctx, start+12), calloc);
					if (child)
					{
						child->dlofs = getw32(start+12);
						f3dtree_addChild(tree, child);
					}
					start += 16;
				}
			}
			return;
		}
	}
	return;
}


#if 0
static
void
SetTileSize(
	unsigned char *dest
	, int t0, int s0_ul, int t0_ul, int t0_w, int t0_h
	, int t1, int s1_ul, int t1_ul, int t1_w, int t1_h
)
{
	uint32_t _gfx[16] = {0};
	_gfx[0] = 0xE8000000;
	_gfx[1] = 0x00000000;
	_gfx[2] = (s0_ul & 0x7FFU) << 0xC | 0xF2000000 | t0_ul & 0x7FFU;
	_gfx[3] = (uint32_t)((t0 & 7U) << 0x18 | ((s0_ul & 0x7FFU) + (t0_w - 1) * 4 & 0xFFF) << 0xC | (t0_ul & 0x7FFU) + (t0_h - 1) * 4 & 0xFFF);
	_gfx[4] = 0xE8000000;
	_gfx[5] = 0x00000000;
	_gfx[6] = (s1_ul & 0x7FFU) << 0xC | 0xF2000000 | t1_ul & 0x7FFU;
	_gfx[7] = (uint32_t)((t1 & 7U) << 0x18 | ((s1_ul & 0x7FFU) + (t1_w - 1) * 4 & 0xFFF) << 0xC | (t1_ul & 0x7FFU) + (t1_h - 1) * 4 & 0xFFF);
	_gfx[8] = 0xDF000000;
	_gfx[9] = 0x00000000;
	
	for (int i=0; i < sizeof(_gfx) / sizeof(_gfx[0]); ++i)
	{
		uint32_t ok = _gfx[i];
		dest[0] = (ok >> 24);
		dest[1] = (ok >> 16) & 0xFF;
		dest[2] = (ok >>  8) & 0xFF;
		dest[3] = (ok >>  0) & 0xFF;
		
		dest += 4;
	}
}
#endif

static
inline
void
mat44_to_matn64(unsigned char *dest, float src[16])
{
#define    FTOFIX32(x)    (long)((x) * (float)0x00010000)
	int32_t t;
	unsigned char *intpart = dest;
	unsigned char *fracpart = dest + 0x20;
	for (int i=0; i < 4; ++i)
	{
		for (int k=0; k < 4; ++k)
		{
			t = FTOFIX32(src[4*i+k]);
			short ip = (t >> 16) & 0xFFFF;
			intpart[0]  = (ip >> 8) & 255;
			intpart[1]  = (ip >> 0) & 255;
			fracpart[0] = (t >>  8) & 255;
			fracpart[1] = (t >>  0) & 255;
			intpart  += 2;
			fracpart += 2;
		}
	}
}


static
void
display_hex(char *title, unsigned char *data, unsigned int len)
{
	fprintf(stderr, "%s\n", title);
	for (unsigned int i = 0; i < len; i += 8)
	{
		fprintf(stderr, "%08X:", i);
		for (unsigned int k = 0; k < 4; ++k)
			fprintf(stderr, " %02X%02X", data[k*2], data[k*2+1]);
		data += 8;
		fprintf(stderr, "\n");
	}
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


static void draw_scene(void)
{
	float m[16];
	float billboard[16];
	float inverse_mv[16];
	float *im;
	float *s;
	
	if (drawmode == DRAWMODE_SCENE)
		camera_flythrough(m);
	else
		camera_arcball(m);
	
	if (do_billboards_0x01)
	{
		/* write billboarding matrices */
		unsigned char *bbs = n64ctx->segment[0x01];
		unsigned char *bbc = bbs + 0x40;
		
		/* generate spherical billboarding */
		invert4x4(m, inverse_mv);
		im = inverse_mv;
		s = billboard;
		
		/* identity */
		memset(s, 0, sizeof(billboard));
		s[0] = 1;
		s[10] = 1;
		s[15] = 1;
		
		/* billboard vertically */
		s[4] = im[4];
		s[5] = im[5];
		s[6] = im[6];
		s[7] = im[7];
		
		/* billboard horizontally */
		s[0] = im[0];
		s[1] = im[1];
		s[2] = im[2];
		s[3] = im[3];
		
		#if 0
		fprintf(stderr, "right: %f %f %f\n", im[0], im[1], im[2]);
		fprintf(stderr, "up   : %f %f %f\n", im[4], im[5], im[6]);
			fprintf(stderr, "matrix currently:\n");
			fprintf(stderr, "%f %f %f %f\n", im[0], im[4], im[8], im[12]);
			fprintf(stderr, "%f %f %f %f\n", im[1], im[5], im[9], im[13]);
			fprintf(stderr, "%f %f %f %f\n", im[2], im[6], im[10], im[14]);
			fprintf(stderr, "%f %f %f %f\n", im[3], im[7], im[11], im[15]);
		#endif
		
		/* convert to N64 format */
		mat44_to_matn64(bbs, billboard);
		
		/* faster to copy sphere and modify it */
		memcpy(bbc, bbs, 0x40);
		bbc[0x08] = 0; /* x */
		bbc[0x09] = 0;
		
		bbc[0x0A] = 0; /* y */
		bbc[0x0B] = 1;
		
		bbc[0x0C] = 0; /* z */
		bbc[0x0D] = 0;
		
		bbc[0x28] = 0; /* x */
		bbc[0x29] = 0;
		
		bbc[0x2A] = 0; /* y */
		bbc[0x2B] = 0;
		
		bbc[0x2C] = 0; /* z */
		bbc[0x2D] = 0;
	}
	
	//display_hex("sphere", n64ctx->segment[0x01] + 0x00, 0x40);
	//display_hex("cylinder", n64ctx->segment[0x01] + 0x40, 0x40);
	
	/* set modelview matrix to the one we just constructed */
	n64binding->set_modelview(n64binding, m);

#if 0
	ia_begin(&ctx);

	ia_color(&ctx, 1.0f, 0.0f, 0.0f, 1.0f);
	ia_texcoord(&ctx, 0, 0.0f, 1.0f);
	ia_vertex(&ctx, -2.0f, -2.0f, 0.0f, 1.0f);

	ia_color(&ctx, 0.0f, 1.0f, 0.0f, 1.0f);
	ia_texcoord(&ctx, 0, 1.0f, 1.0f);
	ia_vertex(&ctx, 2.0f, -2.0f, 0.0f, 1.0f);

	ia_color(&ctx, 0.0f, 0.0f, 1.0f, 1.0f);
	ia_texcoord(&ctx, 0, 0.5f, 0.0f);
	ia_vertex(&ctx, 0.0f, 2.0f, 0.0f, 1.0f);

	ia_end(&ctx);
#endif
	n64ctx->use_shading = 1;
	
	if (drawmode == DRAWMODE_SCENE)
	{
		/* in scenes, envcolor usually defaults 80808080 */
		n64ctx->envcolor = (struct vec4){0.5f,0.5f,0.5f,0.5f};
		unsigned char *room = zmap;
		for (room = zmap; *room != 0x14; room += 8)
		{
			if (*room == 0x0A)
			{
				unsigned char *header = f3dzex_ptr(n64ctx, room+4);
				if (!header)
					return;
				int zmode_list[] = {N64_ZMODE_OPA, N64_ZMODE_XLU, N64_ZMODE_DEC};
				int zmode_count = sizeof(zmode_list) / sizeof(zmode_list[0]);
				float *depth_ofs = n64binding->depth_ofs(n64binding);
				/* mesh header types */
				if (*header == 0)
				{
					int count = header[1];
					unsigned char *start = f3dzex_ptr(n64ctx, header+4);
					unsigned char *end   = f3dzex_ptr(n64ctx, header+8);
					unsigned char *ostart = start;
					for (int i = 0; i < zmode_count; ++i)
					{
						int zmode = zmode_list[i];
						if (zmode == N64_ZMODE_DEC)
						{
							*depth_ofs = -0.00001;
							n64ctx->force_blending_override = 1;
						}
						else
						{
							*depth_ofs = 0;
							n64ctx->force_blending_override = 0;
						}
						n64ctx->func_walk = f3dzex_walk_zmode;
						n64ctx->only_zmode = zmode;
						start = ostart;
						for (int count = header[1]; count; --count)
						{
							do_dlist(start);
							do_dlist(start+4);
							start += 8;
						}
					}
				}
				else if (*header == 1)
				{
					fprintf(stderr, "[!] type 1 header unsupported!\n");
					exit(EXIT_FAILURE);
				}
				else if (*header == 2)
				{
					int count = header[1];
					unsigned char *start = f3dzex_ptr(n64ctx, header+4);
					unsigned char *end   = f3dzex_ptr(n64ctx, header+8);
					unsigned char *ostart = start;
					for (int i = 0; i < zmode_count; ++i)
					{
						int zmode = zmode_list[i];
						if (zmode == N64_ZMODE_DEC)
						{
							*depth_ofs = -0.00001;
							n64ctx->force_blending_override = 1;
						}
						else
						{
							*depth_ofs = 0;
							n64ctx->force_blending_override = 0;
						}
						n64ctx->func_walk = f3dzex_walk_zmode;
						n64ctx->only_zmode = zmode;
						start = ostart;
						for (int count=header[1]; count; --count)
						{
							do_dlist(start+8);
							do_dlist(start+12);
							start += 16;
						}
					}
				}
				return;
			}
		}
		return;
	}
	
	/* walk the provided display list; an OBJ will be written */
	n64ctx->envcolor = (struct vec4){1,1,1,0.5f};
	
	/* zmode magic */
	n64ctx->func_walk = f3dzex_walk_zmode;
	
	for (unsigned int i = 0; i < DL_list_count; ++i)
	{
		unsigned char temp[0x40];
		struct mat44f *m = n64ctx->matrix.stack+1;
		ofs = DL_list[i].ofs;
#define DIV_100   0.01f
		float x = DL_list[i].x;// * DIV_100;
		float y = DL_list[i].y;// * DIV_100;
		float z = DL_list[i].z;// * DIV_100;
		
		n64ctx->matrix.mat = n64ctx->matrix.stack+1;
		*n64ctx->matrix.mat = mat44f_identity();
		m->w.x += x;
		m->w.y += y;
		m->w.z += z;
		n64ctx->only_zmode = N64_ZMODE_OPA;
		f3dzex_walk_zmode(n64ctx, zobj + ofs);
		
		n64ctx->matrix.mat = n64ctx->matrix.stack+1;
		*n64ctx->matrix.mat = mat44f_identity();
		m->w.x += x;
		m->w.y += y;
		m->w.z += z;
		n64ctx->only_zmode = N64_ZMODE_XLU;
		f3dzex_walk_zmode(n64ctx, zobj + ofs);
		
		n64ctx->matrix.mat = n64ctx->matrix.stack+1;
		*n64ctx->matrix.mat = mat44f_identity();
		m->w.x += x;
		m->w.y += y;
		m->w.z += z;
		n64ctx->only_zmode = N64_ZMODE_DEC;
		f3dzex_walk_zmode(n64ctx, zobj + ofs);
		
		/* old single-pass routine */
		//f3dzex_walk(n64ctx, zobj + ofs);
	}
	return;/**/
	
	/* walk material, then mesh */
	/* XXX: requires `ci4-sphere-grass.zobj` */
	f3dzex_walk(n64ctx, zobj + 0x25B0);
	f3dzex_walk(n64ctx, zobj + 0x13A8);

#if 0
	/* yellow transparent triangle */
	ctx.texture_enable[0] = 0;
	ctx.textures[0] = NULL;
	ctx.flags |= BLEND_ENABLE;

	ia_begin(&ctx);
	ia_color(&ctx, 1.0f, 1.0f, 0.0f, 0.5f);
	ia_vertex(&ctx, 0.0f, -2.0f, -2.0f, 1.0f);
	ia_vertex(&ctx, 0.0f, -2.0f, 2.0f, 1.0f);
	ia_vertex(&ctx, 0.0f, 2.0f, 0.0f, 1.0f);
	ia_end(&ctx);
#endif
}

static
inline
int
get_int(char *str)
{
	int r = -1;
	/* starts with 'x' */
	if (tolower(*str) == 'x')
	{
		if (!sscanf(str + 1, "%X", &r))
			return -1;
	}
	/* starts with 0x */
	else if (tolower(str[1]) == 'x')
	{
		if (!sscanf(str, "%X", &r))
			return -1;
	}
	/* assume decimal */
	else if (!sscanf(str, "%d", &r))
		return -1;
	
	return r;
}

static
inline
time_t
file_loadtime(char *path)
{
	struct stat file_stat;
	if (stat(path, &file_stat))
	{
		fprintf(
			stderr
			, "[!] something went wrong getting stats for '%s'"
			, path
		);
		exit(EXIT_FAILURE);
	}
	return file_stat.st_mtime;
}

int main(int argc, char *argv[])
{
	fprintf(stderr, "welcome to zzviewer-rrw <z64.me>\n");
	if (argc < 3)
		showargs(0);
	
	/* oot/mm */
	f3dzex_select_microcode(N64MICRO_F3DZEX);
	/* banjo kazooie */
	//f3dzex_select_microcode(N64MICRO_F3DEX);
	
	struct {
		char           *fn;
		unsigned char  *data;
		int             offset;
		char            reload;    /* enables reload change test */
		time_t          loadtime;  /* for reloading changes */
	} ramseg[16] = {0};
	int ramseg_count = sizeof(ramseg) / sizeof(ramseg[0]);
	
	for (int i=1; i < argc; ++i)
	{
		char *arg = argv[i];
		if (!memcmp(arg, "-r", 2))
		{
			int seg = get_int(arg + 2);
			char *plus = strchr(arg + 2, '+');
			if (seg < 1 || seg >= ramseg_count)
			{
				fprintf(stderr, "invalid argument '%s'\n", arg);
				exit(EXIT_FAILURE);
			}
			if (i+1 >= argc)
			{
				fprintf(stderr, "'%s': no filename provided\n", arg);
				exit(EXIT_FAILURE);
			}
			ramseg[seg].fn = argv[i+1];
			ramseg[seg].data = file_load(ramseg[seg].fn, &zobj_size);
			if (strchr(arg, 'r'))
			{
				ramseg[seg].reload = 1;
				ramseg[seg].loadtime = file_loadtime(ramseg[seg].fn);
			}
			if (!ramseg[seg].data)
			{
				fprintf(
					stderr
					, "something went wrong loading '%s'\n"
					, ramseg[seg].fn
				);
				exit(EXIT_FAILURE);
			}
			if (plus)
			{
				int offset = get_int(plus+1);
				if (offset < 0)
				{
					fprintf(stderr, "invalid argument '%s'\n", arg);
					exit(EXIT_FAILURE);
				}
				ramseg[seg].offset = offset;
			}
			++i;
		}
		else if (!memcmp(arg, "-z1", 3))
		{
			char *nextDL;
			char *op;
			char *Oop;
			char *ss;
			int dl;
			
			if (i+2 >= argc)
			{
				fprintf(stderr, "'%s': no filename/DL provided\n", arg);
				exit(EXIT_FAILURE);
			}
			zobj = file_load(argv[i+1], &zobj_size);
			if (!zobj)
			{
				fprintf(
					stderr
					, "something went wrong loading '%s'\n"
					, argv[i+1]
				);
				exit(EXIT_FAILURE);
			}
			if (!ramseg[0x06].data)
				ramseg[0x06].data = zobj;
			
			Oop = op = strdup(argv[i+2]);
			if (!op)
			{
				fprintf(stderr, "memory error\n");
				exit(EXIT_FAILURE);
			}
			
			while (op && *op)
			{
				float x = 0;
				float y = 0;
				float z = 0;
				
				/* DLs are separated by | */
				nextDL = strchr(op, '|');
				if (nextDL)
				{
					/* set end of string to 0 and advance next loc */
					*nextDL = '\0';
					nextDL += 1;
				}
				
				/* every substring should begin with a DL */
				dl = get_int(op);
				if (dl < 0)
				{
					fprintf(
						stderr
						, "invalid argument '%s', need display list\n"
						, argv[i+2]
					);
					exit(EXIT_FAILURE);
				}
				
				/* the '(' character starts the (x,y,z) list */
				ss = strchr(op, '(');
				if (ss)
				{
					if (sscanf(ss+1, "%f, %f, %f", &x, &y, &z) != 3)
					{
						fprintf(
							stderr
							, "failed to grab (x,y,z) components from '%s'\n"
							, ss
						);
						exit(EXIT_FAILURE);
					}
				}
				
				/* add to list of DLs */
				if (DL_list_count >= sizeof(DL_list) / sizeof(DL_list[0]))
				{
					fprintf(stderr, "too many DLs specified\n");
					exit(EXIT_FAILURE);
				}
				DL_list[DL_list_count].ofs = dl;
				DL_list[DL_list_count].x    = x;
				DL_list[DL_list_count].y    = y;
				DL_list[DL_list_count].z    = z;
				DL_list_count += 1;
				
				/* advance to next item */
				op = nextDL;
			}
			
			i += 2;
			free(Oop);
		}
		else if (!memcmp(arg, "-z", 2))
		{
			if (i+2 >= argc)
			{
				fprintf(stderr, "'%s': no filename/DL provided\n", arg);
				exit(EXIT_FAILURE);
			}
			zobj = file_load(argv[i+1], &zobj_size);
			if (!zobj)
			{
				fprintf(
					stderr
					, "something went wrong loading '%s'\n"
					, argv[i+1]
				);
				exit(EXIT_FAILURE);
			}
			if (!ramseg[0x06].data)
				ramseg[0x06].data = zobj;
			int dl = get_int(argv[i+2]);
			if (dl < 0)
			{
				fprintf(stderr, "invalid argument '%s', need display list\n", argv[i+2]);
				exit(EXIT_FAILURE);
			}
			ofs = dl;
			i += 2;
				
			/* add to list of DLs */
			if (DL_list_count >= sizeof(DL_list) / sizeof(DL_list[0]))
			{
				fprintf(stderr, "too many DLs specified\n");
				exit(EXIT_FAILURE);
			}
			DL_list[DL_list_count].ofs = ofs;
			DL_list[DL_list_count].x    = 0;
			DL_list[DL_list_count].y    = 0;
			DL_list[DL_list_count].z    = 0;
			DL_list_count += 1;
		}
		else if (!memcmp(arg, "-s", 2))
		{
			if (i+2 >= argc)
			{
				fprintf(stderr, "'%s': no scene/room provided\n", arg);
				exit(EXIT_FAILURE);
			}
			zscene_fn = argv[i+1];
			zscene = file_load(zscene_fn, &zobj_size);
			if (!zscene)
			{
				fprintf(
					stderr
					, "something went wrong loading '%s'\n"
					, zscene_fn
				);
				exit(EXIT_FAILURE);
			}
			zmap_fn = argv[i+2];
			zmap = file_load(zmap_fn, &zobj_size);
			if (!zmap)
			{
				fprintf(
					stderr
					, "something went wrong loading '%s'\n"
					, zmap_fn
				);
				exit(EXIT_FAILURE);
			}
			drawmode = DRAWMODE_SCENE;
			i += 3;
		}
		else
		{
			fprintf(stderr, "unknown argument '%s'\n", arg);
			exit(EXIT_FAILURE);
		}
	}
	
	if ((!zobj && drawmode == DRAWMODE_OBJECT) || (drawmode==DRAWMODE_SCENE && (!zmap || !zscene)) )
	{
		showargs("[!] missing '-z' or '-s' arg!\n");
		exit(EXIT_FAILURE);
	}
	
	/*zobj = file_load(argv[1], &zobj_size);
	if (!zobj)
		showargs();
	
	if (!sscanf(argv[2], "%X", &ofs))
		showargs();*/
	unsigned char* ptr;
	
	/* initialization */
	int window_width  = WIDTH;
	int window_height = HEIGHT;
	
	/* allocate binding structure and get necessary functions */
#if (BINDING == SWRAST)
	n64binding = n64binding_new_swrast(calloc);
#elif (BINDING == RAYLIB)
	n64binding = n64binding_new_raylib(calloc);
#endif
	
	/* make sure it worked */
	if (!n64binding)
	{
		fprintf(stderr, "[!] memory error\n");
		exit(EXIT_FAILURE);
	}
	
	/* tell binding to create window */
	n64binding->init(
		n64binding
		, "zzviewer-rrw"
		, window_width
		, window_height
	);
	
	/* with everything initialized, it is safe to grab pointers
	 * to structures for convenience */
	input = n64binding->input_struct;
	n64ctx = n64binding->n64ctx;
	zuifont = zuiSprite_debugFont();
	ctx_surf = n64binding->ctx_surf;
	
	/*unsigned char *gameplay_keep;
	size_t gameplay_keep_size;
	gameplay_keep = file_load("gameplay_keep.zobj", &gameplay_keep_size);
	if (!gameplay_keep)
	{
		fprintf(stderr, "failed to open `gameplay_keep.zobj`!\n");
		return 1;
	}*/
	
	/* allocate data for ram segment 0x01: two billboarding matrices */
	if (!ramseg[0x01].data)
	{
		do_billboards_0x01 = 1;
		ramseg[0x01].data = malloc(0x40 * 2);
	}
	
	/* set ram segment 0x06 to our ZOBJ */
	for (int i=0; i < ramseg_count; ++i)
	{
		n64ctx->segment[i] = ramseg[i].data + ramseg[i].offset;
	}
	
	/* scene/room segments */
	if ( drawmode == DRAWMODE_SCENE)
	{
		n64ctx->segment[2] = zscene;
		n64ctx->segment[3] = zmap;
		
		ramseg[2].fn = zscene_fn;
		if (ramseg[2].reload)
			ramseg[2].loadtime = file_loadtime(ramseg[2].fn);
		
		ramseg[3].fn = zmap_fn;
		if (ramseg[3].reload)
			ramseg[3].loadtime = file_loadtime(ramseg[3].fn);
		
		scene_preprocess();
		//scene_print_tree(n64ctx->f3dtree);
	}
	/*n64ctx->segment[0x06] = zobj;
	n64ctx->segment[0x04] = gameplay_keep;
	n64ctx->segment[0x08] = zobj;
	n64ctx->segment[0x09] = zobj + 0x4000;*/

	/************* drawing loop *************/
	while (n64binding->input(n64binding))
	{
		/* handle file refresh */
		for (int i=0; i < ramseg_count; ++i)
		{
			if (!ramseg[i].reload)
				continue;
			time_t newtime = file_loadtime(ramseg[i].fn);
			if (newtime > ramseg[i].loadtime)
			{
				ramseg[i].loadtime = newtime;
				persistent_readfile(ramseg[i].fn, ramseg[i].data);
				n64ctx->segment[i] = ramseg[i].data + ramseg[i].offset;
			}
		}
		
		/* clear display */
		n64binding->clear(n64binding, 0x39, 0x39, 0x39, 0xFF);
		
#if 0
		/* scrolling texture test for fun */
		if (n64ctx->segment[8])
		{
			static int timer = 0;
			int s1_ul = timer & 0x7F;
			int t0_ul = (timer * 1) & 0x7F;
			timer += 1;
			SetTileSize(
				n64ctx->segment[8]
				, 0, 0x7F - s1_ul, t0_ul, 32, 32
				, 1, s1_ul, t0_ul, 32, 32
			);
		}
#endif

		single_display_test = 1;
		
		n64ctx->branches_are_DLs = 1;

		draw_scene();
		
//		break;
		
#if 1 //TODO the way this works is being refactored
		/* draw scene explorer thing */
		if (drawmode == DRAWMODE_SCENE)
		{
			scene_boxes();
			
			//zuiSprite_debugFont_draw(ctx_surf, zuifont, "Hello, world!\n""Wow!", RIGHT_PANE_X, 0);
			scene_print_tree_zui(n64ctx->f3dtree);
			
			/* info bar on bottom of right pane */
			zui_rect(
				ctx_surf
				, RIGHT_PANE_X
				, RIGHT_PANE_INFOBAR_Y
				, RIGHT_PANE_W
				, RIGHT_PANE_INFOBAR_H
				, 0xFF202020
			);
			
			if (ui_option(
					"render all branches"
					, &n64ctx->branches_are_DLs
					, RIGHT_PANE_X
					, RIGHT_PANE_INFOBAR_Y
					, 0
				))
			{
				
			}
			
#if 1
			if (ui_option(
					"show pointer cullers"
					, &global.show_pointer_cullers
					, RIGHT_PANE_X
					, RIGHT_PANE_INFOBAR_Y
					, 1
				))
			{
				
			}
			
			if (ui_option(
					"show inline cullers"
					, &global.show_inline_cullers
					, RIGHT_PANE_X
					, RIGHT_PANE_INFOBAR_Y
					, 2
				))
			{
				
			}
#endif
			if (n64ctx->highlight_me)
			{
				struct f3dtree *tree = n64ctx->f3dtree;
				tree = f3dtree_search_data(tree, n64ctx->highlight_me);
				if (tree)
				{
					char ok[64];
					sprintf(ok, "selected data at %08X", tree->dlofs);
					zuiSprite_debugFont_draw(
						ctx_surf
						, zuifont
						, ok
						, RIGHT_PANE_X
						, RIGHT_PANE_INFOBAR_Y + 8 * 3
					);
				}
			}
		}
		else if (drawmode == DRAWMODE_OBJECT)
		{
			
			/* info bar on bottom of right pane */
			zui_rect(
				ctx_surf
				, RIGHT_PANE_X
				, 0
				, RIGHT_PANE_W
				, HEIGHT_VIEW
				, 0xFF202020
			);
			zuiSprite_debugFont_draw(
				ctx_surf
				, zuifont
				, "zzviewer-rrw <z64.me>"
				, RIGHT_PANE_X + 16
				, 16
			);
		}
#endif
		
		/* display result */
		n64binding->show(n64binding);
	}

	/************* cleanup *************/
	free(zobj);
	n64binding->cleanup(n64binding);
	return EXIT_SUCCESS;
}

