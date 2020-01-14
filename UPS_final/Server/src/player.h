//
// Created by dubs on 23.12.19.
//

#ifndef UPS_SERVER_PLAYER_H
#define UPS_SERVER_PLAYER_H

#include "structs.h"

player_t *player_create(connection_t *conn, char *nickname);
void add_player(player_t *player);
void player_list_print();
void player_remove(player_t *player);
player_t *find_player(char *id);
player_t *find_player_by_nickname(char *nickname);

#endif //UPS_SERVER_PLAYER_H
