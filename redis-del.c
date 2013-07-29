#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hiredis/hiredis.h"

int main( int argc, char *argv[] ) {
    redisReply *reply;
    long int i;
    char buffer[256];
    char* keyString;
    char* host;
    int port;

    if (argc < 2) {
        printf("Usage: redis-del \"some:key:*\" (host) (port)\n");
        exit(1);
    } else {
        if (argc == 2) {
            keyString = argv[1];
            host = "127.0.0.1";
            port = 6379;
        } else if (argc == 3) {
            keyString = argv[1];
            host = argv[2];
            port = 6379;

        } else if (argc == 4) {
            keyString = argv[1];
            host = argv[2];
            port = atoi(argv[3]);
        }

        redisContext *c = redisConnect(host, port);
        if (c != NULL && c->err) {
            printf("Error: %s\n", c->errstr);
            exit(1);
        }

        reply = redisCommand(c,  "KEYS %s", keyString);
        if ( reply->type == REDIS_REPLY_ERROR )
            printf( "Error: %s\n", reply->str );
        else if ( reply->type != REDIS_REPLY_ARRAY )
            printf( "Unexpected type: %d\n", reply->type );
        else if (reply->elements > 0) {
            printf("%zu matched keys. Delete them? [y/N]: ", reply->elements);
            scanf ("%s", buffer);
            printf("\n");
            if (strcmp(buffer, "y") == 0 || strcmp(buffer, "Y") == 0) {
                for ( i=0; i<reply->elements; ++i ){
                    redisCommand(c, "DEL %s", reply->element[i]->str);
                    printf( "%lu: %s\n", i, reply->element[i]->str );
                }
                printf("%li objects removed\n", i);
            } else {
                printf("Cancelled!\n");
            }
        } else
            printf("No keys matched!\n");
        freeReplyObject(reply);
        exit(0);
    }
}

