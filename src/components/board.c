#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// HEX neighbors 
static const int dr[6] = {-1, -1, 0, 0, 1, 1};
static const int dc[6] = {0, 1, -1, 1, -1, 0};

// UTILS
static BBIndex index_of(BoardState* b, int r, int c) {
    int global = r * b->n + c;
    BBIndex res = { global / 64, global % 64 };
    return res;
}

static int in_bounds(BoardState* b, int r, int c) {
    return r >= 0 && r < b->n && c >= 0 && c < b->n;
}

Player board_at(BoardState* b, int r, int c) {
    BBIndex idx = index_of(b, r, c);

    if ((b->black_bits[idx.bb] >> idx.i) & 1ULL) return BLACK;
    if ((b->white_bits[idx.bb] >> idx.i) & 1ULL) return WHITE;
    return EMPTY;
}

static char stone_char(Player p) {
    if (p == BLACK) return 'O';
    if (p == WHITE) return '@';
    return '.';
}

// NEIGHBORS
static void connect_neighbors(BoardState* b, GameHistory* h,
                              int r, int c, Player p) 
{
    int flat = r * b->n + c;

    for (int k = 0; k < 6; k++)
    {
        int nr = r + dr[k];
        int nc = c + dc[k];

        if (!in_bounds(b, nr, nc)) continue;
        if (board_at(b, nr, nc) != p) continue;

        int nflat = nr * b->n + nc;

        if (p == BLACK)
        {
            dsu_union(b->dsu_black, flat, nflat,
                      h->changes, &h->changes_size);
        }
        else
        {
            dsu_union(b->dsu_white, flat, nflat,
                      h->changes, &h->changes_size);
        }
    }
}

// BOARD
BoardState* board_create(int n) 
{
    BoardState* b = malloc(sizeof(BoardState));

    b->n = n;
    b->size = n * n;

    for (int i = 0; i < 3; i++) 
    {
        b->black_bits[i] = 0ULL;
        b->white_bits[i] = 0ULL;
    }

    b->dsu_black = dsu_create(b->size + 4);
    b->dsu_white = dsu_create(b->size + 4);

    b->N = b->size;
    b->S = b->size + 1;
    b->W = b->size + 2;
    b->E = b->size + 3;

    return b;
}

void board_draw(BoardState* b) 
{
    int n = b->n;

    printf("  ");
    for (char litera = 'A'; litera < 'A' + n; litera++)
        printf("%2c ", litera);
    printf("\n");

    for (int r = 0; r < n; r++) 
    {
        for (int i = 0; i < r; i++)
        {
            printf("  ");
        }
        printf("%2d ", r);

        for (int c = 0; c < n; c++) 
        {
            Player p = board_at(b, r, c);

            char ch = stone_char(p);

            printf(" %c ", ch);
        }
        printf("\n");
    }
}

void board_free(BoardState* b) 
{
    dsu_free(b->dsu_black);
    dsu_free(b->dsu_white);
    free(b);
}

// GAME
GameState* game_create(int n) 
{
    GameState* g = malloc(sizeof(GameState));
    g->board = board_create(n);
    g->current_player = BLACK;
    return g;
}

void game_free(GameState* g)
{
    board_free(g->board);
    free(g);
}

// MOVES
int game_make_move(GameState* g, GameHistory* h, int r, int c) 
{
    BoardState* b = g->board;
    Player p = g->current_player;

    if (!in_bounds(b, r, c)) return 0;
    if (board_at(b, r, c) != EMPTY) return 0;

    assert(h->size < MAX_HISTORY);

    BBIndex idx = index_of(b, r, c);
    int flat = r * b->n + c;

    Move* mv = &h->moves[h->size++];

    mv->pos = flat;
    mv->player = p;
    mv->snapshot_black = dsu_snapshot(h->changes_size);
    mv->snapshot_white = dsu_snapshot(h->changes_size);

    if (p == BLACK) 
    {
        b->black_bits[idx.bb] |= (1ULL << idx.i);

        if (r == 0)
        {
            dsu_union(b->dsu_black, flat, b->N,
                      h->changes, &h->changes_size);
        }
        if (r == b->n - 1)
        {
            dsu_union(b->dsu_black, flat, b->S,
                      h->changes, &h->changes_size);
        }
        connect_neighbors(b, h, r, c, BLACK);

    } 
    else 
    {
        b->white_bits[idx.bb] |= (1ULL << idx.i);

        if (c == 0)
        {
            dsu_union(b->dsu_white, flat, b->W,
                      h->changes, &h->changes_size);
        }
        if (c == b->n - 1)
        {
            dsu_union(b->dsu_white, flat, b->E,
            h->changes, &h->changes_size);
        }


        connect_neighbors(b, h, r, c, WHITE);
    }

    g->current_player = (p == BLACK) ? WHITE : BLACK;
    return 1;
}

void game_undo(GameState* g, GameHistory* h) 
{
    assert(h->size > 0);

    BoardState* b = g->board;
    Move mv = h->moves[--h->size];

    int flat = mv.pos;
    int bb = flat / 64;
    int i  = flat % 64;

    dsu_rollback(b->dsu_black,
                 h->changes, &h->changes_size,
                 mv.snapshot_black);

    dsu_rollback(b->dsu_white,
                 h->changes, &h->changes_size,
                 mv.snapshot_white);

    if (mv.player == BLACK)
    {
        b->black_bits[bb] &= ~(1ULL << i);
    }
    else
    {
        b->white_bits[bb] &= ~(1ULL << i);
    }

    g->current_player = mv.player;
}

int board_check_win(BoardState* b, Player p)
{
    if (p == BLACK)
    {
        return dsu_connected(b->dsu_black, b->N, b->S);
    }
    else
    {
        return dsu_connected(b->dsu_white, b->W, b->E);
    }
}