#include <mge/log.h>

#include <mgl/memory/allocator.h>
#include <mgl/file/logger.h>

static mgl_logger_t* mge_engine_logger = NULL;
static mgl_logger_t* mge_game_client_logger = NULL;
static mgl_logger_t* mge_game_server_logger = NULL;

void mge_internal_init_log(void)
{
	mge_engine_logger = mgl_logger_open(mgl_standard_allocator, u8"engine");
	mge_game_client_logger = mgl_logger_open(mgl_standard_allocator, u8"game_client");
	mge_game_server_logger = mgl_logger_open(mgl_standard_allocator, u8"game_server");
}

void mge_internal_terminate_log(void)
{
	mgl_logger_close(mge_game_server_logger);
	mgl_logger_close(mge_game_client_logger);
	mgl_logger_close(mge_engine_logger);
}

void mge_log(mgl_enum_t log, const mgl_chr8_t * msg)
{
	MGL_DEBUG_ASSERT(msg != NULL);

	switch (log)
	{
		case MGE_LOG_ENGINE: mgl_logger_add(mge_engine_logger, msg); return;
		case MGE_LOG_GAME_CLIENT: mgl_logger_add(mge_game_client_logger, msg); return;
		case MGE_LOG_GAME_SERVER: mgl_logger_add(mge_game_server_logger, msg); return;
		default: mge_fatal_error(MGE_LOG_ENGINE, u8"Failed to log message, invalid log channel"); return;
	}
}

void mge_fatal_error(mgl_enum_t log, const mgl_chr8_t * msg)
{
	MGL_DEBUG_ASSERT(msg != NULL);
	
	mge_log(log, u8"FATAL ERROR: ");
	mge_log(log, msg);
	mge_log(log, u8"\n");

	mge_internal_terminate_log();
	mgl_abort();
}

void mge_fatal_mgl_error(mgl_enum_t log, const mgl_chr8_t * msg, mgl_error_t err)
{
	MGL_DEBUG_ASSERT(msg != NULL);
	
	mge_log(log, u8"FATAL MGL ERROR: ");
	mge_log(log, msg);
	mge_log(log, u8" (");
	mge_log(log, mgl_get_error_string(err));
	mge_log(log, u8")");
	mge_log(log, u8"\n");

	mge_internal_terminate_log();
	mgl_abort();
}

void mge_fatal_mrl_error(mgl_enum_t log, const mgl_chr8_t * msg, mrl_error_t err)
{
	MGL_DEBUG_ASSERT(msg != NULL);

	mge_log(log, u8"FATAL MRL ERROR: ");
	mge_log(log, msg);
	mge_log(log, u8" (");
	mge_log(log, mrl_get_error_string(err));
	mge_log(log, u8")");
	mge_log(log, u8"\n");

	mge_internal_terminate_log();
	mgl_abort();
}
