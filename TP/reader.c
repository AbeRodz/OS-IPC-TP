#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "logger.h"

#define FIFO_NAME "myfifo"

FileLogger *logger, *signLogger;


int main(void) {
    logger = create_file_logger("Log.txt");
    signLogger = create_file_logger("Sign.txt");
    if ((logger == NULL) ||  (signLogger == NULL)) {
        perror("Failed to create loggers");
        return -1;
    }
    
    char s[1024 ];
    int num, fd;

    logger->base.log_message((BaseLogger *)logger, LOG_INFO,"waiting for writers...\n");

    fd = open(FIFO_NAME, O_RDONLY);
    if (fd == -1) {
        logger->base.log_message((BaseLogger *)logger, LOG_ERROR,"open");

        return -1;
    }
    logger->base.log_message((BaseLogger *)logger, LOG_INFO,"got a writer\n");


    while(1){

    
    
    if ((num = read(fd, s, sizeof(s))) == -1){
        logger->base.log_message((BaseLogger *)logger, LOG_ERROR,"read\n");


          return -1;
    }
    else {
        s[num] = '\0';
        logger->base.log_message((BaseLogger *)logger, LOG_INFO,"message received: %s", s);
        logger->base.log_message((BaseLogger *)logger, LOG_INFO,"reader: read %d bytes", num);


    }
    } 
    close_file_logger(logger);
    close_file_logger(signLogger);
    close(fd);
    return 0;
}