#ifndef MGE_LOG_H
#define MGE_LOG_H
#ifdef __cplusplus
extern "C" {
#endif

#include <mgl/error.h>
#include <mrl/error.h>

enum
{
	MGE_LOG_ENGINE,
	MGE_LOG_GAME_CLIENT,
	MGE_LOG_GAME_SERVER,
};

void mge_internal_init_log(void);

void mge_internal_terminate_log(void);

void mge_log(mgl_enum_t log, const mgl_chr8_t* msg);

void mge_fatal_error(mgl_enum_t log, const mgl_chr8_t* msg);

void mge_fatal_mgl_error(mgl_enum_t log, const mgl_chr8_t* msg, mgl_error_t err);

void mge_fatal_mrl_error(mgl_enum_t log, const mgl_chr8_t* msg, mrl_error_t err);

#define MGE_LOG_VERBOSE_0(log, msg) do { mge_log(log, msg); } while (0)
	
#if MGE_VERBOSE_LEVEL >= 1
#	define MGE_LOG_VERBOSE_1(log, msg) do { mge_log(log, msg); } while (0)
#else
#	define MGE_LOG_VERBOSE_1(log, msg) do {} while (0)
#endif

#if MGE_VERBOSE_LEVEL >= 2
#	define MGE_LOG_VERBOSE_2(log, msg) do { mge_log(log, msg); } while (0)
#else
#	define MGE_LOG_VERBOSE_2(log, msg) do {} while (0)
#endif

#if MGE_VERBOSE_LEVEL >= 3
#	define MGE_LOG_VERBOSE_3(log, msg) do { mge_log(log, msg); } while (0)
#else
#	define MGE_LOG_VERBOSE_3(log, msg) do {} while (0)
#endif

#ifdef __cplusplus
}
#endif
#endif
