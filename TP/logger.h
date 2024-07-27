#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

// Log levels
typedef enum {
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LogLevel;

// BaseLogger 
typedef struct {
    void (*log_message)(void *self, LogLevel level, const char *format, ...);
} BaseLogger;

// FileLogger 
typedef struct {
    BaseLogger base;
    FILE *file;
} FileLogger;

// Logger Prototypes
BaseLogger* create_base_logger(void);
FileLogger* create_file_logger(const char *filename);
void close_file_logger(FileLogger *logger);

#endif // LOGGER_H
