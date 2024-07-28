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
#include <errno.h>
#include <signal.h>
#include "logger.h"

#define FIFO_NAME "myfifo"

const char *fifo = "myfifo";

int fifo_fd;
BaseLogger *logger;
// volatile check if reader is closed
volatile int is_reader_closed = 0;

// helper function to send a message via fifo
int send_message(const char *buffer) {
    int num;
    if ((num = write(fifo_fd, buffer, strlen(buffer))) == -1) {
        logger->log_message(logger, LOG_ERROR, "Error while writing to fifo");
        // Broken pipe, means no readers are available.
        if (errno == EPIPE) {
            logger->log_message(logger, LOG_ERROR, "Broken pipe: no reader available.");
            is_reader_closed = 1;
        }
        return -1;
    }
    logger->log_message(logger, LOG_INFO, "Writer: wrote %d bytes", num);
    return 0;
}

// Function that detects a signal action.
void signal_action(int signal, void(*handler)(int)){
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(signal, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

// Handler for SIGUSR1/SIGUSR2
void signal_handler(int signal) {
    char message[32];
    if (signal == SIGUSR1) {
        logger->log_message(logger, LOG_INFO, "SIGN:1");
        sprintf(message, "SIGN:1");

        if (send_message(message) == -1) {
            logger->log_message(logger, LOG_ERROR, "Error sending SIGN:1");
        }

    } else if (signal == SIGUSR2) {
        logger->log_message(logger, LOG_INFO, "SIGN:2");
        sprintf(message, "SIGN:2");

        if (send_message(message) == -1) {
            logger->log_message(logger, LOG_ERROR, "Error sending SIGN:2");
        }
    }
}

// Checks if FIFO exists
int isFIFOCreated() {
    struct stat st;
    if (stat(FIFO_NAME, &st) != 0) {
        return -1;
    }
    logger->log_message(logger, LOG_INFO, "FIFO: %s found", FIFO_NAME);
    return 0;
}

// Function that opens an existing FIFO if found, if not found then it creates a new fifo.
int openFIFO(int *pid){
    // checking for existing FIFO
    if (isFIFOCreated() == -1) {
        logger->log_message(logger, LOG_INFO, "Creating named FIFO...");
        if (mkfifo(fifo, 0666) == -1) {
            logger->log_message(logger, LOG_ERROR, "Error while making named FIFO on pid %d", *pid);
            return -1;
        }
        logger->log_message(logger, LOG_INFO, "FIFO: %s created successfully", FIFO_NAME);
    }
    // Opening FIFO
    if ((fifo_fd = open(fifo, O_WRONLY)) == -1) {
        logger->log_message(logger, LOG_ERROR, "Error while opening FIFO on pid %d", *pid);
        return -1;
    }
    return 0;
}

// Function to close and unlink fifo resources, means it terminated gracefully;.
void cleanup(){
    close(fifo_fd);
    unlink(fifo);
    logger->log_message(logger, LOG_INFO, "Terminating gracefully");
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
        logger->log_message(logger, LOG_ERROR, "Error while opening FIFO");
        return -1;
    }
    // signal handling
    signal_action(SIGUSR1, signal_handler);
    signal_action(SIGUSR2, signal_handler);

    // According to this post, when the reader terminates it triggers a SIGPIPE which would make a E_PIPE errno.
    //https://stackoverflow.com/questions/67442277/c-to-python-piping-how-to-detect-reader-access
    signal_action(SIGPIPE, SIG_IGN);
    logger->log_message(logger, LOG_INFO, "Waiting for readers...");

    // Write loop 
    while (1) {
        printf("input text: ");
        errno = 0;
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {

            char message[1050];
            logger->log_message(logger, LOG_INFO, "Sending message: %s", buffer);
            snprintf(message, sizeof(message), "DATA:%s", buffer);

            if (send_message(message) == -1) {
                if (is_reader_closed){
                    logger->log_message(logger, LOG_ERROR, "Reader is closed, terminating...");
                    break;
                }
                logger->log_message(logger, LOG_ERROR, "Write error");
                break;
            }

        } else {
            if (feof(stdin)) {
                break;
            }else if (errno == EINTR) {
                clearerr(stdin); 
                continue; 
            } else {
                logger->log_message(logger, LOG_ERROR, "fgets error, terminating...");
                cleanup();
                return -1;
            }
        }
    }
    cleanup();
    return 0;
}
