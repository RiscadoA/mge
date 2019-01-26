#ifndef MGE_RESOURCE_MANAGER_H
#define MGE_RESOURCE_MANAGER_H
#ifdef __cplusplus
extern "C" {
#endif 

#include <mgl/type.h>
#include <mgl/thread/mutex.h>

#define MGE_MAX_RESOURCE_DEPENDENCY_COUNT 8
#define MGE_MAX_RESOURCE_NAME_SIZE 64
#define MGE_MAX_RESOURCE_DATA_PATH_SIZE 256

	typedef struct mge_resource_t mge_resource_t;
	typedef struct mge_resource_access_base_t mge_resource_access_base_t;
	typedef struct mge_resource_manager_t mge_resource_manager_t;

	enum
	{
		MGE_RESOURCE_HINT_CPU_ONLY	= 0x00000001,
		MGE_RESOURCE_HINT_GPU_ONLY	= 0x00000002,
		MGE_RESOURCE_HINT_PERMANENT = 0x00000004,
	};

	enum
	{
		MGE_RESOURCE_EMPTY				= 0x00,
		MGE_RESOURCE_TEXT				= 0x01,
		MGE_RESOURCE_MESH				= 0x02,
		MGE_RESOURCE_SKELETON			= 0x03,
		MGE_RESOURCE_ANIMATION			= 0x04,
		MGE_RESOURCE_SOUND				= 0x05,
		MGE_RESOURCE_STREAMING_SOUND	= 0x06,
		MGE_RESOURCE_MATERIAL			= 0x07,
		MGE_RESOURCE_SHADER				= 0x08,
	};

	struct mge_resource_t
	{
		mgl_enum_u32_t type;
		mgl_flags_u32_t hints;
		mgl_u32_t dependency_count;
		mgl_chr8_t name[MGE_MAX_RESOURCE_NAME_SIZE];

		mge_resource_manager_t* manager;

		struct
		{
			mgl_mutex_t mutex;
			mgl_u64_t reference_count;

			void* ptr;
			mgl_chr8_t path[MGE_MAX_RESOURCE_DATA_PATH_SIZE];
			mgl_u64_t offset;
		} data;

		struct
		{
			mge_resource_t* resource;
			mgl_chr8_t name[MGE_MAX_RESOURCE_NAME_SIZE];
		} dependencies[MGE_MAX_RESOURCE_DEPENDENCY_COUNT];
	};

	struct mge_resource_access_base_t
	{
		mge_resource_t* rsc;
	};

	/// <summary>
	///		Initializes a resource manager.
	/// </summary>
	/// <param name="allocator">Allocator used</param>
	/// <param name="max_resource_count">Max resource count</param>
	/// <returns>Pointer to manager</returns>
	mge_resource_manager_t* mge_init_resource_manager(void* allocator, mgl_u64_t max_resource_count);

	/// <summary>
	///		Terminates a resource manager.
	/// </summary>
	/// <param name="manager">Pointer to manager</param>
	void mge_terminate_resource_manager(mge_resource_manager_t* manager);

	/// <summary>
	///		Adds a resource info file to the resource manager.
	/// </summary>
	/// <param name="manager">Pointer to manager</param>
	/// <param name="path">Path to resource info file</param>
	void mge_add_resource_info_file(mge_resource_manager_t* manager, const mgl_chr8_t* path);

	/// <summary>
	///		Searchs for a resource.
	/// </summary>
	/// <param name="manager">Pointer to manager</param>
	/// <param name="name">Resource name</param>
	/// <returns>Pointer to resource</returns>
	mge_resource_t* mge_find_resource(mge_resource_manager_t* manager, const mgl_chr8_t* name);

	/// <summary>
	///		Opens a resource access.
	/// </summary>
	/// <param name="rsc">Resource pointer</param>
	/// <param name="access">Access pointer</param>
	/// <param name="rsc_type">Type of resource</param>
	void mge_open_resource(mge_resource_t* rsc, void* access, mgl_enum_u32_t rsc_type);

	/// <summary>
	///		Closes a resource access.
	/// </summary>
	/// <param name="access">Resource a access</param>
	void mge_close_resource(void* access);

#ifdef __cplusplus
}
#endif
#endif
