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

static void
texture_gl_shrink(struct gc_tag * tag, enum gc_power p);
static void
texture_gl_cleanup(struct cleanup * str);

static struct texture_gl *
alloc_texture_gl(void)
{
	struct texture_gl * tex;
	tex = gc_calloc(sizeof(*tex),
			offsetof(struct texture_gl, base.__gc_tag),
			0,
			texture_gl_shrink,
			NULL);
	return NULL;
}

static void
dealloc_texture_gl(struct texture_gl * tex)
{
	return;
}

static void
texture_gl_shrink(struct gc_tag * tag, enum gc_power p)
{
	struct texture_gl * tex;
	tex = (struct texture_gl *)tag->ptr;
	TRACE(OPENGL, "shrink texture %p, power=%d\n", tex, p);
	if (p >= GC_LIGHT) {
		if (!tex->params.flat) {
			if ((TEXGL_BITMAP(tex))
					&&
					(tex->phy_bitmap != TEXGL_BITMAP(tex)->data))
			{
				/* free phy_bitmap */
				TRACE(OPENGL, "\tfree phy_bitmap\n");
				GC_FREE_BLOCK(tex->phy_bitmap);
				tex->phy_bitmap = NULL;
			}
		}
	}

	if (p >= GC_HARD) {
		if (tex->phy_bitmap != NULL) {
			if ((TEXGL_BITMAP(tex))
					&&
					(tex->phy_bitmap == TEXGL_BITMAP(tex)->data))
			{
				TRACE(OPENGL, "\tput bitmap\n");
				PUT_BITMAP(TEXGL_BITMAP(tex));
				SET_TEXGL_BITMAP(tex, NULL);
			} else {
				TRACE(OPENGL, "\tfree phy_bitmap\n");
				GC_FREE_BLOCK(tex->phy_bitmap);
			}

			tex->phy_bitmap = NULL;
		}
	}
	
	if (p == GC_DESTROY) {
		WARNING(MEMORY, "gl texture %p destroied\n",
				tex);
		TEXGL_CLEANUP(tex);
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

	if (tex->hwtexs) {
		TRACE(OPENGL, "\tdelete hw texture\n");
		glDeleteTextures(tex->nr_hwtexs, tex->hwtexs);
		
		if (tex->hwtexs != tex->_hwtexs_save) {
			GC_FREE_BLOCK(tex->hwtexs);
			tex->hwtexs = NULL;
		}

		tex->hwtexs = NULL;
		FREE_HWMEM(tex->occupied_hwmem);
	}

	if (tex->phy_bitmap) {
		if (!((TEXGL_BITMAP(tex))
				&&
				(tex->phy_bitmap == TEXGL_BITMAP(tex)->data)))
		{
			TRACE(OPENGL, "\tfree phy_bitmap\n");
			GC_FREE_BLOCK(tex->phy_bitmap);
		}
		tex->phy_bitmap = NULL;
	}

	if (TEXGL_BITMAP(tex)) {
		TRACE(OPENGL, "\tput bitmap\n");
		PUT_BITMAP(TEXGL_BITMAP(tex));
		SET_TEXGL_BITMAP(tex, NULL);
	}
	list_del(&tex->list);

	dealloc_texture_gl(tex);
}

struct texture *
tex_create(res_id_t bitmap_res_id, struct rectangle rect, void * args)
{
	struct texture_gl_params * params = (struct texture_gl_params *)args;
}

// vim:tabstop=4:shiftwidth=4

