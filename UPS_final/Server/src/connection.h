//
// Created by dubs on 23.12.19.
//

#ifndef UPS_SERVER_CONNECTION_H
#define UPS_SERVER_CONNECTION_H

#include "structs.h"

extern player_t *player_list;
extern pthread_mutex_t player_list_mutex;
extern game_t *game_list;
extern pthread_mutex_t game_list_mutex;

void *connection_serve();
void *connection_handle(void *arg);
char *generate_id();
int find_id(char *id);
void game_join_handle(player_t *player);
void game_loop(player_t *player);

#endif //UPS_SERVER_CONNECTION_H
