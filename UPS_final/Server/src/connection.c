//
// Created by dubs on 23.12.19.
//

#include <sys/socket.h>
#include "sys/types.h"
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "connection.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "arpa/inet.h"
#include "structs.h"
#include "player.h"
#include "game.h"
#include "game_logic.h"
#include "ctype.h"
#include "sys/time.h"

char* address;
int port;

player_t *player_list;
pthread_mutex_t player_list_mutex;
game_t *game_list;
pthread_mutex_t game_list_mutex;
pthread_t thread_id;
struct timeval timeout;

/// Generates unique id for players and games
char *generate_id()
{
    char *id = malloc(sizeof(char) * 20);

    do{
        sprintf(id, "%d", rand());
    }
    while (find_id(id) != 0);

    return id;
}

/// Tries to find existing id passed by argument
/// \param id       wanted id
/// \return         0 - if id not exists, 1 - if id exists
int find_id(char *id)
{
    if(!id)
        return -1;

    pthread_mutex_lock(&player_list_mutex);

    if(player_list != NULL)
    {
        player_t *player_ptr = player_list;

        while(player_ptr != NULL)
        {
            if(strcmp(player_ptr->id, id) == 0)
            {
                pthread_mutex_unlock(&player_list_mutex);
                return 1;
            }
            player_ptr = player_ptr->next;
        }
    }

    pthread_mutex_unlock(&player_list_mutex);

    pthread_mutex_lock(&game_list_mutex);

    if(game_list != NULL)
    {
        game_t *game_ptr = game_list;

        while(game_ptr != NULL)
        {
            if(strcmp(game_ptr->id, id) == 0)
            {
                pthread_mutex_unlock(&game_list_mutex);
                return 1;
            }
            game_ptr = game_ptr->next;
        }
    }

    pthread_mutex_unlock(&game_list_mutex);

    return 0;
}

/// Creates new socket, bind to port, listening to client.
/// \param arg      address and port
void *connection_serve(void *arg) {
    int sockfd, ret;
    struct sockaddr_in serverAddr;

    int newSocket;
    struct sockaddr_in newAddr;

    socklen_t addr_size;
    connection_t *arguments;

    bzero((int *)&addr_size, sizeof(addr_size));
    bzero((char *)&newAddr, sizeof(newAddr));

    // set timeout
    timeout.tv_sec = 60;
    timeout.tv_usec = 0;
    int flags = 1;

    // create new server socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Server Socket is created.\n");

    // set socket parameters
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&flags, sizeof(int));

    char *a = (char *) arg;

    // no address and port passed
    if(a == NULL)
    {
        address = "127.0.0.1";
        port = 10000;
    }
    else
    {
        char *t = strtok(a, ";");
        address = malloc(strlen(t) * sizeof(char));
        sprintf(address, "%s", t);
        t = strtok(NULL, ";");

        // only address passed
        if(t == NULL)
        {
            port = 10000;
        }
        // both address and port passed
        else
        {
            port = atoi(t);
        }
    }

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(address);

    // Bind socket to address
    ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if(ret < 0){
        printf("[-]Error in binding.\n");
        exit(1);
    }
    printf("[+]Bind to port %d\n", port);

    // Listen on socket
    if(listen(sockfd, 10) == 0){
        printf("[+]Listening on address %s:%d\n", address, port);
    }else{
        printf("[-]Error in binding.\n");
    }


    while(1){
        // Client socket
        newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
        if(newSocket < 0){
            exit(1);
        }

        arguments = malloc(sizeof(connection_t));
        arguments->client_address = newAddr;
        arguments->client_address_len = addr_size;
        arguments->client_socket = newSocket;

        thread_id = 0;
        if(pthread_create(&thread_id, NULL, connection_handle, (void *)arguments) == 0){

        }
    }
}

