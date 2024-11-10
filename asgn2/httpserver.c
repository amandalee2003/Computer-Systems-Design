//httpserver.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <regex.h>

#include "asgn2_helper_funcs.h"

#define BUFSIZE 4096
#define REQEX   "^([a-zA-Z]{1,8}) /([a-zA-Z0-9.-]{1,63}) (HTTP/[0-9]\\.[0-9])\r\n"
#define REGEX2  "([a-zA-Z0-9.-]{1,128}): ([ -~]{1,128})\r\n"

typedef struct Request {
    int fd;
    int content_length;
    int remainings;
    char *command;
    char *path;
    char *version;
    char *messages_body;
} Request;

int read_n_until(int fd, char *buffer, size_t max_size) {
    char characters;
    size_t i = 0;
    char delimiters[] = "\r\n\r\n";
    int matched = 0;
    while (i < (max_size - 1)) {
        if (read(fd, &characters, 1) != 1) {
            break;
        }
        buffer[i++] = characters;
        if (characters == delimiters[matched]) {
            matched = matched + 1;
            if (matched == sizeof(delimiters) - 1) {

                buffer[i] = '\0';
                return 1;
            }
        } else {
            matched = 0;
        }
    }
    buffer[i] = '\0';
    return 0;
}

int write_n_until(int out, char *buf, int n) {
    int bytes_written = 0;
    while (bytes_written < n) {
        ssize_t result = write(out, buf + bytes_written, n - bytes_written);
        if (result == -1) {
            if (errno == EINTR) {
                continue;
            }
            if (errno != EINTR) {
                return 1;
            }
        }
        bytes_written = bytes_written + result;
    }
    return bytes_written;
}

