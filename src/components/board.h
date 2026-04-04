#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include "dsu.h"

#define MAX_HISTORY 256
#define MAX_CHANGES 2048

typedef struct {
    int bb;
    int i;
} BBIndex;

typedef enum {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
} Player;

typedef struct {
    int pos;
    Player player;
    int snapshot_black;
    int snapshot_white;
} Move;

typedef struct {
    Move moves[MAX_HISTORY];
    int size;

    DSUChange changes[MAX_CHANGES];
    int changes_size;
} GameHistory;

typedef struct {
    uint64_t black_bits[3];
    uint64_t white_bits[3];

    DSU* dsu_black;
    DSU* dsu_white;

    int n;
    int size;

    int N, S, W, E;
} BoardState;

typedef struct {
    BoardState* board;
    Player current_player;
} GameState;



BoardState* board_create(int n);
void board_draw(BoardState* b);
void board_free(BoardState* b);

GameState* game_create(int n);
void game_free(GameState* g);

int game_make_move(GameState* g, GameHistory* h, int r, int c);
void game_undo(GameState* g, GameHistory* h);

int board_check_win(BoardState* b, Player p);
Player board_at(BoardState* b, int r, int c);

#endif