/// Takes data from client to create new player and creates new player or restore existing player after network crash.
/// \param arg      Client socket
void *connection_handle(void *arg)
{
    char *token, *nickname;
    char buffer[1024];
    connection_t *arguments = (connection_t*) arg;
    setsockopt(arguments->client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

    sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);

    int r;
    player_t *player = NULL;

    do {
        if(player != NULL && player->disconnected == 'y')
        {
            break;
        }
        r = recv(arguments->client_socket, &buffer, sizeof(buffer), 0);
        if (strcmp(buffer, "ping\0") == 0 || strcmp(buffer, "ping\n") == 0) {
            pthread_mutex_lock(&player_list_mutex);
            pthread_mutex_lock(&game_list_mutex);
            if(player != NULL && player->in_queue == 'y' && player->game->players[1] != NULL)
            {
                for(int i = 0; i < 1024; i++)
                {
                    buffer[i] = '\0';
                }
                sprintf(buffer, "%s;%c;%s\n", "Player found!", 'X', player->game->players[1]->nickname);
                pthread_mutex_unlock(&game_list_mutex);
                send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
                player->on_turn = 'y';
                sprintf(buffer, "%s", "Your turn\n\0");
                send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
                player->symbol = 'X';
                pthread_mutex_unlock(&player_list_mutex);

                // Entering game
                player->in_queue = 'n';
                player->in_game = 'y';
                game_loop(player);
                continue;
            } else
            {
                pthread_mutex_unlock(&game_list_mutex);
                pthread_mutex_unlock(&player_list_mutex);
            }

            sprintf(buffer, "%s", "ping\n");
            send(arguments->client_socket, buffer, strlen(buffer) * sizeof(char), 0);
            continue;
        } else if (strcmp(buffer, ":exit") == 0) {
            close(arguments->client_socket);
            break;
        }

        if (buffer[r - 1] == '\n')
            buffer[r - 1] = '\0';

        if(player != NULL)
        {
            if (strcmp(buffer, ":play") == 0 && player->in_queue == 'n' && player->in_game == 'n')
            {
                game_join_handle(player);
                continue;
            }
        }

        token = strtok(buffer, ";");

        if (strcmp(token, "reconnect") == 0) {
            token = strtok(NULL, ";");
            if (token != NULL) {
                player = find_player(token);

                // If players still exists
                if (player != NULL) {
                    player->reconnecting = 'y';
                    pthread_mutex_lock(&player_list_mutex);
                    player->socket = arguments->client_socket;
                    sprintf(buffer, "%s", "reconnected\n");
                    send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
                    pthread_mutex_unlock(&player_list_mutex);

                    // If was in game, send actual matrix and who is on turn
                    if (player->in_game == 'y') {
                        for (int i = 0; i < MATRIX_SIZE; i++) {
                            for (int j = 0; j < MATRIX_SIZE; ++j) {
                                if (player->game->matrix[i][j] == 'X' || player->game->matrix[i][j] == 'O') {
                                    sprintf(buffer, "%c;%d;%d\n", player->game->matrix[i][j], i, j);
                                    send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
                                }
                            }
                        }
                        if (player->on_turn == 'y') {
                            sprintf(buffer, "%s", "Your turn\n\0");
                            send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
                        } else {
                            sprintf(buffer, "%s", "Enemy turn\n\0");
                            send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
                        }
                        game_loop(player);
                        continue;
                    }

                        // If was in game queue
                    else if (player->in_queue == 'y') {
                        sprintf(buffer, "%s", "Exit or play?\n");
                        send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
                        sprintf(buffer, "%s", "Waiting for second player\n");
                        send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
                    }

                        // If was in lobby
                    else {
                        sprintf(buffer, "%s", "Exit or play?\n");
                        send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
                    }
                }

                    // Player was disconnected too long
                else {
                    sprintf(buffer, "%s", "session expired\n");
                    send(arguments->client_socket, buffer, strlen(buffer) * sizeof(char), 0);

                    if ((token = strtok(NULL, ";")) != NULL) {
                        nickname = token;
                    } else {
                        nickname = "player";
                    }

                    player = player_create(arguments, nickname);
                    add_player(player);

                    sprintf(buffer, "%s;%s;%s\n", "login", player->id, player->nickname);
                    send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);

                    sprintf(buffer, "%s", "Exit or play?\n");
                    send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
                }
            }
        }
        else if(strcmp(token, "nickname") == 0)
        {
            if ((token = strtok(NULL, ";")) != NULL) {
                nickname = token;
            } else {
                nickname = "player";
            }

            player = find_player_by_nickname(nickname);

            if(player == NULL)
            {
                player = player_create(arguments, nickname);
                add_player(player);

                sprintf(buffer, "%s;%s;%s\n", "login", player->id, player->nickname);
                send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
            } else
            {
                player->reconnecting = 'y';
                pthread_mutex_lock(&player_list_mutex);
                player->socket = arguments->client_socket;
                sprintf(buffer, "%s", "reconnected\n");
                send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
                pthread_mutex_unlock(&player_list_mutex);

                sprintf(buffer, "%s;%s;%s\n", "login", player->id, player->nickname);
                send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);

                // If was in game, send actual matrix and who is on turn
                if (player->in_game == 'y') {
                    for (int i = 0; i < MATRIX_SIZE; i++) {
                        for (int j = 0; j < MATRIX_SIZE; ++j) {
                            if (player->game->matrix[i][j] == 'X' || player->game->matrix[i][j] == 'O') {
                                sprintf(buffer, "%c;%d;%d\n", player->game->matrix[i][j], i, j);
                                send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
                            }
                        }
                    }
                    if (player->on_turn == 'y') {
                        sprintf(buffer, "%s", "Your turn\n\0");
                        send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
                    } else {
                        sprintf(buffer, "%s", "Enemy turn\n\0");
                        send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
                    }
                    game_loop(player);
                    continue;
                }

                    // If was in game queue
                else if (player->in_queue == 'y') {
                    sprintf(buffer, "%s", "Exit or play?\n");
                    send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
                    sprintf(buffer, "%s", "Waiting for second player\n");
                    send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
                }

                    // If was in lobby
                else {
                    sprintf(buffer, "%s", "Exit or play?\n");
                    send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
                }
            }

            sprintf(buffer, "%s", "Exit or play?\n");
            send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
        }
        else
        {
            sprintf(buffer, "%s", "ERROR!!! Bad message, disconnecting...\n");
            send(arguments->client_socket, buffer, strlen(buffer) * sizeof(char), 0);
            close(arguments->client_socket);
            break;
        }
    }
    while (r > 0);

    if(player != NULL)
    {
        if(player->reconnecting == 'y')
        {
            player->reconnecting = 'n';
        }
        else
        {
            pthread_mutex_lock(&player_list_mutex);
            pthread_mutex_lock(&game_list_mutex);
            if(player->game != NULL && player->game->players[1] == NULL)
            {
                pthread_mutex_unlock(&game_list_mutex);
                pthread_mutex_unlock(&player_list_mutex);
                game_remove(player->game);
            } else
            {
                pthread_mutex_unlock(&game_list_mutex);
                pthread_mutex_unlock(&player_list_mutex);
            }

            player_remove(player);
        }
    }


    free(arguments);
    pthread_detach(pthread_self());
}

