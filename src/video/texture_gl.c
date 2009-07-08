/* 
 * texture_gl.c
 * Texture OpenGL implementation
 */

#include <stdio.h>

#include <common/defs.h>
#include <common/exception.h>
#include <common/mm.h>
#include <common/geometry.h>

#include <resource/bitmap.h>

#include <video/texture.h>
#include <video/texture_gl.h>

#include <common/utils.h>
#include <math/matrix.h>

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#endif

#ifdef VIDEO_OPENGL_ENGINE

#define FREE_HWMEM(n)	do {} while(0)
#define ALLOC_HWMEM(n)	do {} while(0)

#define free_hwtexs(tex)	do {	\
	if (tex->hwtexs != tex->_hwtexs_save)	\
		GC_FREE_BLOCK_SET(tex->hwtexs);		\
} while (0)

static void
texture_gl_cleanup(struct cleanup * str);

static inline struct texture_gl *
alloc_texture_gl(void)
{
	struct texture_gl * tex;
	tex = gc_calloc(sizeof(*tex),
			offsetof(struct texture_gl, base.__gc_tag),
			0,
			NULL);
	TRACE(MEMORY, "alloc result: %p\n", tex);
	INIT_LIST_HEAD(&tex->list);
	return tex;
}

static inline void
dealloc_texture_gl(struct texture_gl * tex)
{
	TRACE(MEMORY, "dealloc %p\n", tex);
	gc_free(&tex->base.__gc_tag);
	return;
}

static void
free_phy_bitmap(struct texture_gl * tex)
{
	if (tex->phy_bitmap == NULL)
		return;
	TRACE(OPENGL, "tex %p free phy_bitmap\n", tex);
	if ((TEXGL_BITMAP(tex))
			&&
			(tex->phy_bitmap == TEXGL_BITMAP(tex)->data))
		return;
	TRACE(OPENGL, "\tfree phy_bitmap\n");
	GC_FREE_BLOCK_SET(tex->phy_bitmap);
}

static void
release_bitmap(struct texture_gl * tex)
{
	if (TEXGL_BITMAP(tex)) {
		TRACE(OPENGL, "tex %p release bitmap\n", tex);
		if (tex->phy_bitmap == TEXGL_BITMAP(tex)->data)
			tex->phy_bitmap = NULL;
		PUT_BITMAP(TEXGL_BITMAP(tex));
		SET_TEXGL_BITMAP(tex, NULL);
	}
}

static void
load_bitmap(struct texture_gl * tex)
{
	struct bitmap * b;
	if (TEXGL_BITMAP(tex) != NULL) {
		if (TEXGL_DUMMY_BITMAP(tex) == NULL) {
			b = TEXGL_BITMAP(tex);
			memcpy(&tex->__dummy_bitmap, b, sizeof(*b));
			tex->dummy_bitmap = &tex->__dummy_bitmap;
		}
		return;
	}
	b = GET_BITMAP(tex->base.bitmap_res_id);
	assert(b != NULL);
	SET_TEXGL_BITMAP(tex, b);
	GRAB_BITMAP(b);

	/* save bitmap layout */
	memcpy(&tex->__dummy_bitmap, b, sizeof(*b));
	tex->dummy_bitmap = &tex->__dummy_bitmap;

	switch (tex->dummy_bitmap->format) {
		case BITMAP_RGB:
			tex->tex_data_format = GL_RGB;
			break;
		case BITMAP_RGBA:
			tex->tex_data_format = GL_RGBA;
			break;
		case BITMAP_LUMINANCE:
			tex->tex_data_format = GL_LUMINANCE;
			break;
		case BITMAP_LUMINANCE_ALPHA:
			tex->tex_data_format = GL_LUMINANCE_ALPHA;
			break;
		default:
			FATAL(OPENGL, "Unknown texture format here: %d\n",
					TEXGL_DUMMY_BITMAP(tex)->format);
			THROW(EXCEPTION_FATAL,
					"Unknown texture format: 0x%x\n", TEXGL_DUMMY_BITMAP(tex)->format);
	}
}

static void
free_hwmem(struct texture_gl * tex)
{
	if (tex->hwtexs) {
		TRACE(OPENGL, "tex %p delete hw texture\n", tex);
		glDeleteTextures(tex->nr_hwtexs, tex->hwtexs);
		GL_POP_ERROR();
		
		free_hwtexs(tex);
		
		FREE_HWMEM(tex->occupied_hwmem);
	}
}

