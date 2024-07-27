#include "logger.h"
#include <stdlib.h>
#include <string.h>


// Helper function to get log level string for writing into files
const char* get_log_level_str_file(LogLevel level) {
    switch (level) {
        //case LOG_DEBUG: return "DEBUG";
        case LOG_INFO: return "INFO";
        case LOG_WARN: return "WARN";
        case LOG_ERROR: return "ERROR";
        default: return "RESET";
    }
}
// Helper function to get log level string for writing into console
const char* get_log_level_str_console(LogLevel level) {
    switch (level) {
        case LOG_INFO: return GRN "INFO" RESET;
        case LOG_WARN: return YEL "WARN" RESET;
        case LOG_ERROR: return RED "ERROR" RESET;
        default: return WHT "UNKNOWN" RESET;
    }
}
// Helper function for writing onto console
static void console_writer(struct tm *t,va_list *args,LogLevel *level, const char *format, ...){
    printf("[%04d-%02d-%02d %02d:%02d:%02d] %s: ",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec,
        get_log_level_str_console(*level));
    vprintf(format, *args);
    printf("\n");
}
// Helper function for writing onto file
static void file_writer(FileLogger *logger ,struct tm *t,va_list *args,LogLevel *level, const char *format, ...){
        fprintf(logger->file, "[%04d-%02d-%02d %02d:%02d:%02d] %s: ",
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min, t->tm_sec,
                get_log_level_str_file(*level));
        vfprintf(logger->file, format, *args);
        fprintf(logger->file, "\n");
        fflush(logger->file);
}


// Logging function for console-only logging
void log_message_console(void *self, LogLevel level, const char *format, ...) {
    va_list args;
    va_start(args, format);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    console_writer(t,&args,&level,format);

    va_end(args);
}

// Logging function for file logging
void log_message_file(void *self, LogLevel level, const char *format, ...) {
    va_list args;
    va_list Cargs;
    va_start(args, format);
    va_start(Cargs, format);

    FileLogger *logger = (FileLogger *)self;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    console_writer(t,&Cargs,&level,format);

    if (logger->file) {
        file_writer(logger,t,&args,&level,format);
    }

    va_end(Cargs);
    va_end(args);
}

// Create base logger that logs to the console only
BaseLogger* create_base_logger(void) {
    BaseLogger *logger = (BaseLogger *) malloc(sizeof(BaseLogger));
    if (logger == NULL) {
        perror("Failed to allocate memory for base logger");
        return NULL;
    }

    logger->log_message = log_message_console;
    return logger;
}

// Create file logger that logs to a file and console
FileLogger* create_file_logger(const char *filename) {
    FileLogger *logger = (FileLogger *) malloc(sizeof(FileLogger));
    if (logger == NULL) {
        perror("Failed to allocate memory for file logger");
        return NULL;
    }

    logger->file = fopen(filename, "a");
    if (logger->file == NULL) {
        perror("Failed to open log file");
        free(logger);
        return NULL;
    }

    logger->base.log_message = log_message_file;
    return logger;
}

// Closes the file logger
void close_file_logger(FileLogger *logger) {
    if (logger->file) {
        fclose(logger->file);
    }
    free(logger);
}