/// Handling joining into game
/// \param player       Player
void game_join_handle(player_t *player)
{
    char buffer[1024];
    for(int i = 0; i < 1024; i++)
    {
        buffer[i] = '\0';
    }

    player->in_queue = 'y';

    game_t *g = find_free_seat();

    // If does no exists game with only 1 player
    if(g == NULL)
    {
        g = create_game(player);
        add_game(g);
        pthread_mutex_lock(&player_list_mutex);
        player->game = g;
        sprintf(buffer, "%s", "Waiting for second player\n\0");
        send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
        pthread_mutex_unlock(&player_list_mutex);
    }

    // Game with only 1 player exists
    else
    {
        pthread_mutex_lock(&game_list_mutex);
        g->players[1] = player;
        pthread_mutex_unlock(&game_list_mutex);
        pthread_mutex_lock(&player_list_mutex);
        player->game = g;
        player->symbol = 'O';
        player->on_turn = 'n';
        pthread_mutex_lock(&game_list_mutex);
        sprintf(buffer, "%s;%c;%s\n", "Player found!", 'O', g->players[0]->nickname);
        pthread_mutex_unlock(&game_list_mutex);
        send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
        sprintf(buffer, "%s", "Enemy turn\n\0");
        send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
        pthread_mutex_unlock(&player_list_mutex);

        // Entering game
        player->in_queue = 'n';
        player->in_game = 'y';
        game_loop(player);
    }
}

