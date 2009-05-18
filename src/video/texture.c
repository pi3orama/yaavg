/* 
 * texture.c by WN @ May. 16, 2009
 */

#include <video/texture.h>

void
tex_common_init(struct texture * tex,
		res_id_t bitmap_res_id,
		struct rectangle rect,
		struct texture_params * params)
{
	static TEXTURE_PARAM(default_param);
	tex->bitmap_res_id = bitmap_res_id;
	tex->rect = rect;
	geom_adjust_rect(&tex->rect);
	if (params == NULL)
		params = &default_param;
	tex->params = *params;

	tex->bitmap = NULL;
	tex->ref_count = 0;
	return;
}

void
tex_common_cleanup(struct texture * tex)
{
	return;
}

// vim:tabstop=4:shiftwidth=4

