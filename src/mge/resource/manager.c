#include <mge/resource/manager.h>
#include <mge/log.h>

#include <mge/resource/text.h>

#include <mgl/file/archive.h>
#include <mgl/string/manipulation.h>
#include <mgl/memory/allocator.h>
#include <mgl/memory/manipulation.h>

struct mge_resource_manager_t
{
	void* allocator;
	mgl_u64_t max_resource_count;
	mge_resource_t* resources;
};

static void mge_force_resource_load(mge_resource_t* rsc)
{
	MGL_DEBUG_ASSERT(rsc != NULL);

	switch (rsc->type)
	{
		case MGE_RESOURCE_EMPTY:
			break;

		case MGE_RESOURCE_TEXT:
			mge_resource_load_text(rsc->manager->allocator, rsc);
			break;

		default:
			MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"Couldn't load resource '");
			MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, rsc->name);
			MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"'\n");
			mge_fatal_error(MGE_LOG_ENGINE, u8"Failed to load resource, unsupported resource type");
			return;
	}

	MGE_LOG_VERBOSE_2(MGE_LOG_ENGINE, u8"Loaded resource '");
	MGE_LOG_VERBOSE_2(MGE_LOG_ENGINE, rsc->name);
	MGE_LOG_VERBOSE_2(MGE_LOG_ENGINE, u8"'\n");
}

static void mge_force_resource_unload(mge_resource_t* rsc)
{
	MGL_DEBUG_ASSERT(rsc != NULL);

	switch (rsc->type)
	{
		case MGE_RESOURCE_EMPTY:
			break;

		case MGE_RESOURCE_TEXT:
			mge_resource_unload_text(rsc);
			break;

		default:
			MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"Couldn't unload resource '");
			MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, rsc->name);
			MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"'\n");
			mge_fatal_error(MGE_LOG_ENGINE, u8"Failed to unload resource, unsupported resource type");
			return;
	}

	rsc->data.ptr = NULL;

	MGE_LOG_VERBOSE_2(MGE_LOG_ENGINE, u8"Unloaded resource '");
	MGE_LOG_VERBOSE_2(MGE_LOG_ENGINE, rsc->name);
	MGE_LOG_VERBOSE_2(MGE_LOG_ENGINE, u8"'\n");
}

static void mge_access_resource(mge_resource_t* rsc, void* access)
{
	MGL_DEBUG_ASSERT(rsc != NULL);

	switch (rsc->type)
	{
		case MGE_RESOURCE_TEXT:
			mge_resource_access_text(rsc, (mge_text_resource_access_t*)access);
			return;

		case MGE_RESOURCE_EMPTY:
			MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"Couldn't access resource '");
			MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, rsc->name);
			MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"'\n");
			mge_fatal_error(MGE_LOG_ENGINE, u8"Failed to access resource, EMPTY type resources cannot be accessed (they do not store data)");
			return;

		default:
			MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"Couldn't access resource '");
			MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, rsc->name);
			MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"'\n");
			mge_fatal_error(MGE_LOG_ENGINE, u8"Failed to access resource, unsupported resource type");
			break;
	}
}

static void mge_resource_load_dependencies(mge_resource_t* rsc)
{
	MGL_DEBUG_ASSERT(rsc != NULL);

	// Load dependencies
	for (mgl_u32_t i = 0; i < rsc->dependency_count; ++i)
	{
		// Find dependency resource
		if (rsc->dependencies[i].resource == NULL)
			rsc->dependencies[i].resource = mge_find_resource(rsc->manager, rsc->dependencies[i].name);

		// Lock data mutex
		mgl_error_t err = mgl_lock_mutex(&rsc->dependencies[i].resource->data.mutex);
		if (err != MGL_ERROR_NONE)
			mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to lock resource dependency data mutex", err);

		// Increase ref count
		rsc->dependencies[i].resource->data.reference_count += 1;

		// Load dependency dependencies
		mge_resource_load_dependencies(rsc->dependencies[i].resource);

		// Load resource
		if (rsc->dependencies[i].resource->data.ptr == NULL)
			mge_force_resource_load(rsc->dependencies[i].resource);

		// Unock data mutex
		err = mgl_unlock_mutex(&rsc->dependencies[i].resource->data.mutex);
		if (err != MGL_ERROR_NONE)
			mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to unlock resource dependency data mutex", err);
	}
}

