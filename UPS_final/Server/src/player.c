//
// Created by dubs on 23.12.19.
//

#include <pthread.h>
#include "player.h"
#include "stdlib.h"
#include "stdio.h"
#include "connection.h"
#include "string.h"

/// Creates new player
/// \param conn         connection struct
/// \param nickname     Player nickname
/// \return             Player struct
player_t *player_create(connection_t *conn, char *nickname)
{
    player_t *p = malloc(sizeof(player_t));
    p->nickname = malloc(sizeof(char) * 50);
    sprintf(p->nickname, "%s", nickname);
    p->client_addr = conn->client_address;
    p->symbol = 'N';
    p->socket = conn->client_socket;
    p->id = generate_id();
    p->next = NULL;
    p->game = NULL;
    p->disconnected = 'n';
    p->on_turn = 'n';
    p->in_lobby = 'n';
    p->in_queue = 'n';
    p->in_game = 'n';
    p->reconnecting = 'n';

    return p;
}

/// Adds player into player list
/// \param player       Player struct
void add_player(player_t *player)
{
    if(!player)
        return;

    pthread_mutex_lock(&player_list_mutex);

    if(player_list == NULL)
    {
        player_list = player;
    }
    else
    {
        player_t *ptr = player_list;

        while (ptr->next != NULL)
        {
            ptr = ptr->next;
        }

        ptr->next = player;
    }

    pthread_mutex_unlock(&player_list_mutex);
}

/// Destroys player and removes him from player list
/// \param         Player struct
void player_remove(player_t *player)
{
    pthread_mutex_lock(&player_list_mutex);
    player_t * prev;

    free(player->nickname);
    player->game = NULL;

    if(player_list->id == player->id)
    {
        if(player->next != NULL)
        {
            player_list = player->next;
        }
        else
        {
            player_list = NULL;
        }
        free(player->id);
        player->next = NULL;
        free(player);
        player = NULL;
        pthread_mutex_unlock(&player_list_mutex);
    }
    else
    {
        prev = player_list;
        while (prev->next->id != player->id)
        {
            prev = prev->next;
        }
        prev->next = player->next;
        free(player->id);
        player->next = NULL;
        free(player);
        player = NULL;
        pthread_mutex_unlock(&player_list_mutex);
    }
}

/// Prints player list
void player_list_print()
{
    if(player_list == NULL)
    {
        printf("No players connected\n");
        return;
    }

    pthread_mutex_lock(&player_list_mutex);
    player_t *ptr = player_list;

    while (ptr != NULL)
    {
        printf("%s, %s\n", ptr->id, ptr->nickname);
        ptr = ptr->next;
    }
    pthread_mutex_unlock(&player_list_mutex);
}

/// Finds player by id
/// \param id
/// \return     NULL - player not exists, Player struct - player exists
player_t *find_player(char *id)
{
    if(player_list == NULL)
    {
        return NULL;
    }

    pthread_mutex_lock(&player_list_mutex);
    player_t *ptr = player_list;

    while (ptr != NULL)
    {
        if(strcmp(id, ptr->id) == 0)
        {
            pthread_mutex_unlock(&player_list_mutex);
            return ptr;
        }
        ptr = ptr->next;
    }

    pthread_mutex_unlock(&player_list_mutex);
    return NULL;
}

/// Finds player by id
/// \param id
/// \return     NULL - player not exists, Player struct - player exists
player_t *find_player_by_nickname(char *nickname)
{
    if(player_list == NULL)
    {
        return NULL;
    }

    pthread_mutex_lock(&player_list_mutex);
    player_t *ptr = player_list;

    while (ptr != NULL)
    {
        if(strcmp(nickname, ptr->nickname) == 0)
        {
            pthread_mutex_unlock(&player_list_mutex);
            return ptr;
        }
        ptr = ptr->next;
    }

    pthread_mutex_unlock(&player_list_mutex);
    return NULL;
}