#ifndef MGE_RESOURCE_TEXT_H
#define MGE_RESOURCE_TEXT_H
#ifdef __cplusplus
extern "C" {
#endif 

#include <mge/resource/manager.h>

	typedef struct mge_text_resource_data_t mge_text_resource_data_t;
	typedef struct mge_text_resource_access_t mge_text_resource_access_t;

	struct mge_text_resource_access_t
	{
		mge_resource_access_base_t base;
		mge_text_resource_data_t* data;
	};

	struct mge_text_resource_data_t
	{
		void* allocator;
		mgl_u64_t size;
		const mgl_chr8_t* text;
	};

	void mge_resource_load_text(void* allocator, mge_resource_t* rsc);

	void mge_resource_unload_text(mge_resource_t* rsc);

	void mge_resource_access_text(mge_resource_t* rsc, mge_text_resource_access_t* access);

#ifdef __cplusplus
}
#endif
#endif
