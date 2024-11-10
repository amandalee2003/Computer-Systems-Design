#include "asgn2_helper_funcs.h"
#include "connection.h"
#include "debug.h"
#include "queue.h"
#include "request.h"
#include "response.h"
#include "rwlock.h"

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXLINE     1024
#define NUM_THREADS 4
#define OPTIONS     "t:"

queue_t *queue;

pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t audit = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t plock = PTHREAD_MUTEX_INITIALIZER;

void handle_connection(int);
void handle_get(conn_t *);
void handle_put(conn_t *);
void handle_unsupported(conn_t *, const Request_t *);
void audit_log(const char *, conn_t *, const Response_t *);
void *worker();

void handle_connection(int connfd) {
    conn_t *connection = conn_new(connfd);
    const Response_t *response = conn_parse(connection);
    if (response != NULL) {
        conn_send_response(connection, response);
    } else {
        const Request_t *request = conn_get_request(connection);
        if (request == &REQUEST_GET) {
            handle_get(connection);
        } else if (request == &REQUEST_PUT) {
            handle_put(connection);
        } else {
            handle_unsupported(connection, request);
        }
    }
    conn_delete(&connection);
}

void handle_get(conn_t *conn) {
    char *uri = conn_get_uri(conn);
    const Response_t *res = NULL;
    pthread_mutex_lock(&qlock);
    int fd = open(uri, O_RDONLY, 0666);
    if (fd < 0) {
        if (errno == EACCES) {
            res = &RESPONSE_FORBIDDEN;
        } else if (errno == ENOENT) {
            res = &RESPONSE_NOT_FOUND;
        } else {
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
        }
        pthread_mutex_unlock(&qlock);
        goto out_get;
    }

    //flock(fd, LOCK_SH);
    //rw shared lock goes here (read)

    pthread_mutex_unlock(&qlock);
    struct stat status;
    if (fstat(fd, &status) < 0) {
        res = &RESPONSE_INTERNAL_SERVER_ERROR;
        goto out_get;
    }
    uint64_t size = status.st_size;
    if (S_ISDIR(status.st_mode)) {
        res = &RESPONSE_FORBIDDEN;
        goto out_get;
    }
    res = conn_send_file(conn, fd, size);
    if (res == NULL) {
        res = &RESPONSE_OK;
        audit_log("GET", conn, res);
    }
    close(fd);
    return;
out_get:
    audit_log("GET", conn, res);
    conn_send_response(conn, res);
    close(fd);
}

void handle_unsupported(conn_t *conn, const Request_t *req) {
    conn_send_response(conn, &RESPONSE_NOT_IMPLEMENTED);
    const char *method = request_get_str(req);
    audit_log(method, conn, &RESPONSE_NOT_IMPLEMENTED);
}

void handle_put(conn_t *conn) {
    char *uri = conn_get_uri(conn);
    const Response_t *res = NULL;
    bool existed = access(uri, F_OK) == 0;
    pthread_mutex_lock(&plock);
    int fd = open(uri, O_CREAT | O_WRONLY, 0600);
    if (fd < 0) {
        if (errno == EACCES || errno == EISDIR || errno == ENOENT) {
            res = &RESPONSE_FORBIDDEN;
            pthread_mutex_unlock(&plock);
            goto out;
        } else {
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            pthread_mutex_unlock(&plock);
            goto out;
        }
    }
    //flock(fd, LOCK_EX);
    //rw exclusive lock goes here (write)
    ftruncate(fd, 0);
    //
    pthread_mutex_unlock(&plock);
    res = conn_recv_file(conn, fd);
    if (res == NULL && existed) {
        res = &RESPONSE_OK;
    } else if (res == NULL && !existed) {
        res = &RESPONSE_CREATED;
    }
    audit_log("PUT", conn, &RESPONSE_OK);
    close(fd);
    conn_send_response(conn, res);
    return;
out:
    audit_log("PUT", conn, &RESPONSE_OK);
    conn_send_response(conn, res);
    close(fd);
}

void *worker() {
    uintptr_t socket_fd = 0;
    while (1) {

        queue_pop(queue, (void **) &socket_fd);
        handle_connection(socket_fd);
        close(socket_fd);
    }
}

void audit_log(const char *method, conn_t *conn, const Response_t *res) {
    pthread_mutex_lock(&audit);
    const char *uri = conn_get_uri(conn);
    const char *request_id = conn_get_header(conn, "Request-Id");
    if (request_id == NULL) {
        request_id = "0";
    }
    uint16_t status_code = response_get_code(res);
    fprintf(stderr, "%s,%s,%d,%s\n", method, uri, status_code, request_id);
    pthread_mutex_unlock(&audit);
    return;
}

int main(int argc, char **argv) {
    int opt = 0;
    int num_threads = NUM_THREADS;
    char *endptr = NULL;
    size_t port = (size_t) strtoull(argv[1], &endptr, 10);
    size_t num_port;

    if (argc < 2) {
        warnx("wrong arguments: %s port_num", argv[0]);
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 't':
            num_threads = atoi(optarg);
            port = (size_t) strtol(argv[3], NULL, 10);
            break;

        default:
            num_port = strtol(argv[optind], &endptr, 10);
            if (endptr && *endptr != '\0') {
                warnx("invalid port number: %s", argv[optind]);
                return EXIT_FAILURE;
            }
            port = (size_t) num_port;
            break;
        }
    }
    signal(SIGPIPE, SIG_IGN);
    Listener_Socket sock;
    listener_init(&sock, port);
    queue = queue_new(num_threads);
    pthread_t worker_threads[num_threads];
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&worker_threads[i], NULL, worker, NULL);
    }
    while (1) {
        uintptr_t connfd = (uintptr_t) listener_accept(&sock);
        queue_push(queue, (void *) connfd);
    }
    return EXIT_SUCCESS;
}
