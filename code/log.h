/* date = January 26th 2022 0:24 pm */

#ifndef _LOG_H
#define _LOG_H

typedef enum _log_level
{
	LOG_FATAL,
	LOG_ERROR,
	LOG_WARN,
	LOG_DEBUG,
	LOG_INFO
} log_level;

void _log(log_level level, const char *format, ...);

void initialize_logger();

#define V_FATAL(format, ...) _log(LOG_FATAL, format, ##__VA_ARGS__)
#define V_FAIL(format, ...) _log(LOG_ERROR, format, ##__VA_ARGS__)
#define V_WARN(format, ...) _log(LOG_WARN, format, ##__VA_ARGS__)
#define V_INFO(format, ...) _log(LOG_INFO, format, ##__VA_ARGS__)

#if defined(DEBUG)
#define V_DEBUG(format, ...) _log(LOG_DEBUG, format, #__VA_ARGS__)
#else
#define V_DEBUG(format, ...)
#endif

#endif //_LOG_H
