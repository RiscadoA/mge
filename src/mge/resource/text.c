#include <mge/resource/text.h>
#include <mge/log.h>

#include <mgl/file/archive.h>
#include <mgl/memory/allocator.h>

void mge_resource_load_text(void* allocator, mge_resource_t * rsc)
{
	MGL_DEBUG_ASSERT(allocator != NULL && rsc != NULL && rsc->type == MGE_RESOURCE_TEXT);

	// Find and open file
	mgl_iterator_t file;
	mgl_error_t err = mgl_file_find(rsc->data.path, &file);
	if (err != MGL_ERROR_NONE)
	{
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"Couldn't find text resource data file on '");
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, rsc->data.path);
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"'\n");
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to load text resource data file, file not found", err);
	}

	mgl_file_stream_t stream;
	err = mgl_file_open(&file, &stream, MGL_FILE_READ);
	if (err != MGL_ERROR_NONE)
	{
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"Couldn't open text resource data file on '");
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, rsc->data.path);
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"'\n");
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to load text resource data file, couldn't open file", err);
	}

	// Seek data offset
	err = mgl_seek_r(&stream, (mgl_i64_t)rsc->data.offset, MGL_STREAM_SEEK_BEGIN);
	if (err != MGL_ERROR_NONE)
	{
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"Couldn't seek to offset on text resource data file on '");
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, rsc->data.path);
		MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"'\n");
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to seek to offset on text resource data file", err);
	}

	// Read data
	mgl_u64_t text_size;
	err = mgl_read(&stream, &text_size, sizeof(text_size), NULL);
	if (err != MGL_ERROR_NONE)
		goto read_error;

	mge_text_resource_data_t* data;
	err = mgl_allocate(allocator, sizeof(mge_text_resource_data_t) + text_size + 1, (void**)&data);
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to allocate text resource data", err);
	
	data->allocator = allocator;
	data->size = text_size;
	data->text = (mgl_u8_t*)data + sizeof(mge_text_resource_data_t);

	*((mgl_u8_t*)data + sizeof(mge_text_resource_data_t) + text_size) = 0;
	err = mgl_read(&stream, (mgl_u8_t*)data + sizeof(mge_text_resource_data_t), text_size, NULL);
	if (err != MGL_ERROR_NONE)
		goto read_error;

	rsc->data.ptr = data;

	// Close file
	mgl_file_close(&stream);

	return;

read_error:
	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"Failed to read text resource data file on '");
	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, rsc->data.path);
	MGE_LOG_VERBOSE_0(MGE_LOG_ENGINE, u8"'\n");
	mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to read text resource data file", err);
}

void mge_resource_unload_text(mge_resource_t * rsc)
{
	MGL_DEBUG_ASSERT(rsc != NULL && rsc->type == MGE_RESOURCE_TEXT);

	mge_text_resource_data_t* data = (mge_text_resource_data_t*)rsc->data.ptr;
	mgl_error_t err = mgl_deallocate(data->allocator, data);
	if (err != MGL_ERROR_NONE)
		mge_fatal_mgl_error(MGE_LOG_ENGINE, u8"Failed to deallocate text resource data", err);
}

void mge_resource_access_text(mge_resource_t * rsc, mge_text_resource_access_t * access)
{
	MGL_DEBUG_ASSERT(rsc != NULL && access != NULL && rsc->type == MGE_RESOURCE_TEXT);

	access->base.rsc = rsc;
	access->data = (mge_text_resource_data_t*)rsc->data.ptr;
}
