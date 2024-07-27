/*
Author: Abraham Rodriguez.
Date: July 25, 2024

Description:

Writer must print its own PID and await any input text.
The process sends data through a named FIFO onto the reader.

The format is DATA:XXXXXX

The process can await SIGUSR1 and SIGUSR2; this should prompt an output:
    SIGN:1/SIGN:2.

The writer should detect EOF and gracefully terminate.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "logger.h"

#define FIFO_NAME "myfifo"

const char *fifo = "myfifo";

int fifo_fd;
BaseLogger *logger;

// helper function to send a message via fifo
int send_message(const char *buffer) {
    int num;
    if ((num = write(fifo_fd, buffer, strlen(buffer))) == -1) {
        return -1;
    }
    logger->log_message(logger, LOG_INFO, "writer: wrote %d bytes", num);
    return 0;
}

// Handler for SIGUSR1/SIGUSR2
void signal_handler(int signal) {
    char message[32];
    if (signal == SIGUSR1) {
        logger->log_message(logger, LOG_INFO, "SIGN:1");
        sprintf(message, "SIGN:1");

        if (send_message(message) == -1) {
            logger->log_message(logger, LOG_ERROR, "error sending SIGN:1");
        }

    } else if (signal == SIGUSR2) {
        logger->log_message(logger, LOG_INFO, "SIGN:2");
        sprintf(message, "SIGN:2");

        if (send_message(message) == -1) {
            logger->log_message(logger, LOG_ERROR, "error sending SIGN:2");
        }
    }
}

// Checks if FIFO exists
int isFIFOCreated() {
    struct stat st;
    if (stat(FIFO_NAME, &st) != 0) {
        return -1;
    }
    logger->log_message(logger, LOG_INFO, "fifo: %s found", FIFO_NAME);
    return 0;
}

// Function that opens and existing FIFO if found, if not found then it creates a new fifo.
int openFIFO(int *pid ){
    // checking for existing FIFO
    if (isFIFOCreated() == -1) {
        logger->log_message(logger, LOG_INFO, "creating named fifo...");
        if (mkfifo(fifo, 0666) == -1) {
            logger->log_message(logger, LOG_ERROR, "error while making named fifo on pid %d", *pid);
            return -1;
        }
        logger->log_message(logger, LOG_INFO, "fifo: %s created successfully", FIFO_NAME);
    }
    // Opening FIFO
    if ((fifo_fd = open(fifo, O_WRONLY)) == -1) {
        logger->log_message(logger, LOG_ERROR, "error while opening fifo on pid %d", *pid);
        return -1;
    }
    return 0;
}

int main() {
    char buffer[1024];
    // logger instance
    logger = create_base_logger();
    if (logger == NULL) {
        perror("Failed to create logger");
        return -1;
    }
    // print PID
    pid_t pid;
    pid = getpid();
    logger->log_message(logger, LOG_INFO, "PID: %d", pid);

    // checking for existing FIFO
    if (openFIFO(&pid) == -1){
        logger->log_message(logger, LOG_ERROR, "error while opening fifo");
        return -1;
    }

    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);
    logger->log_message(logger, LOG_INFO, "waiting for readers...");

    // Write loop 
    while (1) {
        printf("input text: ");
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (strncmp(buffer, "EOF", 3) == 0) {
                logger->log_message(logger, LOG_INFO, "detected OOF, terminating gracefully");
                break;
            }

            char message[1050];
            logger->log_message(logger, LOG_INFO, "sending message: %s", buffer);
            snprintf(message, sizeof(message), "DATA:%s", buffer);

            if (send_message(message) == -1) {
                logger->log_message(logger, LOG_ERROR, "write error");
            }

        } else {
            if (feof(stdin)) {
                break;
            }
        }
    }
    logger->log_message(logger, LOG_INFO, "terminating gracefully");
    close(fifo_fd);
    unlink(fifo);

}
