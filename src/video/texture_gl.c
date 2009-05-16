/* 
 * texture_gl.c
 * Texture OpenGL implementation
 */

#include <common/defs.h>
#include <common/exception.h>
#include <common/mm.h>
#include <common/geometry.h>

#include <resource/bitmap.h>

#include <video/texture.h>
#include <video/texture_gl.h>

#define FREE_HWMEM(n)	do {} while(0)
#define ALLOC_HWMEM(n)	do {} while(0)

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
	b = GET_BITMAP(tex->base.bitmap_res_id);
	assert(b != NULL);
	SET_TEXGL_BITMAP(tex, b);
}

static void
free_hwmem(struct texture_gl * tex)
{
	if (tex->hwtexs) {
		TRACE(OPENGL, "\tdelete hw texture\n");
		glDeleteTextures(tex->nr_hwtexs, tex->hwtexs);
		GL_POP_ERROR();
		
		if (tex->hwtexs != tex->_hwtexs_save)
			GC_FREE_BLOCK_SET(tex->hwtexs);
		
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
		default:
			INTERNAL_ERROR(MEMORY, "shrink power %d unknown\n", p);
	}
	return;
}

static void
texture_gl_cleanup(struct cleanup * str)
{
	struct texture_gl * tex;

	tex = container_of(str,
			struct texture_gl, base.cleanup);

	TRACE(OPENGL, "cleanup gl texture %p\n", tex);

	free_hwmem(tex);
	release_bitmap(tex);
	free_phy_bitmap(tex);

	list_del(&tex->list);

	tex_common_cleanup(&tex->base);

	dealloc_texture_gl(tex);
}

/* init texgl will hold the bitmap, continouse methods should
 * release it */
static void
init_texgl(struct texture_gl * tex)
{
	struct bitmap * b;
	struct rectangle * rect = &tex->base.rect;

	TRACE(OPENGL, "Init texgl %p from bitmap %llu\n",
			tex->base.bitmap_res_id);

	load_bitmap(tex);
	b = tex->base.bitmap;
	assert(b != NULL);
	TRACE(OPENGL, "bitmap size: %d, %d\n", b->w, b->h);

	/* convert the rect */
	struct rectangle full_rect = {
		0, 0, b->w, b->h,
	};
	*rect = geom_rect_cover(full_rect, *rect);
	TRACE(OPENGL, "bounded rect:" RECT_FMT "\n", RECT_ARG(rect));

	/* how many hw texture we need? */
}

static void
load_texgl(struct texture_gl * tex)
{
	return;
}


struct texture *
tex_create(res_id_t bitmap_res_id, struct rectangle rect,
		struct texture_params * params,
		void * args)
{
	struct texture_gl_params * gl_params = (struct texture_gl_params *)args;
	struct texture_gl * tex;

	static TEXTURE_GL_PARAM(default_gl_param);

	if (gl_params == NULL)
		gl_params = &default_gl_param;

	TRACE(OPENGL, "create texture from bitmap %llu {"  RECT_FMT "}\n",
			bitmap_res_id, RECT_ARG(&rect));

	tex = alloc_texture_gl();
	
	tex->base.cleanup.args = tex;
	tex->base.cleanup.function = texture_gl_cleanup;

	tex_common_init(&tex->base, bitmap_res_id, rect, params);

	tex->gl_params = *gl_params;

	/* insert into priority list */
	insert_texgl(tex);

	if (bitmap_res_id == 0)
		return &tex->base;

	/* compute hw data */
	init_texgl(tex);
	if ((tex->base.params.pin) || (tex->gl_params.imm)) {
		load_texgl(tex);
		TEXGL_SHRINK(tex, GC_NORMAL);
	}
	return &tex->base;
}

// vim:tabstop=4:shiftwidth=4

