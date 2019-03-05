#ifndef MGE_CONFIG_H
#define MGE_CONFIG_H
#ifdef __cplusplus
extern "C" {
#endif 

#include <mgl/type.h>

typedef struct mge_engine_config_t mge_engine_config_t;

struct mge_engine_config_t
{
	mgl_bool_t debug_mode;
	mgl_u64_t max_resource_count;
	mgl_u64_t max_scene_node_count;
	mgl_u32_t window_width;
	mgl_u32_t window_height;
	mgl_bool_t window_fullscreen;
};

#define MGE_DEFAULT_ENGINE_CONFIG ((mge_engine_config_t) { \
MGL_FALSE,\
1024,\
1024,\
800,\
600,\
MGL_FALSE,\
})

void mge_load_config(int argc, char** argv, mge_engine_config_t* config);

#ifdef __cplusplus
}
#endif
#endif
