//
// Created by dubs on 25.12.19.
//

#include "game_logic.h"
#include "structs.h"

/// Checks, if player already won
/// \param matrix       game matrix
/// \param symbol       symbol to add into matrix
/// \param row          row position of new symbol
/// \param column       column position of new symbol
/// \return             win - game ends, no - no players won already
char *check_win(char matrix[MATRIX_SIZE][MATRIX_SIZE], char symbol, int row, int column)
{
    int symbols = 0;
    int act_row = row, act_column = column;
    char *result;

    while (act_row >= 0 && act_column >= 0 && matrix[act_row][act_column] == symbol)
    {
        symbols++;
        act_row--;
        act_column--;
    }

    act_row = row + 1;
    act_column = column + 1;

    while (act_row < MATRIX_SIZE && act_column < MATRIX_SIZE && matrix[act_row][act_column] == symbol)
    {
        symbols++;
        act_row++;
        act_column++;
    }

    act_row = row;
    act_column = column;

    if(symbols >= 5)
    {
        result = "win";
        return result;
    }

    symbols = 0;

    while (act_row >= 0 && act_column < MATRIX_SIZE && matrix[act_row][act_column] == symbol)
    {
        symbols++;
        act_row--;
        act_column++;
    }

    act_row = row + 1;
    act_column = column - 1;

    while (act_row < MATRIX_SIZE && act_column >=0 && matrix[act_row][act_column] == symbol)
    {
        symbols++;
        act_row++;
        act_column--;
    }

    act_row = row;
    act_column = column;

    if(symbols >= 5)
    {
        result = "win";
        return result;
    }

    symbols = 0;

    while (act_row >=0 && matrix[act_row][column] == symbol)
    {
        symbols++;
        act_row--;
    }

    act_row = row + 1;

    while (act_row < MATRIX_SIZE && matrix[act_row][column] == symbols)
    {
        symbols++;
        act_row++;
    }

    act_row = row;

    if(symbols >= 5)
    {
        result = "win";
        return result;
    }

    symbols = 0;

    while (act_column >= 0 && matrix[row][act_column] == symbol)
    {
        symbols++;
        act_column--;
    }

    act_column = column + 1;

    while (act_column < MATRIX_SIZE && matrix[row][act_column] == symbol)
    {
        symbols++;
        act_column++;
    }

    if(symbols >= 5)
    {
        result = "win";
        return result;
    }

    result = "no";
    return result;
}

/// Checks, if move is valid
/// \param matrix       game matrix
/// \param row          row position of move
/// \param column       column position of move
/// \return             y - move is valid, n - move is invalid
char valid_move(char matrix[MATRIX_SIZE][MATRIX_SIZE], int row, int column)
{
    if(row < 0 || row >= MATRIX_SIZE || column < 0 || column >= MATRIX_SIZE)
    {
        return 'n';
    }
    if(matrix[row][column] == 'X' || matrix[row][column] == 'O')
    {
        return 'n';
    }
    return 'y';
}