int get(Request *R) {
    int file;
    int bytes_written;
    off_t size;
    struct stat s_buf;
    if (R->content_length != -1 || R->remainings > 0) {
        dprintf(R->fd, "HTTP/1.1 400 Bad Request\r\nContent-Length: %d\r\n\r\nBad Request\n", 12);
        return 1;
    }
    if ((file = open(R->path, O_RDONLY | O_DIRECTORY)) != -1) {
        dprintf(R->fd, "HTTP/1.1 403 Forbidden\r\nContent-Length: %d\r\n\r\nForbidden\n", 10);
        return 1;
    }

    if ((file = open(R->path, O_RDONLY)) == -1) {
        if (errno == EACCES) {
            dprintf(R->fd, "HTTP/1.1 403 Forbidden\r\nContent-Length: %d\r\n\r\nForbidden\n", 10);
        } else if (errno == ENOENT) {
            dprintf(R->fd, "HTTP/1.1 404 Not Found\r\nContent-Length: %d\r\n\r\nNot Found\n", 10);
        }
        if ((file = open(R->path, O_RDONLY)) != -1) {
            dprintf(R->fd,
                "HTTP/1.1 500 Internal Server Error\r\nContent-Length: %d\r\n\r\nInternal Server "
                "Error\n",
                22);
        }
        return 1;
    }
    fstat(file, &s_buf);
    size = s_buf.st_size;

    dprintf(R->fd, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n", size);
    bytes_written = pass_n_bytes(file, R->fd, size);
    if (bytes_written == -1) {
        dprintf(R->fd,
            "HTTP/1.1 500 Internal Server Error\r\nContent-Length: %d\r\n\r\nInternal Server "
            "Error\n",
            22);
        return 1;
    }
    close(file);
    return 0;
}

int put(Request *R) {
    int file;
    int status_code = 0;
    int bytes_written;
    int total_bytes_written;
    if (R->content_length == -1) {
        dprintf(R->fd, "HTTP/1.1 400 Bad Request\r\nContent-Length: %d\r\n\r\nBad Request\n", 12);
        return 1;
    }
    if ((file = open(R->path, O_WRONLY | O_DIRECTORY, S_IRUSR | S_IWUSR)) != -1) {
        dprintf(R->fd, "HTTP/1.1 403 Forbidden\r\nContent-Length: %d\r\n\r\nForbidden\n", 10);
        return 1;
    }

    if ((file = open(R->path, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) == -1) {
        if (errno == EACCES) {
            dprintf(R->fd, "HTTP/1.1 403 Forbidden\r\nContent-Length: %d\r\n\r\nForbidden\n", 10);
            return 1;
        } else if (errno == EEXIST) {
            status_code = 200;
        } else {
            //fprintf(stderr, "HERE\n");
            dprintf(R->fd,
                "HTTP/1.1 500 Internal Server Error\r\nContent-Length: %d\r\n\r\nInternal Server "
                "Error\n",
                22);
            return 1;
        }
    }
    if ((file = open(R->path, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) != -1) {
        status_code = 201;
    }
    if (status_code == 200) {
        if ((file = open(R->path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) == -1) {
            if (errno == EACCES) {
                dprintf(
                    R->fd, "HTTP/1.1 403 Forbidden\r\nContent-Length: %d\r\n\r\nForbidden\n", 10);
                return 1;
            }
            if (errno != EACCES) {
                //fprintf(stderr,"HERE\n");
                dprintf(R->fd,
                    "HTTP/1.1 500 Internal Server Error\r\nContent-Length: %d\r\n\r\nInternal "
                    "Server Error\n",
                    22);
                return 1;
            }
        }
    }
    bytes_written = write_n_bytes(file, R->messages_body, R->remainings);
    if (bytes_written == -1) {
        //fprintf(stderr,"HERE\n");
        //dprintf(R->fd, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\nOK\n", 3);
        dprintf(R->fd,
            "HTTP/1.1 500 Internal Server Error\r\nContent-Length: %d\r\n\r\nInternal Server "
            "Error\n",
            22);
        return 1;
    }
    total_bytes_written = R->content_length - R->remainings;
    bytes_written = pass_n_bytes(R->fd, file, total_bytes_written);
    if (bytes_written == -1) {
        //printf("HERE\n");
        dprintf(R->fd,
            "HTTP/1.1 500 Internal Server Error\r\nContent-Length: %d\r\n\r\nInternal Server "
            "Error\n",
            22);
        return 1;
    }
    if (status_code == 201) {
        dprintf(R->fd, "HTTP/1.1 201 Created\r\nContent-Length: %d\r\n\r\nCreated\n", 8);
    }
    if (status_code != 201) {
        dprintf(R->fd, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\nOK\n", 3);
    }
    close(file);
    return 0;
}

int parse_request(Request *R, char *buf, ssize_t bytes_read) {
    int total_offset = 0;
    int rc;
    int value;
    regex_t re;
    regmatch_t pair[4];

    rc = regcomp(&re, REQEX, REG_EXTENDED);
    rc = regexec(&re, buf, 4, pair, 0);
    if (rc == 0) {
        R->command = buf;
        R->path = buf;
        R->version = buf;
        R->path += pair[2].rm_so;
        R->version += pair[3].rm_so;
        buf[pair[1].rm_eo] = '\0';
        R->path[pair[2].rm_eo - pair[2].rm_so] = '\0';
        R->version[pair[3].rm_eo - pair[3].rm_so] = '\0';
        buf = buf + pair[3].rm_eo;
        buf = buf + 2;
        total_offset = total_offset + pair[3].rm_eo;
        total_offset = total_offset + 2;
    }
    if (rc != 0) {
        dprintf(R->fd, "HTTP/1.1 400 Bad Request\r\nContent-Length: %d\r\n\r\nBad Request\n", 12);
        regfree(&re);
        return 1;
    }
    R->content_length = -1;
    rc = regcomp(&re, REGEX2, REG_EXTENDED);
    rc = regexec(&re, buf, 3, pair, 0);
    while (rc == 0) {
        buf[pair[1].rm_eo] = '\0';
        buf[pair[2].rm_eo] = '\0';
        if (strncmp(buf, "Content-Length", 14) == 0) {
            value = strtol(buf + pair[2].rm_so, NULL, 10);
            if (errno == EINVAL) {
                dprintf(R->fd,
                    "HTTP/1.1 400 Bad Request\r\nContent-Length: %d\r\n\r\nBad Request\n", 12);
            }
            R->content_length = value;
        }
        buf = buf + pair[2].rm_eo;
        buf = buf + 2;
        total_offset = total_offset + pair[2].rm_eo;
        total_offset = total_offset + 2;
        rc = regexec(&re, buf, 3, pair, 0);
    }
    if ((rc != 0) && (buf[0] == '\r' && buf[1] == '\n')) {
        R->messages_body = buf + 2;
        total_offset = total_offset + 2;
        R->remainings = bytes_read - total_offset;
    } else if (rc != 0) {
        dprintf(R->fd, "HTTP/1.1 400 Bad Request\r\nContent-Length: %d\r\n\r\nBad Request\n", 12);
        regfree(&re);
        return 1;
    }
    regfree(&re);
    return 0;
}

int requests(Request *R) {
    if (strncmp(R->version, "HTTP/1.1", 8) != 0) {
        dprintf(R->fd,
            "HTTP/1.1 505 Version Not Supported\r\nContent-Length: %d\r\n\r\nVersion Not "
            "Supported\n",
            22);
        return 1;
    } else if (strncmp(R->command, "GET", 3) == 0) {
        return (get(R));
    } else if (strncmp(R->command, "PUT", 3) == 0) {
        return (put(R));
    } else {
        dprintf(R->fd,
            "HTTP/1.1 501 Not Implemented\r\nContent-Length: %d\r\n\r\nNot Implemented\n", 16);
        return 1;
    }
}

int main(int argc, char *argv[]) {
    Listener_Socket socket;
    Request R;
    int socket_status;
    int sock_fd;
    ssize_t bytes_read;
    int port;

    if (argc != 2) {
        return 1;
    }
    char buf[BUFSIZE + 1] = { '\0' };

    port = strtol(argv[1], NULL, 10);
    if (errno == EINVAL) {
        fprintf(stderr, "Invalid Port\n");
        return 1;
    }
    socket_status = listener_init(&socket, port);
    if (socket_status == -1) {
        fprintf(stderr, "Invalid Port\n");
        return 1;
    }

    while (1) {
        sock_fd = listener_accept(&socket);
        if (sock_fd == -1) {
            fprintf(stderr, "Unable to Establish Connection\n");
            return 1;
        }
        R.fd = sock_fd;
        bytes_read = read_n_until(sock_fd, buf, BUFSIZE);
        if (bytes_read == -1) {
            dprintf(
                R.fd, "HTTP/1.1 400 Bad Request\r\nContent-Length: %d\r\n\r\nBad Request\n", 12);
            return 1;
        }
        if (parse_request(&R, buf, bytes_read) != EXIT_FAILURE) {
            requests(&R);
        }
        close(sock_fd);
        memset(buf, '\0', sizeof(buf));
    }
    return 0;
}