static void
texture_gl_cleanup(struct cleanup * c)
{
	struct texture_gl * tex;

	remove_cleanup(c);
	tex = container_of(c,
			struct texture_gl, base.cleanup);

	TRACE(OPENGL, "cleanup gl texture %p\n", tex);

	free_hwmem(tex);
	release_bitmap(tex);
	free_phy_bitmap(tex);

	list_del(&tex->list);

	tex_common_cleanup(&tex->base);

	dealloc_texture_gl(tex);
}

static void
init_texgl(struct texture_gl * tex)
{
	struct bitmap * b;
	struct rectangle * rect = &tex->base.rect;

	TRACE(OPENGL, "Init texgl %p from bitmap %llu\n",
			tex, tex->base.bitmap_res_id);

	load_bitmap(tex);

	b = tex->base.bitmap;
	assert(b != NULL);
	TRACE(OPENGL, "bitmap size: (%d x %d)\n", b->w, b->h);

	/* convert the rect */
	struct rectangle full_rect = {
		0, 0, b->w, b->h,
	};
	*rect = geom_rect_cover(full_rect, *rect);
	TRACE(OPENGL, "bounded rect:" RECT_FMT "\n", RECT_ARG(rect));

	/* how many hw texture we need? */
	int nw, nh;
	nw = (rect->w + gl_max_texture_size() - 1) / gl_max_texture_size();
	nh = (rect->h + gl_max_texture_size() - 1) / gl_max_texture_size();

	tex->nr_hwtexs = nw * nh;
	assert(tex->nr_hwtexs > 0);
	TRACE(OPENGL, "tex %p should be split into %d (%d x %d) pieces\n",
			tex, tex->nr_hwtexs, nw, nh);
	tex->nw = nw;
	tex->nh = nh;

	/* Can this texture use bitmap's data field directly? */
	if ((nw * nh == 1) && (RECT_SAME(rect, &full_rect)))
		tex->use_bitmap_data = TRUE;
	else
		tex->use_bitmap_data = FALSE;

	/* can this texture use NPOT or RECT? */
	int potw, poth;
	potw = pow2roundup(rect->w);
	poth = pow2roundup(rect->h);
	if (nw * nh == 1) {
		if ((rect->w == potw) && (rect->h == poth)) {
			tex->internal_type = TEXGL_NORMAL;
			tex->texgl_target = GL_TEXTURE_2D;
			tex->tile_w = rect->w;
			tex->tile_h = rect->h;
			TRACE(OPENGL,
					"tex %p use NORMAL (%d x %d)\n",
					tex, tex->tile_w, tex->tile_h);
		} else {
			if (gl_tex_NPOT_enabled()) {
				tex->internal_type = TEXGL_NPOT;
				tex->texgl_target = GL_TEXTURE_2D;
				tex->tile_w = rect->w;
				tex->tile_h = rect->h;
				TRACE(OPENGL,
						"tex %p use NPOT (%d x %d)\n",
						tex, tex->tile_w, tex->tile_h);
			} else if (gl_tex_RECT_enabled()) {
				tex->internal_type = TEXGL_RECT;
				tex->texgl_target = GL_TEXTURE_RECTANGLE;
				tex->tile_w = rect->w;
				tex->tile_h = rect->h;
				TRACE(OPENGL,
						"tex %p use RECT (%d x %d)\n",
						tex, tex->tile_w, tex->tile_h);
			} else {
				tex->internal_type = TEXGL_NORMAL;
				tex->texgl_target = GL_TEXTURE_2D;
				tex->tile_w = potw;
				tex->tile_h = poth;
				TRACE(OPENGL,
						"tex %p use NORMAL (%d x %d)\n",
						tex, tex->tile_w, tex->tile_h);
			}
		}
	} else {
		tex->internal_type = TEXGL_NORMAL;
		tex->texgl_target = GL_TEXTURE_2D;
		tex->tile_w = rect->w > gl_max_texture_size() ?
			gl_max_texture_size() : potw;
		tex->tile_h = rect->h > gl_max_texture_size() ?
			gl_max_texture_size() : poth;
		TRACE(OPENGL,
				"tex %p use NORMAL (%d x %d)\n",
				tex, tex->tile_w, tex->tile_h);
	}
	return;
}


/* give the tile number(x, y), return:
 * the start ptr in bitmap's data */
