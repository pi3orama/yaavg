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

#ifdef VIDEO_OPENGL_ENGINE

#define FREE_HWMEM(n)	do {} while(0)
#define ALLOC_HWMEM(n)	do {} while(0)

#define free_hwtexs(tex)	do {	\
	if (tex->hwtexs != tex->_hwtexs_save)	\
		GC_FREE_BLOCK_SET(tex->hwtexs);		\
} while (0)

static LIST_HEAD(texture_gl_list);

static void
insert_texgl(struct texture_gl * tex)
{
	struct list_head * pos;
	list_for_each(pos, &texture_gl_list) {
		struct texture_gl * t;
		t = list_entry(pos, typeof(*t), list);
		if (t->gl_params.importance >= tex->gl_params.importance)
			break;
	}
	__list_add(&tex->list, pos->prev, pos);
}

static void
texture_gl_shrink(struct gc_tag * tag, enum gc_power p);
static void
texture_gl_cleanup(struct cleanup * str);

static inline struct texture_gl *
alloc_texture_gl(void)
{
	struct texture_gl * tex;
	tex = gc_calloc(sizeof(*tex),
			offsetof(struct texture_gl, base.__gc_tag),
			0,
			texture_gl_shrink,
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
texture_gl_shrink(struct gc_tag * tag, enum gc_power p)
{
	struct texture_gl * tex;
	tex = (struct texture_gl *)tag->ptr;
	TRACE(OPENGL, "shrink texture %p, power=%d\n", tex, p);

	switch (p) {
		case GC_DESTROY:
			WARNING(MEMORY, "gl texture %p destroied\n",
					tex);
			TEXGL_CLEANUP(tex);
			break;
		case GC_HARD:
			free_hwmem(tex);
		case GC_MEDIUM:
			release_bitmap(tex);
		case GC_LIGHT:
			free_phy_bitmap(tex);
		case GC_NORMAL:
			if (tex->base.params.pin) {
				free_phy_bitmap(tex);
			}
			break;
		default:
			INTERNAL_ERROR(MEMORY, "shrink power %d unknown\n", p);
	}
	return;
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

	if (tex->gl_params.flat) {
		tex->nr_hwtexs = 0;
		tex->use_bitmap_data = TRUE;
		release_bitmap(tex);
		return;
	}

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
			tex->tile_w = rect->w;
			tex->tile_h = rect->h;
			TRACE(OPENGL,
					"tex %p use NORMAL (%d x %d)\n",
					tex, tex->tile_w, tex->tile_h);
		} else {
			if (gl_tex_NPOT_enabled()) {
				tex->internal_type = TEXGL_NPOT;
				tex->tile_w = rect->w;
				tex->tile_h = rect->h;
				TRACE(OPENGL,
						"tex %p use NPOT (%d x %d)\n",
						tex, tex->tile_w, tex->tile_h);
			} else if (gl_tex_RECT_enabled()) {
				tex->internal_type = TEXGL_RECT;
				tex->tile_w = rect->w;
				tex->tile_h = rect->h;
				TRACE(OPENGL,
						"tex %p use RECT (%d x %d)\n",
						tex, tex->tile_w, tex->tile_h);
			} else {
				tex->internal_type = TEXGL_NORMAL;
				tex->tile_w = potw;
				tex->tile_h = poth;
				TRACE(OPENGL,
						"tex %p use NORMAL (%d x %d)\n",
						tex, tex->tile_w, tex->tile_h);
			}
		}
	} else {
		tex->internal_type = TEXGL_NORMAL;
		tex->tile_w = rect->w > gl_max_texture_size() ?
			gl_max_texture_size() : potw;
		tex->tile_h = rect->h > gl_max_texture_size() ?
			gl_max_texture_size() : poth;
		TRACE(OPENGL,
				"tex %p use NORMAL (%d x %d)\n",
				tex, tex->tile_w, tex->tile_h);
	}

	release_bitmap(tex);
	return;
}


/* give the tile number(x, y), return:
 * the start ptr in bitmap's data */
static inline uint8_t *
tile_to_data(struct texture_gl * tex, int x, int y)
{
	load_bitmap(tex);

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
	if (tex->gl_params.flat) {
		tex->occupied_hwmem = 0;
		return;
	}

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

	if (gl_tex_compression_enabled())
		internalformat = GL_COMPRESSED_RGBA;
	else
		internalformat = GL_RGBA;

	format = tex->tex_data_format;

	for (y = 0; y < tex->nh; y ++) {
		for (x = 0; x < tex->nw; x++) {
			uint8_t * data;
			GLenum target = tex->gl_params.target;
			data = tile_to_phydata(tex, x, y);
			glBindTexture(target, tex->hwtexs[nr]);
			TRACE(OPENGL, "tex %p: start load hwtex %d from phy data %p\n",
					tex, nr, data);

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
					if ((tex->tile_w == get_tile_width(tex, x, y)) &&
							(tex->tile_h == get_tile_height(tex, x, y)))
					{
						glTexImage2D(GL_TEXTURE_2D,
								0,
								internalformat,
								tex->tile_w,
								tex->tile_h,
								0,	/* border */
								format,
								type,
								data);
					} else {
						glTexImage2D(GL_TEXTURE_2D,
								0,
								internalformat,
								tex->tile_w,
								tex->tile_h,
								0,	/* border */
								format,
								type,
								NULL);
						glTexSubImage2D(GL_TEXTURE_2D,
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
			if (gl_tex_compression_enabled()) {
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

	ALLOC_HWMEM(tex->occupied_hwmem);

	return;
}

/* 
 * after load_texgl, bitmap is released, if not use bitmap data
 */
static void
load_texgl(struct texture_gl * tex)
{
	if (tex->gl_params.flat)
		return;
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

	TRACE(OPENGL, "create texture from bitmap %llu {"  RECT_FMT "}\n",
			bitmap_res_id, RECT_ARG(&rect));

	tex = alloc_texture_gl();
	
	tex->base.cleanup.args = tex;
	tex->base.cleanup.function = texture_gl_cleanup;

	tex_common_init(&tex->base, bitmap_res_id, rect, params);

	/* insert the reinit hook */
	tex->reinit_hook.fn = texgl_reinit_hook;
	tex->reinit_hook.pprivate = tex;
	video_hook_reinit(&tex->reinit_hook);

	tex->gl_params = *gl_params;

	/* insert into priority list */
	insert_texgl(tex);
	make_cleanup(&tex->base.cleanup);

	if (bitmap_res_id == 0)
		return tex;

	/* compute hw data */
	init_texgl(tex);
	if ((tex->base.params.pin) || (tex->gl_params.imm)) {
		load_texgl(tex);
	}

	TEXGL_SHRINK(tex, GC_NORMAL);
	if (!tex->use_bitmap_data)
		release_bitmap(tex);
	return tex;
}

#endif	/* VIDEO_OPENGL_ENGINE */

// vim:tabstop=4:shiftwidth=4

