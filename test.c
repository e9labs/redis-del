#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "hiredis/hiredis.h"

#define TEST_KEY_PREFIX "redis-del-test"
#define TEST_VALUE "test-data"
#define COMMAND_BUFFER_SIZE 256

static char* generate_test_key(void) {
    static char key[64];
    time_t now = time(NULL);
    unsigned int rand_val;

    srand((unsigned int)now);
    rand_val = (unsigned int)rand();

    snprintf(key, sizeof(key), "%s:%lu:%u", TEST_KEY_PREFIX,
             (unsigned long)now, rand_val);
    return key;
}

int main(void) {
    redisContext *context;
    redisReply *reply;
    const char *test_key;
    char command[COMMAND_BUFFER_SIZE];
    int status = EXIT_FAILURE;

    // Connect to Redis
    context = redisConnect("127.0.0.1", 6379);
    if (context == NULL || context->err) {
        fprintf(stderr, "Connection error: %s\n",
                context ? context->errstr : "Failed to allocate context");
        if (context) {
            redisFree(context);
        }
        return EXIT_FAILURE;
    }

    // Generate unique test key
    test_key = generate_test_key();
    printf("Using test key: %s\n", test_key);

    // Set test key
    reply = redisCommand(context, "SET %s %s", test_key, TEST_VALUE);
    if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
        fprintf(stderr, "Failed to set test key\n");
        goto cleanup;
    }
    freeReplyObject(reply);

    // Verify key exists
    reply = redisCommand(context, "EXISTS %s", test_key);
    if (reply == NULL || reply->type != REDIS_REPLY_INTEGER || reply->integer != 1) {
        fprintf(stderr, "Test key was not set properly\n");
        goto cleanup;
    }
    freeReplyObject(reply);
    printf("Test key created successfully\n");

    // Run redis-del command
    snprintf(command, sizeof(command), "echo y | ./redis-del '%s'", test_key);
    printf("Executing: %s\n", command);
    if (system(command) != 0) {
        fprintf(stderr, "Failed to execute redis-del\n");
        goto cleanup;
    }

    // Verify key was deleted
    reply = redisCommand(context, "EXISTS %s", test_key);
    if (reply == NULL || reply->type != REDIS_REPLY_INTEGER) {
        fprintf(stderr, "Failed to check if key was deleted\n");
        goto cleanup;
    }

    if (reply->integer == 0) {
        printf("Test passed: Key was successfully deleted\n");
        status = EXIT_SUCCESS;
    } else {
        fprintf(stderr, "Test failed: Key still exists\n");
    }

cleanup:
    if (reply) {
        freeReplyObject(reply);
    }
    redisFree(context);
    return status;
}