/* before the calling of tile_to_data, caller must guarantee
 * the bitmap has been loaded. */
static inline uint8_t *
tile_to_data(struct texture_gl * tex, int x, int y)
{
	struct bitmap * b = TEXGL_BITMAP(tex);
	struct rectangle * rect = &tex->base.rect;
	assert(b != NULL);

	int ox, oy;
	ox = rect->x;
	oy = rect->y;

	ox += x * tex->tile_w;
	oy += y * tex->tile_h;

	return b->data + (oy * b->w + ox) *
		bitmap_bytes_pre_pixel(b);
}

#define get_tile_width(tex, x, y) \
	(((x) < (tex)->nw - 1) ? (tex)->tile_w : ((tex)->base.rect.w - ((tex)->nw - 1) * (tex)->tile_w))
#define get_tile_height(tex, x, y) \
	(((y) < (tex)->nh - 1) ? (tex)->tile_h : ((tex)->base.rect.h - ((tex)->nh - 1) * (tex)->tile_h))

#define tile_bytes_pre_line(tex) (bitmap_bytes_pre_line(TEXGL_DUMMY_BITMAP(tex)) * (tex)->tile_h)

#define normal_tile_size(tex, y)	((tex)->tile_w * get_tile_height(tex, 0, y)) \
	* bitmap_bytes_pre_pixel(TEXGL_DUMMY_BITMAP(tex))

/* give the tex number(x, y), return:
 * the start ptr in bitmap's data */
static inline uint8_t *
tile_to_phydata(struct texture_gl * tex, int x, int y)
{
	return tex->phy_bitmap + y * tile_bytes_pre_line(tex) +
		x * (normal_tile_size(tex, y));
}


/* 
 * after build_phybitmap, bitmap is released, if not use bitmap data
 */
static void
build_phybitmap(struct texture_gl * tex)
{
	if (tex->phy_bitmap != NULL)
		return;

	load_bitmap(tex);
	if (tex->use_bitmap_data) {
		tex->phy_bitmap = TEXGL_BITMAP(tex)->data;
		/* XXX don't release bitmap */
		return;
	}

	tex->phy_bitmap = GC_CALLOC_BLOCK(tex->base.rect.w
			* tex->base.rect.h
			* bitmap_bytes_pre_pixel(TEXGL_DUMMY_BITMAP(tex)));
	/* rearrange bitmap */
	int x, y;
	for (y = 0; y < tex->nh; y ++) {
		for (x = 0; x < tex->nw; x++) {
			/* XXX */
			uint8_t * src, * dest;
			src = tile_to_data(tex, x, y);
			dest = tile_to_phydata(tex, x, y);
			int i, w, h;
			w = get_tile_width(tex, x, y);
			h = get_tile_height(tex, x, y);
			for (i = 0; i < h; i++) {
				int len = w * bitmap_bytes_pre_pixel(TEXGL_DUMMY_BITMAP(tex));
				memcpy(dest, src, len);
				src += bitmap_bytes_pre_line(TEXGL_DUMMY_BITMAP(tex));
				dest += len;
			}
		}
	}
}

