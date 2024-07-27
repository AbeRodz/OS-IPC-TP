#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "logger.h"
#include <string.h>

#define FIFO_NAME "myfifo"

const char *data_message = "DATA:";
const char *usr_sign_message =  "SIGN:";


FileLogger *logger, *signLogger;

int main(void) {
    // logger instances
    logger = create_file_logger("Log.txt");
    signLogger = create_file_logger("Sign.txt");
    if ((logger == NULL) || (signLogger == NULL)) {
        perror("Failed to create loggers");
        return -1;
    }
    
    char buffer[1024];
    int num, fifo_fd;

    logger->base.log_message((BaseLogger *)logger, LOG_INFO,"waiting for writers...");
    // opening fifo
    fifo_fd = open(FIFO_NAME, O_RDONLY);
    if (fifo_fd == -1) {
        logger->base.log_message((BaseLogger *)logger, LOG_ERROR,"error while opening fifo reader");

        return -1;
    }
    logger->base.log_message((BaseLogger *)logger, LOG_INFO,"got a writer");
    // listener loop
    while(1){
    //reading fifo
        if ((num = read(fifo_fd, buffer, sizeof(buffer))) == -1){
            logger->base.log_message((BaseLogger *)logger, LOG_ERROR,"error while reading");

            return -1;
        }
        
        else if (num > 0){
            buffer[num] = '\0';
            // DATA 
            if (strncmp(buffer, data_message, 5)==0){
                logger->base.log_message((BaseLogger *)logger, LOG_INFO,"data reader: %s", buffer);
                logger->base.log_message((BaseLogger *)logger, LOG_INFO,"reader: read %d bytes", num);
                }
            // SIGNAL 
            else if (strncmp(buffer, usr_sign_message,5)==0){
                signLogger->base.log_message((BaseLogger *)signLogger, LOG_INFO,"signal reader: %s", buffer);
                }
            // undefined or unknown message
            else {
                logger->base.log_message((BaseLogger *)logger, LOG_ERROR,"received undefined message : %s", buffer);
                }
            }
    } 
    // closing loggers and fifo
    close_file_logger(logger);
    close_file_logger(signLogger);
    close(fifo_fd);
    return 0;
}