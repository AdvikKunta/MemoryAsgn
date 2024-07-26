#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <linux/limits.h>
#include <fcntl.h>

void error(void) {
    //function for writing to stderr that there is an Invalid Command and exiting program with code 1
    fprintf(stderr, "Invalid Command\n");
    exit(1);
}

// had MANY comments on previous versions of this file but then it turns out i did it wrong and that I
// had too many comments so i couldn't read my own file LMFAO

void getNow(char *buffer) {
    int bytes_read = 0;
    //collecting input for get
    while (1) {
        int reed = read(0, buffer + bytes_read, PATH_MAX);
        if (reed == -1)
            error();
        if (reed == 0)
            break;
        bytes_read += reed;
    }
    //getting into the parsing of the input
    int expected = strnlen(buffer, bytes_read);
    char *delim = "\n";
    char *location;
    char *token = strtok(buffer, delim);
    location = token;
    int fd = open(location, O_RDONLY, 0);
    if (fd == -1) {
        //throw error if unable to open said file
        error();
    }
    if (strlen(location) > PATH_MAX) {
        error();
    }
    int reality = strlen(location) + 1;
    if (expected != reality) {
        error();
    }

    //do the actual getting part of the function now
    int res = 1;
    int bufRead;
    do {
        int res = read(fd, buffer, PATH_MAX);
        if (res == -1) {
            error();
        }
        if (res == 0) {
            break;
        }
        bufRead = res;

        int write_bytes = write(STDOUT_FILENO, buffer, bufRead);
        if (write_bytes == -1) {
            fprintf(stderr, "Operation Failed\n");
            exit(1);
        }
        while (write_bytes < bufRead) {
            int writeNow = write(STDOUT_FILENO, buffer + write_bytes, bufRead - write_bytes);
            if (writeNow == -1) {
                fprintf(stderr, "Operation Failed\n");
                exit(1);
            }
            write_bytes += writeNow;
        }
    } while (res != 0);
    close(fd);
    exit(0);
}

//helper function for getting the location
int getLocation(void) {
    char loc[PATH_MAX];
    int index = 0;
    while (1) {
        int res = read(0, loc + index, 1);
        if (res == 0) {
            break;
        }
        if (res == -1) {
            error();
        }
        if (strncmp(&loc[index], "\n", 1) == 0) {
            loc[index] = '\0';
            break;
        }
        index += res;
    }

    char *p = loc;
    int fd = open(p, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        error();
    }
    return fd;
}

//helper function to get the length of the content for setting
int getLength(void) {
    char len[30];
    int index = 0;
    int length;
    //looping through to just read all the bytes until the byte i get is a newline character
    while (1) {
        int res = read(0, len + index, 1);
        if (res == 0) {
            break;
        }
        if (res == -1) {
            error();
        }
        //turn the last newline character into a null terminator to make this a c string
        if (strncmp(&len[index], "\n", 1) == 0) {
            len[index] = '\0';
            break;
        }
        index += res;
    }
    //can't input a character array into sscanf (well it didnt work for the previous one so i want to do it here)
    char *p = len;
    sscanf(p, "%d", &length);
    if (!(length >= 0)) {
        error();
    }
    return length;
}

int main(void) {
    char command[4];
    char buffer[PATH_MAX + 1];

    for (int i = 0; i < 4; i++) {
        int commNum = read(0, command + i, 1);
        if (commNum == -1) {
            error();
        }
    }

    if (strcmp(command, "get\n") == 0) {
        getNow(buffer);
    }

    else if (strcmp(command, "set\n") == 0) {

        int fd = getLocation();
        int length = getLength();

        int remaining_bytes = 0;
        while (1) {
            int bytes_read = read(0, buffer, PATH_MAX);
            if (bytes_read == -1) {
                error();
            }
            if (bytes_read == 0) {
                break;
            }

            remaining_bytes = bytes_read;

            if (remaining_bytes > length) {
                remaining_bytes = length;
            }

            int written = write(fd, buffer, remaining_bytes);
            if (written == -1) {
                fprintf(stderr, "Operation Failed\n");
                exit(1);
            }
            while (remaining_bytes > written) {
                int writing = write(fd, buffer, length - written);
                written += writing;
                if (writing == -1) {
                    fprintf(stderr, "Operation Failed\n");
                    exit(1);
                }
            }
        }
        close(fd);
        fprintf(stdout, "OK\n");
    } else {
        error();
    }
    return 0;
}