static void
load_hwtexs(struct texture_gl * tex)
{
	int x, y;
	int nr = 0;

	build_phybitmap(tex);

	if (tex->hwtexs == NULL) {
		if (tex->nr_hwtexs < NR_HWTEX_LMT)
			tex->hwtexs = tex->_hwtexs_save;
		else
			tex->hwtexs = GC_CALLOC_BLOCK(sizeof(GLuint) * tex->nr_hwtexs);

		/* generate texture objects */
		glGenTextures(tex->nr_hwtexs, tex->hwtexs);
		TRY_CLEANUP(gl_check_error, free_hwtexs(tex));
	}

	TRACE(OPENGL, "Begin to load tex %p into hw memory\n",
			tex);

	if (tex->gl_params.target == GL_TEXTURE_3D) {
		WARNING(OPENGL, "3D texture hasn't implemented\n");
		return;
	}

	/* create the texture */
	GLint internalformat;
	GLenum format;
	GLenum type = GL_UNSIGNED_BYTE;

	bool_t use_compress =
		gl_tex_compression_enabled() && (tex->internal_type != TEXGL_RECT);

	if (use_compress)
		internalformat = GL_COMPRESSED_RGBA;
	else
		internalformat = GL_RGBA;

	format = tex->tex_data_format;

	GLenum target;

	if (tex->internal_type == TEXGL_RECT)
		target = GL_TEXTURE_RECTANGLE;
	else
		target = tex->gl_params.target;

	for (y = 0; y < tex->nh; y ++) {
		for (x = 0; x < tex->nw; x++) {
			uint8_t * data;

			data = tile_to_phydata(tex, x, y);
			glBindTexture(target, tex->hwtexs[nr]);
			TRACE(OPENGL, "tex %p: start load hwtex %d from phy data %p\n",
					tex, nr, data);
			
			if (target != GL_TEXTURE_RECTANGLE) {
				glTexParameteri(target, GL_TEXTURE_WRAP_S,
						tex->gl_params.wrap_s);
				if ((target == GL_TEXTURE_2D) || (target == GL_TEXTURE_3D))
					glTexParameteri(target, GL_TEXTURE_WRAP_T,
							tex->gl_params.wrap_t);
				if (target == GL_TEXTURE_3D)
					glTexParameteri(target, GL_TEXTURE_WRAP_R,
							tex->gl_params.wrap_r);
				glTexParameteri(target, GL_TEXTURE_MAG_FILTER,
						tex->gl_params.mag_filter);
				glTexParameteri(target, GL_TEXTURE_MIN_FILTER,
						tex->gl_params.min_filter);
			} else {
				GLenum ws = tex->gl_params.wrap_s;
				GLenum wt = tex->gl_params.wrap_t;
				GLenum maf = tex->gl_params.mag_filter;
				GLenum mif = tex->gl_params.min_filter;

				if ((ws == GL_REPEAT) || (ws == GL_MIRRORED_REPEAT))
					ws = GL_CLAMP_TO_EDGE;
				if ((wt == GL_REPEAT) || (wt == GL_MIRRORED_REPEAT))
					wt = GL_CLAMP_TO_EDGE;
				if ((maf != GL_NEAREST) && (maf != GL_LINEAR))
					maf = GL_LINEAR;
				if ((mif != GL_NEAREST) && (mif != GL_LINEAR))
					mif = GL_LINEAR;

				tex->gl_params.wrap_s = ws;
				tex->gl_params.wrap_t = wt;
				tex->gl_params.mag_filter = maf;
				tex->gl_params.min_filter = mif;

				glTexParameteri(target, GL_TEXTURE_WRAP_S, ws);
				glTexParameteri(target, GL_TEXTURE_WRAP_T, wt);
				glTexParameteri(target, GL_TEXTURE_MAG_FILTER, maf);
				glTexParameteri(target, GL_TEXTURE_MIN_FILTER, mif);
			}

			gl_check_error();

			TRACE(OPENGL, "begin load tex %p's tile %d into hwmem\n",
					tex, nr);
			switch (target) {
				case GL_TEXTURE_1D:
					if (tex->tile_w == get_tile_width(tex, x, y)) {
						glTexImage1D(GL_TEXTURE_1D,
								0,
								internalformat,
								tex->tile_w,
								0,
								format,
								type,
								data);
					} else {
						glTexImage1D(GL_TEXTURE_1D,
								0,
								internalformat,
								tex->tile_w,
								0,
								format,
								type,
								NULL);
						glTexSubImage1D(GL_TEXTURE_1D,
								0, 0,
								get_tile_width(tex, x, y),
								format,
								type,
								data);
					}
					break;
				case GL_TEXTURE_2D:
				case GL_TEXTURE_RECTANGLE:
					if ((tex->tile_w == get_tile_width(tex, x, y)) &&
							(tex->tile_h == get_tile_height(tex, x, y)))
					{
						glTexImage2D(target,
								0,
								internalformat,
								tex->tile_w,
								tex->tile_h,
								0,	/* border */
								format,
								type,
								data);
					} else {
						glTexImage2D(target,
								0,
								internalformat,
								tex->tile_w,
								tex->tile_h,
								0,	/* border */
								format,
								type,
								NULL);
						glTexSubImage2D(target,
								0, 0, 0,
								get_tile_width(tex, x, y),
								get_tile_height(tex, x, y),
								format,
								type,
								data);
					}
					break;
				default:
					FATAL(OPENGL, "Unknown texture target here: %d\n", target);
					THROW(EXCEPTION_FATAL,
							"Unknown texture target: 0x%x", target);
			}
			gl_check_error();

			TRACE(OPENGL, "Finish load texture %d\n", nr);
			/* check result */
			if (use_compress) {
				GLint c, f, s;
				glGetTexLevelParameteriv(
						target, 0, GL_TEXTURE_COMPRESSED, &c);
				glGetTexLevelParameteriv(
						target, 0, GL_TEXTURE_INTERNAL_FORMAT, &f);
				if (c)
					glGetTexLevelParameteriv(
							target, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &s);
				else
					s = get_tile_width(tex, x, y)
						* get_tile_height(tex, x, y) * 4;
				TRACE(OPENGL,
						"result: compressed=%d, internal format=0x%x, size=%d\n",
						c, f, s);
				tex->occupied_hwmem += s;
			} else {
				tex->occupied_hwmem += get_tile_width(tex, x, y)
					* get_tile_height(tex, x, y) * 4;
			}
			nr ++;
		}
	}

	/* Unbind texture */
	glBindTexture(target, 0);
	ALLOC_HWMEM(tex->occupied_hwmem);

	return;
}

