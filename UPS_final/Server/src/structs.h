//
// Created by dubs on 23.12.19.
//

#ifndef UPS_SERVER_STRUCTS_H
#define UPS_SERVER_STRUCTS_H

#include "sys/socket.h"
#include "netinet/in.h"

#define MATRIX_SIZE 25

typedef struct player {
    char disconnected;
    char reconnecting;
    int socket;
    char in_lobby;
    char in_queue;
    char in_game;
    char on_turn;
    char *id;
    char *nickname;
    struct sockaddr_in client_addr;
    char symbol;
    struct game *game;
    struct player *next;
} player_t;

typedef struct game {
    char *id;
    player_t *players[2];
    char matrix[MATRIX_SIZE][MATRIX_SIZE];
    struct game *next;
} game_t;

typedef struct connection{
    struct sockaddr_in client_address;
    socklen_t client_address_len;
    int client_socket;
} connection_t;

#endif //UPS_SERVER_STRUCTS_H