static void mge_resource_unload_dependencies(mge_resource_t* rsc)
{
	MGL_DEBUG_ASSERT(rsc != NULL);

	// Unload dependencies
	for (mgl_u32_t i = 0; i < rsc->dependency_count; ++i)
	{
		// Check dependency
		if (rsc->dependencies[i].resource == NULL)
			mge_fatal_error(MGE_LOG_ENGINE, u8"Failed to unload resource dependency because it is NULL");

		// Lock data mutex
		mgl_error_t err = mgl_lock_mutex(&rsc->dependencies[i].resource->data.mutex);
		if (err != MGL_ERROR_NONE)
			mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to lock resource dependency data mutex", err);

		// Decrease ref count
		rsc->dependencies[i].resource->data.reference_count -= 1;

		if (rsc->dependencies[i].resource->data.reference_count == 0 && !(rsc->dependencies[i].resource->hints & MGE_RESOURCE_HINT_PERMANENT))
		{
			// Unload dependency dependencies
			mge_resource_unload_dependencies(rsc->dependencies[i].resource);

			// Unoad resource
			mge_force_resource_unload(rsc->dependencies[i].resource);
		}	

		// Unock data mutex
		err = mgl_unlock_mutex(&rsc->dependencies[i].resource->data.mutex);
		if (err != MGL_ERROR_NONE)
			mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to unlock resource dependency data mutex", err);
	}
}

static mge_resource_t* mge_get_free_resource(mge_resource_manager_t* manager)
{
	MGL_DEBUG_ASSERT(manager != NULL);

	for (mgl_u32_t i = 0; i < manager->max_resource_count; ++i)
		if (manager->resources[i].manager == NULL)
		{
			manager->resources[i].manager = manager;
			return &manager->resources[i];
		}

	mge_fatal_error(MGE_LOG_ENGINE, u8"Failed to create resource, max resource count surpassed");
	return NULL;
}

mge_resource_manager_t * mge_init_resource_manager(void * allocator, mgl_u64_t max_resource_count)
{
	MGL_DEBUG_ASSERT(allocator != NULL && max_resource_count > 0);

	mge_resource_manager_t* manager;

	// Allocate manager
	mgl_error_t err = mgl_allocate(allocator, sizeof(mge_resource_manager_t), (void**)&manager);
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to allocate resource manager", err);

	// Allocate resources
	err = mgl_allocate(allocator, max_resource_count * sizeof(mge_resource_t), (void**)&manager->resources);
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to allocate resources array on resource manager", err);

	manager->allocator = allocator;
	manager->max_resource_count = max_resource_count;

	// Init resources
	for (mgl_u64_t i = 0; i < manager->max_resource_count; ++i)
		manager->resources[i].manager = NULL;

	MGE_LOG_VERBOSE_1(MGE_LOG_ENGINE, u8"Successfully initialized resource manager\n");

	return manager;
}

void mge_terminate_resource_manager(mge_resource_manager_t * manager)
{
	MGL_DEBUG_ASSERT(manager != NULL);

	// Unload loaded resources
	for (mgl_u64_t i = 0; i < manager->max_resource_count; ++i)
		if (manager->resources[i].manager != NULL)
		{
			if (manager->resources[i].data.ptr != NULL)
				mge_force_resource_unload(&manager->resources[i]);
			mgl_error_t err = mgl_destroy_mutex(&manager->resources[i].data.mutex);
			if (err != MGL_ERROR_NONE)
				mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to destroy resource data mutex", err);
		}

	// Deallocate resources
	mgl_error_t err = mgl_deallocate(manager->allocator, manager->resources);
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to deallocate resources array on resource manager", err);

	// Deallocate manager
	err = mgl_deallocate(manager->allocator, manager);
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to deallocate resource manager", err);

	MGE_LOG_VERBOSE_1(MGE_LOG_ENGINE, u8"Successfully terminated resource manager\n");
}