/// Represents game loop of game
/// \param player       Player
void game_loop(player_t *player)
{
    char bad_message = 'n';
    char buffer[1024];
    player_t *enemy;
    int enemySocket, enemy_index;
    int x, y;
    char *token, *result;

    pthread_mutex_lock(&game_list_mutex);
    game_t *game = player->game;

    // Obtaining enemy player data
    if(game->players[0]->socket == player->socket)
    {
        enemy = game->players[1];
        enemy_index = 1;
        enemySocket = game->players[enemy_index]->socket;
    }
    else
    {
        enemy = game->players[0];
        enemy_index = 0;
        enemySocket = game->players[enemy_index]->socket;
    }
    pthread_mutex_unlock(&game_list_mutex);

    while(1)
    {
        int r = recv(player->socket, &buffer, sizeof(buffer), 0);
        pthread_mutex_lock(&game_list_mutex);
        pthread_mutex_lock(&player_list_mutex);

        // Update enemy socket
        if(game != NULL && game->players[enemy_index] != NULL)
        {
            enemySocket = game->players[enemy_index]->socket;
        }
        pthread_mutex_unlock(&player_list_mutex);
        pthread_mutex_unlock(&game_list_mutex);
        if(r <= 0)
        {
            pthread_mutex_lock(&player_list_mutex);
            if(player->reconnecting == 'y')
            {
                pthread_mutex_unlock(&player_list_mutex);
                break;
            }
            pthread_mutex_unlock(&player_list_mutex);
            pthread_mutex_lock(&game_list_mutex);
            if(game->players[0] != NULL && game->players[1] != NULL)
            {
                pthread_mutex_unlock(&game_list_mutex);
                sprintf(buffer, "%s", "disconnected enemy\n");
                pthread_mutex_lock(&player_list_mutex);
                enemy->on_turn = 'y';
                player->disconnected = 'y';
                player->in_game = 'n';
                pthread_mutex_unlock(&player_list_mutex);
                send(enemySocket, buffer, strlen(buffer) * sizeof(char), 0);
                game_remove(game);
                break;
            }
            else {
                pthread_mutex_unlock(&game_list_mutex);
                pthread_mutex_lock(&player_list_mutex);
                player->disconnected = 'y';
                player->in_game = 'n';
                pthread_mutex_unlock(&player_list_mutex);
                break;
            }
        }
        if(strcmp(buffer, "ping") == 0 || strcmp(buffer, "ping\n") == 0)
        {
            sprintf(buffer, "%s", "ping\n");
            send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
            continue;
        }
        if(buffer[r - 1] == '\n')
            buffer[r - 1] = '\0';
        pthread_mutex_lock(&player_list_mutex);
        if(player->on_turn == 'n')
        {
            sprintf(buffer, "%s", "disconnected enemy\n");
            send(enemySocket, buffer, strlen(buffer) * sizeof(char), 0);
            sprintf(buffer, "%s", "ERROR!!! Bad message, disconnecting...\n");
            send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
            player->disconnected = 'y';
            player->in_game = 'n';
            pthread_mutex_unlock(&player_list_mutex);
            game_remove(game);
            break;
        }
        pthread_mutex_unlock(&player_list_mutex);

        // Player did exit
        if(strcmp(buffer, ":exit") == 0)
        {
            pthread_mutex_lock(&game_list_mutex);
            if(game != NULL && (game->players[0] != NULL && game->players[1] != NULL))
            {
                pthread_mutex_unlock(&game_list_mutex);
                sprintf(buffer, "%s", "disconnected enemy\n");
                pthread_mutex_lock(&player_list_mutex);
                enemy->on_turn = 'y';
                pthread_mutex_unlock(&player_list_mutex);
                send(enemySocket, buffer, strlen(buffer) * sizeof(char), 0);
                game_remove(game);
            }
            pthread_mutex_lock(&player_list_mutex);
            player->disconnected = 'y';
            player->in_game = 'n';
            pthread_mutex_unlock(&player_list_mutex);
            break;
        }

        // Entering lobby after game was ended
        if(strcmp(buffer, "Enter lobby") == 0 && player->game == NULL)
        {
            sprintf(buffer, "%s", "Exit or play?\n");
            send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
            player->in_game = 'n';
            break;
        }
        pthread_mutex_lock(&player_list_mutex);
        if(player->game == NULL)
        {
            sprintf(buffer, "%s", "ERROR!!! Bad message, disconnecting...\n");
            send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
            player->disconnected = 'y';
            player->in_game = 'n';
            pthread_mutex_unlock(&player_list_mutex);
            break;
        }
        pthread_mutex_unlock(&player_list_mutex);
        token = strtok(buffer, ";");
        if (token == NULL)
        {
            pthread_mutex_lock(&player_list_mutex);
            sprintf(buffer, "%s", "disconnected enemy\n");
            enemy->on_turn = 'y';
            send(enemySocket, buffer, strlen(buffer) * sizeof(char), 0);
            sprintf(buffer, "%s", "ERROR!!! Bad message, disconnecting...\n");
            send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
            player->disconnected = 'y';
            player->in_game = 'n';
            pthread_mutex_unlock(&player_list_mutex);
            game_remove(game);
            break;
        }

        // Checking correct message from player
        for (int i = 0; i < strlen(token); i++)
        {
            if(token[i] != '0' && isdigit(token[i]) == 0)
            {
                bad_message = 'y';
                break;
            }
        }
        if(bad_message == 'y')
        {
            pthread_mutex_lock(&player_list_mutex);
            sprintf(buffer, "%s", "disconnected enemy\n");
            enemy->on_turn = 'y';
            send(enemySocket, buffer, strlen(buffer) * sizeof(char), 0);
            sprintf(buffer, "%s", "ERROR!!! Bad message, disconnecting...\n");
            send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
            player->disconnected = 'y';
            player->in_game = 'n';
            pthread_mutex_unlock(&player_list_mutex);
            game_remove(game);
            break;
        }
        x = atoi(token);
        token = strtok(NULL, ";");

        if (token == NULL)
        {
            pthread_mutex_lock(&player_list_mutex);
            sprintf(buffer, "%s", "disconnected enemy\n");
            enemy->on_turn = 'y';
            send(enemySocket, buffer, strlen(buffer) * sizeof(char), 0);
            sprintf(buffer, "%s", "ERROR!!! Bad message, disconnecting...\n");
            send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
            player->disconnected = 'y';
            player->in_game = 'n';
            pthread_mutex_unlock(&player_list_mutex);
            game_remove(game);
            break;
        }

        // Checking correct message from player
        for (int i = 0; i < strlen(token); i++)
        {
            if(token[i] != '0' && isdigit(token[i]) == 0)
            {
                if(token[i] != '\n' && token[i] != '\0')
                {
                    bad_message = 'y';
                    break;
                }
            }
        }
        if(bad_message == 'y')
        {
            pthread_mutex_lock(&player_list_mutex);
            sprintf(buffer, "%s", "disconnected enemy\n");
            enemy->on_turn = 'y';
            send(enemySocket, buffer, strlen(buffer) * sizeof(char), 0);
            sprintf(buffer, "%s", "ERROR!!! Bad message, disconnecting...\n");
            send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
            player->disconnected = 'y';
            player->in_game = 'n';
            pthread_mutex_unlock(&player_list_mutex);
            game_remove(game);
            break;
        }

        y = atoi(token);
        pthread_mutex_lock(&game_list_mutex);

        // Invalid move
        if(valid_move(game->matrix, x, y) == 'n')
        {
            pthread_mutex_unlock(&game_list_mutex);
            pthread_mutex_lock(&player_list_mutex);
            sprintf(buffer, "%s", "disconnected enemy\n");
            enemy->on_turn = 'y';
            send(enemySocket, buffer, strlen(buffer) * sizeof(char), 0);
            sprintf(buffer, "%s", "ERROR!!! Bad message, disconnecting...\n");
            send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
            player->disconnected = 'y';
            player->in_game = 'n';
            pthread_mutex_unlock(&player_list_mutex);
            game_remove(game);
            break;
        }
        pthread_mutex_lock(&player_list_mutex);
        game->matrix[x][y] = player->symbol;
        result = check_win(game->matrix, player->symbol, x, y);
        pthread_mutex_unlock(&player_list_mutex);
        pthread_mutex_unlock(&game_list_mutex);
        sprintf(buffer, "%c;%d;%d\n", player->symbol, x, y);
        send(enemySocket, buffer, strlen(buffer) * sizeof(char), 0);
        send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);

        // Player won
        if(strcmp(result, "win") == 0)
        {
            pthread_mutex_lock(&player_list_mutex);
            sprintf(buffer, "%s", "win\n");
            send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
            player->game = NULL;

            // Send enemy that he lost
            sprintf(buffer, "%s", "lose\n");
            enemy->on_turn = 'y';
            enemy->game = NULL;
            send(enemySocket, buffer, strlen(buffer) * sizeof(char), 0);
            pthread_mutex_unlock(&player_list_mutex);
            game_remove(game);
        }

        // No player has won yet
        else
        {
            pthread_mutex_lock(&player_list_mutex);
            enemy->on_turn = 'y';
            sprintf(buffer, "%s", "Your turn\n\0");
            send(enemySocket, buffer, strlen(buffer) * sizeof(char), 0);
            player->on_turn = 'n';
            sprintf(buffer, "%s", "Enemy turn\n\0");
            pthread_mutex_unlock(&player_list_mutex);
            send(player->socket, buffer, strlen(buffer) * sizeof(char), 0);
        }
    }
}