static void
load_texgl(struct texture_gl * tex)
{
	if (tex->nr_hwtexs == 0) 
		return;
	if (tex->hwtexs == NULL)
		load_hwtexs(tex);
}

static void
texgl_reinit_hook(struct reinit_hook * hook)
{
	struct texture_gl * tex = container_of(hook,
			struct texture_gl, reinit_hook);
	TRACE(OPENGL, "tex %p know video reinit\n", tex);
	if (tex->hwtexs != NULL) {
		/* Re generate hw texture */
		glGenTextures(tex->nr_hwtexs, tex->hwtexs);
		TRY_CLEANUP(gl_check_error, free_hwtexs(tex));
		load_hwtexs(tex);
		if (!tex->use_bitmap_data)
			release_bitmap(tex);
	}
}


struct texture_gl *
texgl_create(res_id_t bitmap_res_id, struct rectangle rect,
		struct texture_params * params,
		struct texture_gl_params * gl_params)
{
	struct texture_gl * tex;

	static TEXTURE_GL_PARAM(default_gl_param);

	assert(gl_inited());

	if (gl_params == NULL)
		gl_params = &default_gl_param;

	TRACE(OPENGL, "create texture from bitmap %llu {" RECT_FMT "}\n",
			bitmap_res_id, RECT_ARG(&rect));

	tex = alloc_texture_gl();
	
	tex->base.cleanup.function = texture_gl_cleanup;

	tex_common_init(&tex->base, bitmap_res_id, rect, params);

	/* insert the reinit hook */
	tex->reinit_hook.fn = texgl_reinit_hook;
	tex->reinit_hook.pprivate = tex;
	video_hook_reinit(&tex->reinit_hook);

	tex->gl_params = *gl_params;

	make_cleanup(&tex->base.cleanup);

	if (bitmap_res_id == 0)
		return tex;

	/* compute hw data */
	/* init_texgl will grab the bitmap */
	init_texgl(tex);

	load_texgl(tex);

	if (tex->base.params.pin)
		free_phy_bitmap(tex);

	if ((!tex->use_bitmap_data) || (tex->base.params.pin))
		release_bitmap(tex);
	return tex;
}

static void
fast_fillmesh4(struct texture_gl * tex,
		struct tex_point * o_points,
		struct tex_point * i_points)
{
	SILENT(OPENGL, "fastmesh 4\n");
	assert(tex->nr_hwtexs == 1);
	memcpy(o_points, i_points, 4 * sizeof(*i_points));

	if (tex->internal_type == TEXGL_RECT) {
#define ___setp(n, axis, v)	do { \
		o_points[n].u.i.it##axis = v;	\
	} while(0)


#define __setp(n, vx, vy)	do { \
		___setp(n, x, vx);	\
		___setp(n, y, vy);	\
	} while(0)

		int tw = tex->base.rect.w;
		int th = tex->base.rect.h;

		__setp(0, 0, 0);
		__setp(1, tw, 0);
		__setp(2, tw, th);
		__setp(3, 0, th);
#undef ___setp
	} else {
		
#define ___setp(n, axis, v) do { \
		o_points[n].u.f.t##axis = v;\
	} while(0)
		__setp(0, 0.0, 0.0);
		__setp(1, 1.0, 0.0);
		__setp(2, 1.0, 1.0);
		__setp(3, 0.0, 1.0);
	}
#undef ___setp
#undef __setp
}

struct vec3 {
	float x, y, z;
};

