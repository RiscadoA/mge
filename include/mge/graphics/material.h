#ifndef MGE_GRAPHICS_MATERIAL_H
#define MGE_GRAPHICS_MATERIAL_H
#ifdef __cplusplus
extern "C" {
#endif 

#include <mgl/type.h>

	typedef struct mge_graphics_material_t mge_graphics_material_t;

	struct mge_graphics_material_t
	{
		mgl_bool_t transparent;
	};

#ifdef __cplusplus
}
#endif
#endif
