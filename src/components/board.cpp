#include "board.hpp"
#include <cstdio>
#include <cassert>
#include <cstring>

#include <iostream>


constexpr int dr[6] = {-1, -1, 0, 0, 1, 1};
constexpr int dc[6] = {0, 1, -1, 1, -1, 0};

static BBIndex index_of(int r, int c) {
    int global = r * BOARD_SIZE + c;
    return { global / 64, global % 64 };
}
static bool in_bounds(int r, int c) {
    return r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE;
}
static char stone_char(Player p) {
    if (p == Player::BLACK) return 'O';
    if (p == Player::WHITE) return '@';
    return '.';
}

BoardState::BoardState() {

    memset(black_bits, 0, sizeof(black_bits));
    memset(white_bits, 0, sizeof(white_bits));

    int size = BOARD_SIZE * BOARD_SIZE;
    dsu_black = new DSU(size + 4);
    dsu_white = new DSU(size + 4);

    N = size;
    S = size + 1;
    W = size + 2;
    E = size + 3;
}

BoardState::BoardState(const uint64_t (&black)[3], const uint64_t (&white)[3]) {
    memcpy(black_bits, black, sizeof(black_bits));
    memcpy(white_bits, white, sizeof(white_bits));

    int size = BOARD_SIZE * BOARD_SIZE;
    dsu_black = new DSU(size + 4);
    dsu_white = new DSU(size + 4);
    N = size;
    S = size + 1;
    W = size + 2;
    E = size + 3;

    rebuildDsu(); 

}

BoardState::~BoardState() {
    delete dsu_black;
    delete dsu_white;
}

void BoardState::rebuildDsu() {
    this->draw();

    for (int block = 0; block < 3; block++)
    {
        uint64_t bits = black_bits[block];
        std::cout << "[" << block << "] " << bits << std::endl;

        while (bits)
        {
            int local = __builtin_ctzll(bits);   
            bits &= bits - 1;
            int id = block * 64 + local;     
            int r = id / BOARD_SIZE;
            int c = id % BOARD_SIZE;

            printf("Processing BLACK id=%d (r=%d,c=%d)\n", id, r, c);
            if (!in_bounds(r,c)) { printf("OUT OF BOUNDS!\n"); continue; }

            for (int k = 0; k < 6; k++) {
                int nr = r + dr[k];
                int nc = c + dc[k];

                if (nr < 0 || nr >= BOARD_SIZE || nc < 0 || nc >= BOARD_SIZE)
                    continue;

                int nid = nr * BOARD_SIZE + nc;

                if (getBit(black_bits, nid)) {
                    dsu_black->unite_compress(id, nid);
                }
            }

            if (r == 0) {
                dsu_black->unite_compress(id, N); 
            }
            if (r == BOARD_SIZE - 1) {
                dsu_black->unite_compress(id, S);
            }
        }
    }
    
    for (int block = 0; block < 3; block++)
    {
        uint64_t bits = white_bits[block];
        std::cout << "[" << block << "] " << bits << std::endl;

        while (bits) 
        {
            int local = __builtin_ctzll(bits);   
            
            bits &= bits - 1;
            int id = block * 64 + local;     
            int r = id / BOARD_SIZE;
            int c = id % BOARD_SIZE;

            printf("Processing WHITE id=%d (r=%d,c=%d)\n", id, r, c);
            if (!in_bounds(r,c)) { printf("OUT OF BOUNDS!\n"); continue; }
            for (int k = 0; k < 6; k++) {
                int nr = r + dr[k];
                int nc = c + dc[k];

                if (nr < 0 || nr >= BOARD_SIZE || nc < 0 || nc >= BOARD_SIZE)
                    continue;

                int nid = nr * BOARD_SIZE + nc;

                if (getBit(white_bits, nid)) {
                    dsu_white->unite_compress(id, nid);
                }
            }

            if (c == 0) {
                dsu_white->unite_compress(id, W); 
            }
            if (c == BOARD_SIZE - 1) {
                dsu_white->unite_compress(id, E);
            }
        }
    }
    this->draw();

}



Player BoardState::at(int r, int c) const {
    BBIndex idx = index_of(r, c);

    if ((black_bits[idx.bb] >> idx.i) & 1ULL) return Player::BLACK;
    if ((white_bits[idx.bb] >> idx.i) & 1ULL) return Player::WHITE;
    return Player::EMPTY;
}

