//
// Created by dubs on 24.12.19.
//

#ifndef UPS_SERVER_GAME_H
#define UPS_SERVER_GAME_H

#include "structs.h"

game_t *create_game(player_t *player);
void add_game(game_t *game);
game_t *find_free_seat();
void game_remove(game_t * game);
void game_list_print();

#endif //UPS_SERVER_GAME_H
