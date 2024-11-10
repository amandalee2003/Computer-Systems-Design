#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096

int main() {
    char buffer[BUFFER_SIZE];
    //char buf[BUFFER_SIZE];
    char *token;
    int n; //,content_length;//temporaryly commented out

    n = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
    if (n < 0) {
        fprintf(stderr, "Invalid Command\n");
        return 1;
    }
    buffer[n] = '\0';

    token = strtok(buffer, "\n");

    if (token == NULL) {
        fprintf(stderr, "Invalid Command\n");
        return 1;
    }

    if (strcmp(token, "get") == 0 && buffer[n - 1] == '\n') {
        token = strtok(NULL, "\n");
        if (token == NULL) {
            fprintf(stderr, "Invalid Command\n");
            return 1;
        }

        //filename = token;
        char *last = strtok(NULL, "\n");

        if (last != NULL) {
            fprintf(stderr, "Invalid Command\n");
            return 1;
        }

        int fd; //////////////////////////, read_bytes;
        char get_buffer[BUFFER_SIZE];

        fd = open(token, O_RDONLY);

        if (fd < 0) {
            fprintf(stderr, "Invalid Command\n");
            return 1;
        }

        int bytes_read = 0;
        do {
            bytes_read = read(fd, get_buffer, BUFFER_SIZE - 1);
            if (bytes_read < 0) {
                fprintf(stderr, "Invalid Command\n");
                return 1;
            } else if (bytes_read > 0) {
                int bytes_written = 0;
                do {
                    int bytes = write(
                        STDOUT_FILENO, get_buffer + bytes_written, bytes_read - bytes_written);
                    if (bytes <= 0) {
                        fprintf(stderr, "Operation Failed\n");
                        return 1;
                    }
                    bytes_written += bytes;
                } while (bytes_written < bytes_read);
            }
        } while (bytes_read > 0);

        close(fd);
    }

    else if (strcmp(token, "set") == 0) {
        char *filename = NULL;
        int content_length = 0;
        //char contents[BUFFER_SIZE];

        token = strtok(NULL, "\n");
        if (token == NULL) {
            fprintf(stderr, "Invalid Command\n");
            return 1;
        }
        filename = token;

        token = strtok(NULL, "\n");
        if (token == NULL) {
            fprintf(stderr, "Invalid Command\n"); ////////////////commented out?????????
            return 1;
        }

        content_length = atoi(token);

        token = strtok(buffer, "\n"); //buffer???????????????
        if (token == NULL) {
            fprintf(stderr, "Invalid Command\n");
            return 1;
        }

        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

        if (fd < 0) {
            fprintf(stderr, "Invalid Command\n");
            return 1;
        }

        int written_bytes = 0;
        while (written_bytes < content_length) {
            int writtenBytes = write(fd, buffer + written_bytes, content_length - written_bytes);
            if (writtenBytes == -1) {
                fprintf(stderr, "Operation Failed\n");
                close(fd);
                return 1;
            }
            written_bytes += writtenBytes;
        }
        /*  int bytes_read = 0;
    do {
	bytes_read = read(fd, buffer, BUFFER_SIZE-1);
	if (bytes_read < 0) {
	    fprintf(stderr, "Invalid Command\n");
	    return 1;
	}
	else if (bytes_read > 0) {
	    int bytes_written = 0;
	    do {
		int bytes = write(STDOUT_FILENO,
		buffer + bytes_written,
		content_length - bytes_written);
	        if (bytes <= 0) {
		    fprintf(stderr, "Operation Failed\n");
		    return 1;
		}
		bytes_written += bytes;
	    } while(bytes_written < bytes_read);
	}
    } while (bytes_read > 0);
*/

        close(fd);
        printf("OK\n");
    } else {
        fprintf(stderr, "Invalid Command\n");
        return 1;
    }

    return 0;
}