static void
fill_opoints(struct texture_gl * tex,
		struct tex_point * o_points,
		struct vec3 * points)
{
	/* for each texture, form o_points */
	int nr = 0;
	for (int y = 0; y < tex->nh; y++) {
		for (int x = 0; x < tex->nw; x++) {
			struct tex_point * o = &o_points[4 * nr];
			int np0 = (tex->nw + 1) * (y) + x;
			int np1 = (tex->nw + 1) * (y) + x + 1;
			int np2 = (tex->nw + 1) * (y + 1) + x + 1;
			int np3 = (tex->nw + 1) * (y + 1) + x;

#define ___seto(n, axis)	\
			o[n].p##axis = points[np##n].axis
#define __seto(n)	\
			do {	\
				___seto(n, x);	\
				___seto(n, y);	\
				___seto(n, z);	\
			} while(0)
			__seto(0);
			__seto(1);
			__seto(2);
			__seto(3);
#undef __seto
#undef ___seto

			if (tex->internal_type == TEXGL_RECT) {
				int itx, ity;
				itx = get_tile_width(tex, x, y);
				ity = get_tile_height(tex, x, y);
				o[0].u.i.itx = o_points[0].u.i.ity = 0;

				o[1].u.i.itx = itx;
				o[1].u.i.ity = 0;

				o[2].u.i.itx = itx;
				o[2].u.i.ity = ity;

				o[3].u.i.itx = 0;
				o[3].u.i.ity = ity;
			} else {
				float tx, ty;
				tx = (float)get_tile_width(tex, x, y) / (float)tex->tile_w;
				ty = (float)get_tile_height(tex, x, y) / (float)tex->tile_h;

				SILENT(OPENGL, "tx=%f, ty=%f\n", tx, ty);
				o[0].u.f.tx = o[0].u.f.ty = 0.0f;

				o[1].u.f.tx = tx;
				o[1].u.f.ty = 0.0;

				o[2].u.f.tx = tx;
				o[2].u.f.ty = ty;

				o[3].u.f.tx = 0.0;
				o[3].u.f.ty = ty;
			}

			SILENT(OPENGL, "for texture %d:\n", nr);
			for (int i = 0; i < 4; i++) {
				SILENT(OPENGL, "\tp%d: %f %f %f %f %f\n",
						i,
						o[i].px,
						o[i].py,
						o[i].pz,
						o[i].u.f.tx,
						o[i].u.f.ty
						);
			}

			nr++;
		}
	}
	
}

static void
fillmesh4(struct texture_gl * tex,
		struct tex_point * o_points,
		struct tex_point * i_points)
{
	SILENT(OPENGL, "fillmesh 4\n");

	if (tex->nr_hwtexs == 1) {
		fast_fillmesh4(tex, o_points, i_points);
		return;
	}

	int nr = 0;
	struct vec3 * points =
#ifdef HAVE_ALLOCA
		alloca((tex->nr_hwtexs + tex->nh + tex->nw + 1) * sizeof(*points));
#else
		GC_MALLOC_BLOCK((tex->nr_hwtexs + tex->nh + tex->nw + 1) * sizeof(*points));
#endif

#define hinterp(p0, p1, axis)	\
	i_points[p0].p##axis + (i_points[p1].p##axis - i_points[p0].p##axis) * \
	y * ((float)tex->tile_h / (float)tex->base.rect.h)


	/* for each point */
	for (int y = 0; y < tex->nh + 1; y++) {
		/* clamp line */
		float x0, y0, z0, x1, y1, z1;

		SILENT(OPENGL, "for hwtex (??, %d):\n", y);

		if (y < tex->nh) {
			x0 = hinterp(0, 3, x);
			y0 = hinterp(0, 3, y);
			z0 = hinterp(0, 3, z);

			x1 = hinterp(1, 2, x);
			y1 = hinterp(1, 2, y);
			z1 = hinterp(1, 2, z);
#undef hinterp
		} else {
			x0 = i_points[3].px;
			y0 = i_points[3].py;
			z0 = i_points[3].pz;

			x1 = i_points[2].px;
			y1 = i_points[2].py;
			z1 = i_points[2].pz;
		}

		SILENT(OPENGL, "clamped pts: (%f, %f, %f), (%f, %f, %f)\n",
				x0, y0, z0, x1, y1, z1);

#define winterp(axis)	\
		axis##0 + (axis##1 - axis##0) * x \
			* ((float)tex->tile_w / (float)tex->base.rect.w)

		for (int x = 0; x < tex->nw; x++) {
			float x2, y2, z2;
			x2 = winterp(x);
			y2 = winterp(y);
			z2 = winterp(z);
#undef winterp
			points[nr].x = x2;
			points[nr].y = y2;
			points[nr].z = z2;
			SILENT(OPENGL, "pts at (%d, %d): (%f, %f, %f)\n",
					x, y, points[nr].x, points[nr].y, points[nr].z);
			nr ++;
		}
		/* last column is special */
		points[nr].x = x1;
		points[nr].y = y1;
		points[nr].z = z1;
		SILENT(OPENGL, "pts at (%d, %d): (%f, %f, %f)\n",
				tex->nw, y, points[nr].x, points[nr].y, points[nr].z);
		nr ++;
	}

