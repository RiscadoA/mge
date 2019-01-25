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
};

#define MGE_DEFAULT_ENGINE_CONFIG ((mge_engine_config_t) { \
MGL_FALSE,\
})

void mge_load_config(int argc, char** argv, mge_engine_config_t* config);

#ifdef __cplusplus
}
#endif
#endif
