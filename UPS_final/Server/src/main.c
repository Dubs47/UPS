#include <stdio.h>
#include <pthread.h>
#include "connection.h"
#include "string.h"
#include "player.h"
#include "game.h"
#include "stdlib.h"
#include "ctype.h"

pthread_t thread_id;

/// Main method of application. Creates server with input address and port. If no address or port is entered, used address 127.0.0.1 and port 10000.
int main(int argc, char *argv[]) {
    char input[1024];

    thread_id = 0;

    // no address and port input
    if(argc <= 1)
    {
        char *args = NULL;
        pthread_create(&thread_id, NULL, connection_serve, (void *) args);
    }
    else
    {
        char *address, *port;
        address = malloc(strlen(argv[1]) * sizeof(char));
        sprintf(address, "%s", argv[1]);
        int x;
        char *token;

        //checking correct format of address
        for(int a = 0; a < 4; a++) {
            if(a == 0)
            {
                token = strtok(argv[1], ".");
            }
            else
            {
                token = strtok(NULL, ".");
            }

            if (token == NULL) {
                printf("Bad format of address argument");
                exit(1);
            }
            for (int i = 0; i < strlen(token); i++) {
                if (token[i] != '0' && isdigit(token[i]) == 0) {
                    if (token[i] != '\n' && token[i] != '\0') {
                        printf("Bad format of address argument");
                        exit(1);
                    }
                }
            }
            x = atoi(token);
            if (x < 0 || x > 255) {
                printf("Bad format of address argument");
                exit(1);
            }
        }
        token = strtok(NULL, ".");
        if (token != NULL && token[0] != '\n' && token[0] != '\0')
        {
            printf("Bad format of address argument");
            exit(1);
        }

        //no port input
        if(argc <= 2)
        {
            pthread_create(&thread_id, NULL, connection_serve, (void *) address);
        }
        else
        {
            //checking correct format of port
            port = malloc(strlen(argv[2]) * sizeof(char));
            sprintf(port, "%s", argv[2]);
            if(strlen(argv[2]) > 5)
            {
                printf("Bad format of port argument");
                exit(1);
            }
            for (int i = 0; i < strlen(port); i++) {
                if (port[i] != '0' && isdigit(port[i]) == 0) {
                    if (port[i] != '\n' && port[i] != '\0') {
                        printf("Bad format of address argument");
                        exit(1);
                    }
                }
            }
            x = atoi(argv[2]);
            if (x < 0 || x > 65635)
            {
                printf("Bad format of address argument");
                exit(1);
            }
            char *args = malloc((strlen(address) + strlen(port) + 1) * sizeof(char));
            sprintf(args, "%s;%s", address, port);
            pthread_create(&thread_id, NULL, connection_serve, (void *) args);
        }
    }

    while(scanf("%s", input) != -1)
    {
        //exit server
        if(strcmp(input, ":quit") == 0)
        {
            char buffer[1024];
            sprintf(buffer, "%s", "server exit\n");
            pthread_mutex_lock(&player_list_mutex);

            player_t *ptr = player_list;

            while (ptr != NULL)
            {
                send(ptr->socket, buffer, strlen(buffer) * sizeof(char), 0);
                ptr = ptr->next;
            }

            pthread_mutex_unlock(&player_list_mutex);
            break;
        }
        //print player list
        if(strcmp(input, ":players") == 0)
        {
            player_list_print();
        }
        //print game list
        if(strcmp(input, ":games") == 0)
        {
            game_list_print();
        }
    }

    pthread_detach(thread_id);
    return 0;
}