void mge_add_resource_info_file(mge_resource_manager_t * manager, const mgl_chr8_t * path)
{
	MGL_DEBUG_ASSERT(manager != NULL && path != NULL);
	
	// Find and open file
	mgl_iterator_t file;
	mgl_error_t err = mgl_file_find(path, &file);
	if (err != MGL_ERROR_NONE)
	{
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"Couldn't find resource info file on '");
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, path);
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"'\n"); 
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to add resource info file, file not found", err);
	}

	mgl_file_stream_t stream;
	err = mgl_file_open(&file, &stream, MGL_FILE_READ);
	if (err != MGL_ERROR_NONE)
	{
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"Couldn't open resource info file on '");
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, path);
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"'\n"); 
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to add resource info file, couldn't open file", err);
	}

	// Get version
	mgl_u32_t version;
	err = mgl_read(&stream, &version, sizeof(version), NULL);
	if (err != MGL_ERROR_NONE)
		goto read_error;
	mgl_from_little_endian_4(&version, &version);

	if (version != 1)
	{
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"Couldn't open resource info file on '");
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, path);
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"'\n");
		mge_fatal_error(MGE_LOG_ENGINE, u8"Failed to add resource info file, unsupported file version (the only version supported is '1' for now)");
	}

	// Get resource count
	mgl_u32_t rsc_count;
	err = mgl_read(&stream, &rsc_count, sizeof(rsc_count), NULL);
	if (err != MGL_ERROR_NONE)
		goto read_error;
	mgl_from_little_endian_4(&rsc_count, &rsc_count);

	for (mgl_u32_t i = 0; i < rsc_count; ++i)
	{
		mge_resource_t* rsc = mge_get_free_resource(manager);

		// Get resource type
		err = mgl_read(&stream, &rsc->type, sizeof(rsc->type), NULL);
		if (err != MGL_ERROR_NONE)
			goto read_error;
		mgl_from_little_endian_4(&rsc->type, &rsc->type);

		// Get resource hints
		err = mgl_read(&stream, &rsc->hints, sizeof(rsc->hints), NULL);
		if (err != MGL_ERROR_NONE)
			goto read_error;
		mgl_from_little_endian_4(&rsc->hints, &rsc->hints);

		// Get resource data offset
		err = mgl_read(&stream, &rsc->data.offset, sizeof(rsc->data.offset), NULL);
		if (err != MGL_ERROR_NONE)
			goto read_error;
		mgl_from_little_endian_8(&rsc->data.offset, &rsc->data.offset);

		// Get resource name
		err = mgl_read(&stream, &rsc->name, MGE_MAX_RESOURCE_NAME_SIZE, NULL);
		if (err != MGL_ERROR_NONE)
			goto read_error;

		// Get resource data path
		err = mgl_read(&stream, &rsc->data.path, MGE_MAX_RESOURCE_DATA_PATH_SIZE, NULL);
		if (err != MGL_ERROR_NONE)
			goto read_error;

		// Get resource dependency count
		err = mgl_read(&stream, &rsc->dependency_count, sizeof(rsc->dependency_count), NULL);
		if (err != MGL_ERROR_NONE)
			goto read_error;
		mgl_from_little_endian_4(&rsc->dependency_count, &rsc->dependency_count);

		if (rsc->dependency_count > MGE_MAX_RESOURCE_DEPENDENCY_COUNT)
		{
			MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"Couldn't open resource info file on '");
			MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, path);
			MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"'\n");
			MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"Too many dependencies on resource '");
			MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, rsc->name);
			MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"'\n");

