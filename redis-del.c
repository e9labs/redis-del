#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include "hiredis/hiredis.h"

#define BUFFER_SIZE 256
#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 6379

static void cleanup(redisContext *context, redisReply *reply) {
    if (reply != NULL) {
        freeReplyObject(reply);
    }
    if (context != NULL) {
        redisFree(context);
    }
}

static int parse_port(const char *str, int *port) {
    char *endptr;
    long val;

    errno = 0;
    val = strtol(str, &endptr, 10);

    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
        || (errno != 0 && val == 0)) {
        return -1;
    }

    if (endptr == str || *endptr != '\0') {
        return -1;
    }

    if (val < 1 || val > 65535) {
        return -1;
    }

    *port = (int)val;
    return 0;
}

int main(int argc, char *argv[]) {
    redisContext *context = NULL;
    redisReply *reply = NULL;
    char buffer[BUFFER_SIZE];
    const char *key_pattern;
    const char *host = DEFAULT_HOST;
    int port = DEFAULT_PORT;
    char *password = NULL;
    int status = EXIT_FAILURE;

    if (argc < 2 || argc > 4) {
        fprintf(stderr, "Usage: redis-del \"key:pattern\" [host] [port]\n");
        return EXIT_FAILURE;
    }

    key_pattern = argv[1];

    if (argc >= 3) {
        host = argv[2];
    }

    if (argc == 4) {
        if (parse_port(argv[3], &port) != 0) {
            fprintf(stderr, "Error: Invalid port number\n");
            return EXIT_FAILURE;
        }
    }

    context = redisConnect(host, port);
    if (context == NULL || context->err) {
        fprintf(stderr, "Connection error: %s\n",
                context ? context->errstr : "Failed to allocate context");
        cleanup(context, NULL);
        return EXIT_FAILURE;
    }

    password = getpass("Password (enter for none): ");
    if (password != NULL && strlen(password) > 0) {
        reply = redisCommand(context, "AUTH %s", password);
        if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
            fprintf(stderr, "Authentication error: %s\n",
                    reply ? reply->str : "Failed to execute AUTH command");
            cleanup(context, reply);
            return EXIT_FAILURE;
        }
        freeReplyObject(reply);
        reply = NULL;
    }

    reply = redisCommand(context, "KEYS %s", key_pattern);
    if (reply == NULL) {
        fprintf(stderr, "Error: Failed to execute KEYS command\n");
        cleanup(context, NULL);
        return EXIT_FAILURE;
    }

    if (reply->type == REDIS_REPLY_ERROR) {
        fprintf(stderr, "Error: %s\n", reply->str);
        cleanup(context, reply);
        return EXIT_FAILURE;
    }

    if (reply->type != REDIS_REPLY_ARRAY) {
        fprintf(stderr, "Error: Unexpected response type: %d\n", reply->type);
        cleanup(context, reply);
        return EXIT_FAILURE;
    }

    if (reply->elements == 0) {
        printf("No keys matched!\n");
        status = EXIT_SUCCESS;
    } else {
        printf("%zu matched keys. Delete them? [y/N]: ", reply->elements);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            fprintf(stderr, "Error reading input\n");
        } else {
            buffer[strcspn(buffer, "\n")] = 0;
            if (strcmp(buffer, "y") == 0 || strcmp(buffer, "Y") == 0) {
                for (size_t i = 0; i < reply->elements; i++) {
                    redisReply *del_reply = redisCommand(context, "DEL %s",
                            reply->element[i]->str);
                    if (del_reply != NULL) {
                        printf("%zu: Deleted %s\n", i, reply->element[i]->str);
                        freeReplyObject(del_reply);
                    } else {
                        fprintf(stderr, "Failed to delete key: %s\n",
                                reply->element[i]->str);
                    }
                }
                printf("%zu objects removed\n", reply->elements);
                status = EXIT_SUCCESS;
            } else {
                printf("Operation cancelled\n");
                status = EXIT_SUCCESS;
            }
        }
    }

    cleanup(context, reply);
    return status;
}