bool BoardState::check_win(Player p) {
    if (p == Player::BLACK)
        return dsu_black->connected(N, S);
    else
        return dsu_white->connected(W, E);
}

void BoardState::draw() const {
    printf("  ");
    for (char c = 'A'; c < 'A' + BOARD_SIZE; c++)
        printf("%2c ", c);
    printf("\n");

    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int i = 0; i < r; i++) printf("  ");
        printf("%2d ", r);

        for (int c = 0; c < BOARD_SIZE; c++) {
            printf(" %c ", stone_char(at(r, c)));
        }
        printf("\n");
    }
}

static void connect_neighbors(BoardState* b, GameHistory& h, int r, int c, Player p) {
    int flat = r * BOARD_SIZE + c;

    for (int k = 0; k < 6; k++) {
        int nr = r + dr[k];
        int nc = c + dc[k];

        if (!in_bounds(nr, nc)) continue;
        if (b->at(nr, nc) != p) continue;

        int nflat = nr * BOARD_SIZE + nc;

        if (p == Player::BLACK)
            b->dsu_black->unite(flat, nflat, h.changes, h.changes_size);
        else
            b->dsu_white->unite(flat, nflat, h.changes, h.changes_size);
    }
}

GameState::GameState() {
    board = new BoardState();
    current_player = Player::BLACK;
}
GameState::GameState(const SerializedState &state) {
    board = new BoardState(state.black_bits, state.white_bits);
    current_player = state.current_player;
}

GameState::~GameState() {
    delete board;
}
GameState& GameState::operator=(GameState&& other) {
    if (this != &other) {
        delete board;
        board = other.board;
        other.board = nullptr;
        current_player = other.current_player;
    }
    return *this;
}

bool GameState::make_move(GameHistory& h, int r, int c) {
    if (!in_bounds(r, c)) return false;
    if (board->at(r, c) != Player::EMPTY) return false;

    assert(h.size < MAX_HISTORY);
    make_fast_move(h, r, c);
    return true;
}

void GameState::make_fast_move(GameHistory& h, int r, int c) {
    Player p = current_player;

    BBIndex idx = index_of(r, c);
    int flat = r * BOARD_SIZE + c;

    Move& mv = h.moves[h.size++];

    mv.pos = flat;
    mv.player = p;
    mv.snapshot_black = DSU::snapshot(h.changes_size);
    mv.snapshot_white = DSU::snapshot(h.changes_size);

    if (p == Player::BLACK)
    {
        board->black_bits[idx.bb] |= (1ULL << idx.i);

        if (r == 0)
            board->dsu_black->unite(flat, board->N, h.changes, h.changes_size);
        if (r == BOARD_SIZE - 1)
            board->dsu_black->unite(flat, board->S, h.changes, h.changes_size);

        connect_neighbors(board, h, r, c, p);
    }
    else
    {
        board->white_bits[idx.bb] |= (1ULL << idx.i);

        if (c == 0)
            board->dsu_white->unite(flat, board->W, h.changes, h.changes_size);
        if (c == BOARD_SIZE - 1)
            board->dsu_white->unite(flat, board->E, h.changes, h.changes_size);

        connect_neighbors(board, h, r, c, p);
    }

    current_player = (p == Player::BLACK) ? Player::WHITE : Player::BLACK;
}


void GameState::undo(GameHistory& h) {
    assert(h.size > 0);
    Move mv = h.moves[--h.size];

    int flat = mv.pos;
    int bb = flat / 64;
    int i  = flat % 64;

    board->dsu_black->rollback(h.changes, h.changes_size, mv.snapshot_black);
    board->dsu_white->rollback(h.changes, h.changes_size, mv.snapshot_white);

    if (mv.player == Player::BLACK)
        board->black_bits[bb] &= ~(1ULL << i);
    else
        board->white_bits[bb] &= ~(1ULL << i);

    current_player = mv.player;
}

SerializedState GameState::Serialize() {
    SerializedState state;
    memcpy(state.black_bits, board->black_bits, sizeof(state.black_bits));
    memcpy(state.white_bits, board->white_bits, sizeof(state.white_bits));
    state.current_player = current_player;

    return state;
}