	fill_opoints(tex, o_points, points);

#ifndef HAVE_ALLOCA
	GC_FREE_BLOCK_SET(points);
#endif
}

static void
fillmesh3(struct texture_gl * tex,
		struct tex_point * o_points,
		struct tex_point * i_points)
{
	mat4x4 M, T, P;
	load_identity(&T);
	load_identity(&P);

	T.m[0][0] = i_points[0].u.f.tx;
	T.m[0][1] = i_points[0].u.f.ty;
	T.m[0][2] = 1.0f;
	T.m[0][3] = 0.0f;


	T.m[1][0] = i_points[1].u.f.tx;
	T.m[1][1] = i_points[1].u.f.ty;
	T.m[1][2] = 1.0f;
	T.m[1][3] = 0.0f;

	T.m[2][0] = i_points[2].u.f.tx;
	T.m[2][1] = i_points[2].u.f.ty;
	T.m[2][2] = 1.0f;
	T.m[2][3] = 0.0f;

	P.m[0][0] = i_points[0].px;
	P.m[0][1] = i_points[0].py;
	P.m[0][2] = i_points[0].pz;
	P.m[0][3] = 0.0f;

	P.m[1][0] = i_points[1].px;
	P.m[1][1] = i_points[1].py;
	P.m[1][2] = i_points[1].pz;
	P.m[1][3] = 0.0f;

	P.m[2][0] = i_points[2].px;
	P.m[2][1] = i_points[2].py;
	P.m[2][2] = i_points[2].pz;
	P.m[2][3] = 0.0f;

	invert_matrix(&T, &T);

	mulmm(&M, &P, &T);

	struct vec3 * points = 
#ifdef HAVE_ALLOCA
		alloca((tex->nr_hwtexs + tex->nh + tex->nw + 1) * sizeof(*points));
#else
		GC_MALLOC_BLOCK((tex->nr_hwtexs + tex->nh + tex->nw + 1) * sizeof(*points));
#endif

	float ty = 0.0f;
	int nr = 0;
	/* for each point */
	for (int y = 0; y < tex->nh + 1; y++) {
		float tx = 0.0f;
		for (int x = 0; x < tex->nw + 1; x++) {
			vec4 tv, pv;
			tv.v[0] = tx;
			tv.v[1] = ty;
			tv.v[2] = 1.0f;
			tv.v[3] = 0.0f;

			mulmv(&pv, &M, &tv);

			points[nr].x = pv.v[0];
			points[nr].y = pv.v[1];
			points[nr].z = pv.v[2];

			nr++;
			tx += (float)tex->tile_w / (float)tex->base.rect.w;
			if (tx > 1.0f)
				tx = 1.0f;
		}

		if (y == tex->nh)
			ty = 1.0f;
		else
			ty += (float)tex->tile_h / (float)tex->base.rect.h;
	}

	fill_opoints(tex, o_points, points);

#ifndef HAVE_ALLOCA
	GC_FREE_BLOCK_SET(points);
#endif
}

void
texgl_fillmesh(struct texture_gl * tex,
		struct tex_point * o_points,
		int nr_ipoints,
		struct tex_point * i_points)
{
	switch (nr_ipoints) {
		case 4:
			fillmesh4(tex, o_points, 
					i_points);
			break;
		case 3:
			fillmesh3(tex, o_points, 
					i_points);
			break;
		default:
			THROW(EXCEPTION_FATAL, "wrong nr_ipoints: %d",
					nr_ipoints);
			break;
	}
	return;
}

#endif	/* VIDEO_OPENGL_ENGINE */

// vim:tabstop=4:shiftwidth=4

