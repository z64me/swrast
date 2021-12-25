static void z64FC_c_add_combined(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->combined.x;
	v->y += mat->combined.y;
	v->z += mat->combined.z;
}
static void z64FC_c_sub_combined(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->combined.x;
	v->y -= mat->combined.y;
	v->z -= mat->combined.z;
}
static void z64FC_c_mul_combined(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->combined.x;
	v->y *= mat->combined.y;
	v->z *= mat->combined.z;
}
static void z64FC_c_set_combined(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->combined.x;
	v->y = mat->combined.y;
	v->z = mat->combined.z;
}
static void z64FC_c_add_texel0(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->texel0.x;
	v->y += mat->texel0.y;
	v->z += mat->texel0.z;
}
static void z64FC_c_sub_texel0(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->texel0.x;
	v->y -= mat->texel0.y;
	v->z -= mat->texel0.z;
}
static void z64FC_c_mul_texel0(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->texel0.x;
	v->y *= mat->texel0.y;
	v->z *= mat->texel0.z;
}
static void z64FC_c_set_texel0(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->texel0.x;
	v->y = mat->texel0.y;
	v->z = mat->texel0.z;
}
static void z64FC_c_add_texel1(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->texel1.x;
	v->y += mat->texel1.y;
	v->z += mat->texel1.z;
}
static void z64FC_c_sub_texel1(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->texel1.x;
	v->y -= mat->texel1.y;
	v->z -= mat->texel1.z;
}
static void z64FC_c_mul_texel1(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->texel1.x;
	v->y *= mat->texel1.y;
	v->z *= mat->texel1.z;
}
static void z64FC_c_set_texel1(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->texel1.x;
	v->y = mat->texel1.y;
	v->z = mat->texel1.z;
}
static void z64FC_c_add_primcolor(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->primcolor.x;
	v->y += mat->primcolor.y;
	v->z += mat->primcolor.z;
}
static void z64FC_c_sub_primcolor(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->primcolor.x;
	v->y -= mat->primcolor.y;
	v->z -= mat->primcolor.z;
}
static void z64FC_c_mul_primcolor(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->primcolor.x;
	v->y *= mat->primcolor.y;
	v->z *= mat->primcolor.z;
}
static void z64FC_c_set_primcolor(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->primcolor.x;
	v->y = mat->primcolor.y;
	v->z = mat->primcolor.z;
}
static void z64FC_c_add_vertcolor(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->vertcolor.x;
	v->y += mat->vertcolor.y;
	v->z += mat->vertcolor.z;
}
static void z64FC_c_sub_vertcolor(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->vertcolor.x;
	v->y -= mat->vertcolor.y;
	v->z -= mat->vertcolor.z;
}
static void z64FC_c_mul_vertcolor(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->vertcolor.x;
	v->y *= mat->vertcolor.y;
	v->z *= mat->vertcolor.z;
}
static void z64FC_c_set_vertcolor(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->vertcolor.x;
	v->y = mat->vertcolor.y;
	v->z = mat->vertcolor.z;
}
static void z64FC_c_add_envcolor(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->envcolor.x;
	v->y += mat->envcolor.y;
	v->z += mat->envcolor.z;
}
static void z64FC_c_sub_envcolor(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->envcolor.x;
	v->y -= mat->envcolor.y;
	v->z -= mat->envcolor.z;
}
static void z64FC_c_mul_envcolor(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->envcolor.x;
	v->y *= mat->envcolor.y;
	v->z *= mat->envcolor.z;
}
static void z64FC_c_set_envcolor(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->envcolor.x;
	v->y = mat->envcolor.y;
	v->z = mat->envcolor.z;
}
static void z64FC_c_add_1(struct n64ctx *mat, struct vec4 *v) {
	v->x += 1;
	v->y += 1;
	v->z += 1;
}
static void z64FC_c_sub_1(struct n64ctx *mat, struct vec4 *v) {
	v->x -= 1;
	v->y -= 1;
	v->z -= 1;
}
static void z64FC_c_mul_1(struct n64ctx *mat, struct vec4 *v) {
	v->x *= 1;
	v->y *= 1;
	v->z *= 1;
}
static void z64FC_c_set_1(struct n64ctx *mat, struct vec4 *v) {
	v->x = 1;
	v->y = 1;
	v->z = 1;
}
static void z64FC_c_add_combined_w(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->combined.w;
	v->y += mat->combined.w;
	v->z += mat->combined.w;
}
static void z64FC_c_sub_combined_w(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->combined.w;
	v->y -= mat->combined.w;
	v->z -= mat->combined.w;
}
static void z64FC_c_mul_combined_w(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->combined.w;
	v->y *= mat->combined.w;
	v->z *= mat->combined.w;
}
static void z64FC_c_set_combined_w(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->combined.w;
	v->y = mat->combined.w;
	v->z = mat->combined.w;
}
static void z64FC_c_add_texel0_w(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->texel0.w;
	v->y += mat->texel0.w;
	v->z += mat->texel0.w;
}
static void z64FC_c_sub_texel0_w(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->texel0.w;
	v->y -= mat->texel0.w;
	v->z -= mat->texel0.w;
}
static void z64FC_c_mul_texel0_w(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->texel0.w;
	v->y *= mat->texel0.w;
	v->z *= mat->texel0.w;
}
static void z64FC_c_set_texel0_w(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->texel0.w;
	v->y = mat->texel0.w;
	v->z = mat->texel0.w;
}
static void z64FC_c_add_texel1_w(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->texel1.w;
	v->y += mat->texel1.w;
	v->z += mat->texel1.w;
}
static void z64FC_c_sub_texel1_w(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->texel1.w;
	v->y -= mat->texel1.w;
	v->z -= mat->texel1.w;
}
static void z64FC_c_mul_texel1_w(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->texel1.w;
	v->y *= mat->texel1.w;
	v->z *= mat->texel1.w;
}
static void z64FC_c_set_texel1_w(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->texel1.w;
	v->y = mat->texel1.w;
	v->z = mat->texel1.w;
}
static void z64FC_c_add_primcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->primcolor.w;
	v->y += mat->primcolor.w;
	v->z += mat->primcolor.w;
}
static void z64FC_c_sub_primcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->primcolor.w;
	v->y -= mat->primcolor.w;
	v->z -= mat->primcolor.w;
}
static void z64FC_c_mul_primcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->primcolor.w;
	v->y *= mat->primcolor.w;
	v->z *= mat->primcolor.w;
}
static void z64FC_c_set_primcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->primcolor.w;
	v->y = mat->primcolor.w;
	v->z = mat->primcolor.w;
}
static void z64FC_c_add_vertcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->vertcolor.w;
	v->y += mat->vertcolor.w;
	v->z += mat->vertcolor.w;
}
static void z64FC_c_sub_vertcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->vertcolor.w;
	v->y -= mat->vertcolor.w;
	v->z -= mat->vertcolor.w;
}
static void z64FC_c_mul_vertcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->vertcolor.w;
	v->y *= mat->vertcolor.w;
	v->z *= mat->vertcolor.w;
}
static void z64FC_c_set_vertcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->vertcolor.w;
	v->y = mat->vertcolor.w;
	v->z = mat->vertcolor.w;
}
static void z64FC_c_add_envcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->envcolor.w;
	v->y += mat->envcolor.w;
	v->z += mat->envcolor.w;
}
static void z64FC_c_sub_envcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->envcolor.w;
	v->y -= mat->envcolor.w;
	v->z -= mat->envcolor.w;
}
static void z64FC_c_mul_envcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->envcolor.w;
	v->y *= mat->envcolor.w;
	v->z *= mat->envcolor.w;
}
static void z64FC_c_set_envcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->envcolor.w;
	v->y = mat->envcolor.w;
	v->z = mat->envcolor.w;
}
static void z64FC_c_add_lodfrac(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->lodfrac;
	v->y += mat->lodfrac;
	v->z += mat->lodfrac;
}
static void z64FC_c_sub_lodfrac(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->lodfrac;
	v->y -= mat->lodfrac;
	v->z -= mat->lodfrac;
}
static void z64FC_c_mul_lodfrac(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->lodfrac;
	v->y *= mat->lodfrac;
	v->z *= mat->lodfrac;
}
static void z64FC_c_set_lodfrac(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->lodfrac;
	v->y = mat->lodfrac;
	v->z = mat->lodfrac;
}
static void z64FC_c_add_primLodfrac(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->primLodfrac;
	v->y += mat->primLodfrac;
	v->z += mat->primLodfrac;
}
static void z64FC_c_sub_primLodfrac(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->primLodfrac;
	v->y -= mat->primLodfrac;
	v->z -= mat->primLodfrac;
}
static void z64FC_c_mul_primLodfrac(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->primLodfrac;
	v->y *= mat->primLodfrac;
	v->z *= mat->primLodfrac;
}
static void z64FC_c_set_primLodfrac(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->primLodfrac;
	v->y = mat->primLodfrac;
	v->z = mat->primLodfrac;
}
static void z64FC_c_add_k5(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->k5;
	v->y += mat->k5;
	v->z += mat->k5;
}
static void z64FC_c_sub_k5(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->k5;
	v->y -= mat->k5;
	v->z -= mat->k5;
}
static void z64FC_c_mul_k5(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->k5;
	v->y *= mat->k5;
	v->z *= mat->k5;
}
static void z64FC_c_set_k5(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->k5;
	v->y = mat->k5;
	v->z = mat->k5;
}
static void z64FC_c_add_0(struct n64ctx *mat, struct vec4 *v) {
	v->x += 0;
	v->y += 0;
	v->z += 0;
}
static void z64FC_c_sub_0(struct n64ctx *mat, struct vec4 *v) {
	v->x -= 0;
	v->y -= 0;
	v->z -= 0;
}
static void z64FC_c_mul_0(struct n64ctx *mat, struct vec4 *v) {
	v->x *= 0;
	v->y *= 0;
	v->z *= 0;
}
static void z64FC_c_set_0(struct n64ctx *mat, struct vec4 *v) {
	v->x = 0;
	v->y = 0;
	v->z = 0;
}
static void z64FC_c_add_center(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->center.x;
	v->y += mat->center.y;
	v->z += mat->center.z;
}
static void z64FC_c_sub_center(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->center.x;
	v->y -= mat->center.y;
	v->z -= mat->center.z;
}
static void z64FC_c_mul_center(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->center.x;
	v->y *= mat->center.y;
	v->z *= mat->center.z;
}
static void z64FC_c_set_center(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->center.x;
	v->y = mat->center.y;
	v->z = mat->center.z;
}
static void z64FC_c_add_scale(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->scale.x;
	v->y += mat->scale.y;
	v->z += mat->scale.z;
}
static void z64FC_c_sub_scale(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->scale.x;
	v->y -= mat->scale.y;
	v->z -= mat->scale.z;
}
static void z64FC_c_mul_scale(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->scale.x;
	v->y *= mat->scale.y;
	v->z *= mat->scale.z;
}
static void z64FC_c_set_scale(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->scale.x;
	v->y = mat->scale.y;
	v->z = mat->scale.z;
}
static void z64FC_c_add_noise(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->noise.x;
	v->y += mat->noise.y;
	v->z += mat->noise.z;
}
static void z64FC_c_sub_noise(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->noise.x;
	v->y -= mat->noise.y;
	v->z -= mat->noise.z;
}
static void z64FC_c_mul_noise(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->noise.x;
	v->y *= mat->noise.y;
	v->z *= mat->noise.z;
}
static void z64FC_c_set_noise(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->noise.x;
	v->y = mat->noise.y;
	v->z = mat->noise.z;
}
static void z64FC_c_add_k4(struct n64ctx *mat, struct vec4 *v) {
	v->x += mat->k4;
	v->y += mat->k4;
	v->z += mat->k4;
}
static void z64FC_c_sub_k4(struct n64ctx *mat, struct vec4 *v) {
	v->x -= mat->k4;
	v->y -= mat->k4;
	v->z -= mat->k4;
}
static void z64FC_c_mul_k4(struct n64ctx *mat, struct vec4 *v) {
	v->x *= mat->k4;
	v->y *= mat->k4;
	v->z *= mat->k4;
}
static void z64FC_c_set_k4(struct n64ctx *mat, struct vec4 *v) {
	v->x = mat->k4;
	v->y = mat->k4;
	v->z = mat->k4;
}
static void z64FC_a_add_combined_w(struct n64ctx *mat, struct vec4 *v) {
	v->w += mat->combined.w;
}
static void z64FC_a_sub_combined_w(struct n64ctx *mat, struct vec4 *v) {
	v->w -= mat->combined.w;
}
static void z64FC_a_mul_combined_w(struct n64ctx *mat, struct vec4 *v) {
	v->w *= mat->combined.w;
}
static void z64FC_a_set_combined_w(struct n64ctx *mat, struct vec4 *v) {
	v->w = mat->combined.w;
}
static void z64FC_a_add_texel0_w(struct n64ctx *mat, struct vec4 *v) {
	v->w += mat->texel0.w;
}
static void z64FC_a_sub_texel0_w(struct n64ctx *mat, struct vec4 *v) {
	v->w -= mat->texel0.w;
}
static void z64FC_a_mul_texel0_w(struct n64ctx *mat, struct vec4 *v) {
	v->w *= mat->texel0.w;
}
static void z64FC_a_set_texel0_w(struct n64ctx *mat, struct vec4 *v) {
	v->w = mat->texel0.w;
}
static void z64FC_a_add_texel1_w(struct n64ctx *mat, struct vec4 *v) {
	v->w += mat->texel1.w;
}
static void z64FC_a_sub_texel1_w(struct n64ctx *mat, struct vec4 *v) {
	v->w -= mat->texel1.w;
}
static void z64FC_a_mul_texel1_w(struct n64ctx *mat, struct vec4 *v) {
	v->w *= mat->texel1.w;
}
static void z64FC_a_set_texel1_w(struct n64ctx *mat, struct vec4 *v) {
	v->w = mat->texel1.w;
}
static void z64FC_a_add_primcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->w += mat->primcolor.w;
}
static void z64FC_a_sub_primcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->w -= mat->primcolor.w;
}
static void z64FC_a_mul_primcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->w *= mat->primcolor.w;
}
static void z64FC_a_set_primcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->w = mat->primcolor.w;
}
static void z64FC_a_add_vertcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->w += mat->vertcolor.w;
}
static void z64FC_a_sub_vertcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->w -= mat->vertcolor.w;
}
static void z64FC_a_mul_vertcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->w *= mat->vertcolor.w;
}
static void z64FC_a_set_vertcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->w = mat->vertcolor.w;
}
static void z64FC_a_add_envcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->w += mat->envcolor.w;
}
static void z64FC_a_sub_envcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->w -= mat->envcolor.w;
}
static void z64FC_a_mul_envcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->w *= mat->envcolor.w;
}
static void z64FC_a_set_envcolor_w(struct n64ctx *mat, struct vec4 *v) {
	v->w = mat->envcolor.w;
}
static void z64FC_a_add_1(struct n64ctx *mat, struct vec4 *v) {
	v->w += 1;
}
static void z64FC_a_sub_1(struct n64ctx *mat, struct vec4 *v) {
	v->w -= 1;
}
static void z64FC_a_mul_1(struct n64ctx *mat, struct vec4 *v) {
	v->w *= 1;
}
static void z64FC_a_set_1(struct n64ctx *mat, struct vec4 *v) {
	v->w = 1;
}
static void z64FC_a_add_0(struct n64ctx *mat, struct vec4 *v) {
	v->w += 0;
}
static void z64FC_a_sub_0(struct n64ctx *mat, struct vec4 *v) {
	v->w -= 0;
}
static void z64FC_a_mul_0(struct n64ctx *mat, struct vec4 *v) {
	v->w *= 0;
}
static void z64FC_a_set_0(struct n64ctx *mat, struct vec4 *v) {
	v->w = 0;
}
static void z64FC_a_add_lodfrac(struct n64ctx *mat, struct vec4 *v) {
	v->w += mat->lodfrac;
}
static void z64FC_a_sub_lodfrac(struct n64ctx *mat, struct vec4 *v) {
	v->w -= mat->lodfrac;
}
static void z64FC_a_mul_lodfrac(struct n64ctx *mat, struct vec4 *v) {
	v->w *= mat->lodfrac;
}
static void z64FC_a_set_lodfrac(struct n64ctx *mat, struct vec4 *v) {
	v->w = mat->lodfrac;
}
static void z64FC_a_add_primLodfrac(struct n64ctx *mat, struct vec4 *v) {
	v->w += mat->primLodfrac;
}
static void z64FC_a_sub_primLodfrac(struct n64ctx *mat, struct vec4 *v) {
	v->w -= mat->primLodfrac;
}
static void z64FC_a_mul_primLodfrac(struct n64ctx *mat, struct vec4 *v) {
	v->w *= mat->primLodfrac;
}
static void z64FC_a_set_primLodfrac(struct n64ctx *mat, struct vec4 *v) {
	v->w = mat->primLodfrac;
}
void (*z64FC_c_oplist_add[])(Z64OPARGS) = {
	[0] = z64FC_c_add_combined
	, [1] = z64FC_c_add_texel0
	, [2] = z64FC_c_add_texel1
	, [3] = z64FC_c_add_primcolor
	, [4] = z64FC_c_add_vertcolor
	, [5] = z64FC_c_add_envcolor
	, [6] = z64FC_c_add_1
	, [7] = z64FC_c_add_combined_w
	, [8] = z64FC_c_add_texel0_w
	, [9] = z64FC_c_add_texel1_w
	, [10] = z64FC_c_add_primcolor_w
	, [11] = z64FC_c_add_vertcolor_w
	, [12] = z64FC_c_add_envcolor_w
	, [13] = z64FC_c_add_lodfrac
	, [14] = z64FC_c_add_primLodfrac
	, [15] = z64FC_c_add_k5
	, [31] = z64FC_c_add_0
	, [32] = z64FC_c_add_center
	, [33] = z64FC_c_add_scale
	, [34] = z64FC_c_add_noise
	, [35] = z64FC_c_add_k4
};
void (*z64FC_c_oplist_sub[])(Z64OPARGS) = {
	[0] = z64FC_c_sub_combined
	, [1] = z64FC_c_sub_texel0
	, [2] = z64FC_c_sub_texel1
	, [3] = z64FC_c_sub_primcolor
	, [4] = z64FC_c_sub_vertcolor
	, [5] = z64FC_c_sub_envcolor
	, [6] = z64FC_c_sub_1
	, [7] = z64FC_c_sub_combined_w
	, [8] = z64FC_c_sub_texel0_w
	, [9] = z64FC_c_sub_texel1_w
	, [10] = z64FC_c_sub_primcolor_w
	, [11] = z64FC_c_sub_vertcolor_w
	, [12] = z64FC_c_sub_envcolor_w
	, [13] = z64FC_c_sub_lodfrac
	, [14] = z64FC_c_sub_primLodfrac
	, [15] = z64FC_c_sub_k5
	, [31] = z64FC_c_sub_0
	, [32] = z64FC_c_sub_center
	, [33] = z64FC_c_sub_scale
	, [34] = z64FC_c_sub_noise
	, [35] = z64FC_c_sub_k4
};
void (*z64FC_c_oplist_mul[])(Z64OPARGS) = {
	[0] = z64FC_c_mul_combined
	, [1] = z64FC_c_mul_texel0
	, [2] = z64FC_c_mul_texel1
	, [3] = z64FC_c_mul_primcolor
	, [4] = z64FC_c_mul_vertcolor
	, [5] = z64FC_c_mul_envcolor
	, [6] = z64FC_c_mul_1
	, [7] = z64FC_c_mul_combined_w
	, [8] = z64FC_c_mul_texel0_w
	, [9] = z64FC_c_mul_texel1_w
	, [10] = z64FC_c_mul_primcolor_w
	, [11] = z64FC_c_mul_vertcolor_w
	, [12] = z64FC_c_mul_envcolor_w
	, [13] = z64FC_c_mul_lodfrac
	, [14] = z64FC_c_mul_primLodfrac
	, [15] = z64FC_c_mul_k5
	, [31] = z64FC_c_mul_0
	, [32] = z64FC_c_mul_center
	, [33] = z64FC_c_mul_scale
	, [34] = z64FC_c_mul_noise
	, [35] = z64FC_c_mul_k4
};
void (*z64FC_c_oplist_set[])(Z64OPARGS) = {
	[0] = z64FC_c_set_combined
	, [1] = z64FC_c_set_texel0
	, [2] = z64FC_c_set_texel1
	, [3] = z64FC_c_set_primcolor
	, [4] = z64FC_c_set_vertcolor
	, [5] = z64FC_c_set_envcolor
	, [6] = z64FC_c_set_1
	, [7] = z64FC_c_set_combined_w
	, [8] = z64FC_c_set_texel0_w
	, [9] = z64FC_c_set_texel1_w
	, [10] = z64FC_c_set_primcolor_w
	, [11] = z64FC_c_set_vertcolor_w
	, [12] = z64FC_c_set_envcolor_w
	, [13] = z64FC_c_set_lodfrac
	, [14] = z64FC_c_set_primLodfrac
	, [15] = z64FC_c_set_k5
	, [31] = z64FC_c_set_0
	, [32] = z64FC_c_set_center
	, [33] = z64FC_c_set_scale
	, [34] = z64FC_c_set_noise
	, [35] = z64FC_c_set_k4
};
void (*z64FC_a_oplist_add[])(Z64OPARGS) = {
	[0] = z64FC_a_add_combined_w
	, [1] = z64FC_a_add_texel0_w
	, [2] = z64FC_a_add_texel1_w
	, [3] = z64FC_a_add_primcolor_w
	, [4] = z64FC_a_add_vertcolor_w
	, [5] = z64FC_a_add_envcolor_w
	, [6] = z64FC_a_add_1
	, [7] = z64FC_a_add_0
	, [8] = z64FC_a_add_lodfrac
	, [9] = z64FC_a_add_primLodfrac
};
void (*z64FC_a_oplist_sub[])(Z64OPARGS) = {
	[0] = z64FC_a_sub_combined_w
	, [1] = z64FC_a_sub_texel0_w
	, [2] = z64FC_a_sub_texel1_w
	, [3] = z64FC_a_sub_primcolor_w
	, [4] = z64FC_a_sub_vertcolor_w
	, [5] = z64FC_a_sub_envcolor_w
	, [6] = z64FC_a_sub_1
	, [7] = z64FC_a_sub_0
	, [8] = z64FC_a_sub_lodfrac
	, [9] = z64FC_a_sub_primLodfrac
};
void (*z64FC_a_oplist_mul[])(Z64OPARGS) = {
	[0] = z64FC_a_mul_combined_w
	, [1] = z64FC_a_mul_texel0_w
	, [2] = z64FC_a_mul_texel1_w
	, [3] = z64FC_a_mul_primcolor_w
	, [4] = z64FC_a_mul_vertcolor_w
	, [5] = z64FC_a_mul_envcolor_w
	, [6] = z64FC_a_mul_1
	, [7] = z64FC_a_mul_0
	, [8] = z64FC_a_mul_lodfrac
	, [9] = z64FC_a_mul_primLodfrac
};
void (*z64FC_a_oplist_set[])(Z64OPARGS) = {
	[0] = z64FC_a_set_combined_w
	, [1] = z64FC_a_set_texel0_w
	, [2] = z64FC_a_set_texel1_w
	, [3] = z64FC_a_set_primcolor_w
	, [4] = z64FC_a_set_vertcolor_w
	, [5] = z64FC_a_set_envcolor_w
	, [6] = z64FC_a_set_1
	, [7] = z64FC_a_set_0
	, [8] = z64FC_a_set_lodfrac
	, [9] = z64FC_a_set_primLodfrac
};
