//
// Created by dubs on 24.12.19.
//

#include <pthread.h>
#include "game.h"
#include "stdlib.h"
#include "connection.h"
#include "structs.h"
#include "stdio.h"

/// Creates new game
/// \param player       Player creating this game
/// \return             Game struct
game_t *create_game(player_t *player)
{
    game_t *g = malloc(sizeof(game_t));
    g->id = generate_id();
    pthread_mutex_lock(&player_list_mutex);
    g->players[0] = player;
    g->players[1] = NULL;
    g->next = NULL;

    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            g->matrix[i][j] = '-';
        }
    }
    pthread_mutex_unlock(&player_list_mutex);

    return g;
}

/// Adds game in game list
/// \param game     Game struct
void add_game(game_t *game)
{
    if(!game)
        return;

    pthread_mutex_lock(&game_list_mutex);

    if(game_list == NULL)
    {
        game_list = game;
    }

    else
    {
        game_t *ptr = game_list;

        while (ptr->next != NULL)
        {
            ptr = ptr->next;
        }

        ptr->next = game;
    }

    pthread_mutex_unlock(&game_list_mutex);

}

/// Find game with only 1 player
/// \return         NULL - no game with only 1 player exists, game struct - game with 1 player
game_t *find_free_seat()
{
    pthread_mutex_lock(&game_list_mutex);

    if(game_list == NULL)
    {
        pthread_mutex_unlock(&game_list_mutex);
        return NULL;
    }

    game_t *ptr = game_list;

    while (ptr != NULL)
    {
        if(ptr->players[1] == NULL)
        {
            pthread_mutex_unlock(&game_list_mutex);
            return ptr;
        }
        ptr = ptr->next;
    }

    pthread_mutex_unlock(&game_list_mutex);
    return NULL;
}

/// Destroys game and removes it from player list
/// \param game         Game struct
void game_remove(game_t *game)
{
    pthread_mutex_lock(&game_list_mutex);

    pthread_mutex_lock(&player_list_mutex);

    if(game->players[0] == NULL && game->players[1] == NULL)
    {
        pthread_mutex_unlock(&player_list_mutex);
        pthread_mutex_unlock(&game_list_mutex);
        return;
    }

    if(game->players[0] != NULL)
        game->players[0]->game = NULL;
    if(game->players[1] != NULL)
        game->players[1]->game = NULL;

    pthread_mutex_unlock(&player_list_mutex);

    game->players[0] = NULL;
    game->players[1] = NULL;
    game_t *prev;

    if(game_list->id == game->id)
    {
        if(game->next != NULL)
        {
            game_list = game->next;
        }
        else
        {
            game_list = NULL;
        }
        free(game->id);
        game->next = NULL;
        free(game);
        game = NULL;
        pthread_mutex_unlock(&game_list_mutex);
    }
    else
    {
        prev = game_list;
        while (prev->next->id != game->id)
        {
            prev = prev->next;
        }
        prev->next = game->next;
        free(game->id);
        game->next = NULL;
        free(game);
        game = NULL;
        pthread_mutex_unlock(&game_list_mutex);
    }
}

/// Prints game list
void game_list_print()
{
    if (game_list == NULL)
    {
        printf("No games created\n");
        return;
    }

    pthread_mutex_lock(&game_list_mutex);
    game_t *ptr = game_list;
    char *second;

    while (ptr != NULL)
    {
        if(ptr->players[1] == NULL)
        {
            second = "";
        }
        else
        {
            second = ptr->players[1]->nickname;
        }
        printf("%s, %s, %s\n", ptr->id, ptr->players[0]->nickname, second);
        ptr = ptr->next;
    }
    pthread_mutex_unlock(&game_list_mutex);
}