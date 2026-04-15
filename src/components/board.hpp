#pragma once

#include <cstdint>
#include "dsu.hpp"
#include "config.hpp"

struct BBIndex {
    int bb;
    int i;
};

enum class Player {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
};

struct Move {
    int pos;
    Player player;
    int snapshot_black;
    int snapshot_white;
};

struct GameHistory {
    Move moves[MAX_HISTORY];
    int size = 0;

    DSUChange changes[MAX_CHANGES];
    int changes_size = 0;
};

class BoardState {
public:
    uint64_t black_bits[3]{};
    uint64_t white_bits[3]{};

    DSU* dsu_black;
    DSU* dsu_white;


    int N, S, W, E;

    BoardState();
    ~BoardState();

    Player at(int r, int c) const;
    void draw() const;

    bool check_win(Player p);
};

class GameState {
public:
    BoardState* board;
    Player current_player;

    GameState();
    ~GameState();

    bool make_move(GameHistory& h, int r, int c);
    void make_fast_move(GameHistory& h, int r, int c);
    void undo(GameHistory& h);
};