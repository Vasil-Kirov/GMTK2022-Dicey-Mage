#include "basic.h"
#include "log.h"
#include <time.h>

static const char * const level_labels[] = {
	"[FATAL] ", "[ERROR] ", "[WARN] ", "[DEBUG] ", "[INFO] "
};

static char log_file_path[1024];
static SDL_RWops *log_file;

#ifndef _WIN32
static const int color_codes[] = {129, 197, 11, 76, 230};
#else
static const u8 color_codes[] = {13, 4, 6, FOREGROUND_GREEN | FOREGROUND_INTENSITY, 
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY};
#endif

void
initialize_logger()
{
	strncpy(log_file_path, SDL_GetBasePath(), 1024);
	if(log_file_path == NULL)
	{
		_log(LOG_WARN, "Couldn't initialize logger, failed to get the path to the log file");
		exit(1);
	}
	strcat(log_file_path, "Errors.log");
	log_file = SDL_RWFromFile(log_file_path, "w+");
}

void
_log(log_level level, const char *format, ...)
{
	time_t raw_seconds;
	struct tm *time_info;
	time(&raw_seconds);
	time_info = localtime(&raw_seconds);
	
	const char time_format[] = "(%d/%d %d:%d:%d)";
	char time_str[4096] = {0};
	snprintf(time_str, 4096, time_format, time_info->tm_mday, time_info->tm_mon + 1,
			 time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
	
	
#ifndef _WIN32
	char color[1024] = {0};
	sprintf(color, "\\u001b[38;5;$%dm ", color_codes[level]);
#endif	
	
	char format_copy[4096] = {0};
#ifndef _WIN32
	strcat(format_copy, color);	
#else
	
#define WIN32_LEAN_AND_MEAN 
#include <ConsoleApi2.h>
	HANDLE STDOUT = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(STDOUT, color_codes[level]);
							
#endif
	strcat(format_copy, time_str);
	strcat(format_copy, level_labels[level]);
	strcat(format_copy, format);
	
	char to_print[4096] = {0};
	
	va_list args;
	va_start(args, format);
	
	vsnprintf(to_print, 4096, format_copy, args);
	
	va_end(args);
	
	strcat(to_print, "\n");
	printf(to_print);
	
#ifdef _WIN32
	SetConsoleTextAttribute(STDOUT, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
	
	if(level < LOG_WARN || level == LOG_DEBUG)
	{
		log_file->write(log_file, to_print, (size_t)strlen(to_print), 1);
	}
	
	if(level == LOG_FATAL)
	{
		log_file->close(log_file);
		exit(1);
	}
}

void
clean_up_logger()
{
	log_file->close(log_file);
}
