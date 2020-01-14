//
// Created by dubs on 25.12.19.
//

#ifndef UPS_SERVER_GAME_LOGIC_H
#define UPS_SERVER_GAME_LOGIC_H

#include "structs.h"

char *check_win(char matrix[MATRIX_SIZE][MATRIX_SIZE], char symbol, int row, int column);
char valid_move(char matrix[MATRIX_SIZE][MATRIX_SIZE], int row, int column);

#endif //UPS_SERVER_GAME_LOGIC_H