#define MGE_XSTR(a) MGE_STR(a)
#define MGE_STR(a) #a
			mge_fatal_error(MGE_LOG_ENGINE, u8"Failed to add resource info file, too many dependencies, the maximum dependency count supported is " MGE_XSTR(MGE_MAX_RESOURCE_DEPENDENCY_COUNT) );
		}

		for (mgl_u32_t j = 0; j < rsc->dependency_count; ++j)
		{
			// Get dependency name
			err = mgl_read(&stream, &rsc->dependencies[j].name, MGE_MAX_RESOURCE_NAME_SIZE, NULL);
			if (err != MGL_ERROR_NONE)
				goto read_error;
			rsc->dependencies[j].resource = NULL;
		}

		rsc->data.ptr = NULL;
		rsc->data.reference_count = 0;
		err = mgl_create_mutex(&rsc->data.mutex);
		if (err != MGL_ERROR_NONE)
			mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to create resource data mutex", err);

		if (rsc->hints & MGE_RESOURCE_HINT_PERMANENT)
			mge_force_resource_load(rsc);
	}

	// Close file
	mgl_file_close(&stream);

	return;

read_error:
	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"Failed to read resource info file on '");
	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, path);
	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"'\n");
	mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to read resource info file", err);
}

mge_resource_t * mge_find_resource(mge_resource_manager_t * manager, const mgl_chr8_t * name)
{
	MGL_DEBUG_ASSERT(manager != NULL && name != NULL);

	for (mgl_u64_t i = 0; i < manager->max_resource_count; ++i)
		if (manager->resources[i].manager == manager && mgl_str_equal(name, manager->resources[i].name))
			return &manager->resources[i];

	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"Couldn't find resource '");
	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, name);
	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"'\n");
	mge_fatal_error(MGE_LOG_ENGINE, u8"Failed to find resource");

	return NULL;
}

void mge_open_resource(mge_resource_t * rsc, void * access, mgl_enum_u32_t rsc_type)
{
	MGL_DEBUG_ASSERT(rsc != NULL && access != NULL);
	if (rsc_type != rsc->type)
		mge_fatal_error(MGE_LOG_ENGINE, u8"Failed to open resource (resource type doesn't match passed type)");

	// Lock data mutex
	mgl_error_t err = mgl_lock_mutex(&rsc->data.mutex);
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to lock resource data mutex", err);

	// Increase ref count
	rsc->data.reference_count += 1;

	// Load resource
	if (rsc->data.ptr == NULL)
	{
		mge_resource_load_dependencies(rsc);
		mge_force_resource_load(rsc);
	}

	// Access resource
	mge_access_resource(rsc, access);

	// Unlock data mutex
	err = mgl_unlock_mutex(&rsc->data.mutex);
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to unlock resource data mutex", err);

	MGE_LOG_VERBOSE_3(MGE_LOG_ENGINE, u8"Opened resource '");
	MGE_LOG_VERBOSE_3(MGE_LOG_ENGINE, rsc->name);
	MGE_LOG_VERBOSE_3(MGE_LOG_ENGINE, u8"'\n");
}

void mge_close_resource(void * access)
{
	MGL_DEBUG_ASSERT(access != NULL);

	mge_resource_t* rsc = ((mge_resource_access_base_t*)access)->rsc;
	if (rsc == NULL)
		mge_fatal_error(MGE_LOG_ENGINE, u8"Failed to close resource access (resource access resource is NULL)");

	// Lock data mutex
	mgl_error_t err = mgl_lock_mutex(&rsc->data.mutex);
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to lock resource data mutex", err);

	MGE_LOG_VERBOSE_3(MGE_LOG_ENGINE, u8"Closed resource '");
	MGE_LOG_VERBOSE_3(MGE_LOG_ENGINE, rsc->name);
	MGE_LOG_VERBOSE_3(MGE_LOG_ENGINE, u8"'\n");

	// Decrease ref count
	rsc->data.reference_count -= 1;

	// Unload resource
	if (rsc->data.reference_count == 0 && !(rsc->hints & MGE_RESOURCE_HINT_PERMANENT))
	{
		mge_force_resource_unload(rsc);
		mge_resource_unload_dependencies(rsc);
	}

	// Unlock data mutex
	err = mgl_unlock_mutex(&rsc->data.mutex);
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to unlock resource data mutex", err);
}
