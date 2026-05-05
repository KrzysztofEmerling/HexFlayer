#include "MCTS.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>

MCTS::MCTS(int iterations, float c_puct, float temperature)
    : iterations(iterations),
      c_puct(c_puct),
      temperature(temperature),
      root(nullptr) {}

MCTS::~MCTS() {
    free_tree(root);
}

void MCTS::free_tree(MCTSNode* node) {
    if (!node) return;

    for (int i = 0; i < node->child_count; i++) {
        free_tree(node->children[i]);
    }

    delete node;
}

MCTSNode* MCTS::create_node(MCTSNode* parent, int r, int c) {
    MCTSNode* node = new MCTSNode();

    node->move_r = r;
    node->move_c = c;
    node->visits = 0;
    node->wins = 0.0f;
    node->prior = 0.0f;

    node->child_count = 0;
    node->parent = parent;
    node->fully_expanded = false;

    return node;
}

float MCTS::center_bias(int r, int c) {
    float cr = (BOARD_SIZE - 1) * 0.5f;
    float cc = (BOARD_SIZE - 1) * 0.5f;

    float d = sqrtf((r-cr)*(r-cr) + (c-cc)*(c-cc));
    return 1.0f / (1.0f + d);
}

int MCTS::count_neighbors(const BoardState* b, int r, int c, Player p) {
    int cnt = 0;

    for (int k = 0; k < 6; k++) {
        int nr = r + dr[k];
        int nc = c + dc[k];

        if (nr < 0 || nr >= BOARD_SIZE || nc < 0 || nc >= BOARD_SIZE)
            continue;

        if (b->at(nr, nc) == p)
            cnt++;
    }

    return cnt;
}

float MCTS::get_prior(GameState& game, int r, int c) {
    BoardState* b = game.board;

    Player me = game.current_player;
    Player opp = (me == Player::BLACK) ? Player::WHITE : Player::BLACK;

    float p = 1.0f;

    p *= center_bias(r, c);
    p *= 1.0f + 0.5f * count_neighbors(b, r, c, me);
    p *= 1.0f + 0.3f * count_neighbors(b, r, c, opp);

    return p;
}

MCTSNode* MCTS::select(MCTSNode* node) {
    while (node && node->fully_expanded && node->child_count > 0) {
        float best_score = -1e9f;
        MCTSNode* best = nullptr;

        for (int i = 0; i < node->child_count; i++) {
            MCTSNode* child = node->children[i];

            float Q = child->visits > 0 ? child->wins / child->visits : 0.0f;

            float U = c_puct * child->prior *
                sqrtf(node->visits + 1.0f) / (child->visits + 1.0f);

            float score = Q + U;

            if (score > best_score) {
                best_score = score;
                best = child;
            }
        }

        if (!best)
            return node;

        node = best;
    }

    return node;
}

void MCTS::expand(MCTSNode* node, GameState& game) {
    float sum_p = 0.0f;

    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (game.board->at(r, c) == Player::EMPTY)
                sum_p += get_prior(game, r, c);
        }
    }

    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (game.board->at(r, c) != Player::EMPTY)
                continue;

            MCTSNode* child = create_node(node, r, c);

            float p = get_prior(game, r, c);
            child->prior = (sum_p > 0.0f) ? (p / sum_p) : 1.0f;

            node->children[node->child_count++] = child;
        }
    }

    node->fully_expanded = true;
}

Player MCTS::simulate(GameState& game, GameHistory& history) {
    int moves[MAX_MOVES][2];
    int move_count = 0;

    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (game.board->at(r, c) == Player::EMPTY) {
                moves[move_count][0] = r;
                moves[move_count][1] = c;
                move_count++;
            }
        }
    }

    while (move_count > 0) {
        int idx = rand() % move_count;

        int r = moves[idx][0];
        int c = moves[idx][1];

        Player who_just_moved = game.current_player;
        game.make_move(history, r, c);

        if (game.board->check_win(who_just_moved))
            return who_just_moved;

        moves[idx][0] = moves[--move_count][0];
        moves[idx][1] = moves[move_count][1];
    }

    return Player::EMPTY;
}

void MCTS::backpropagate(MCTSNode* node, Player winner, Player ai_player) {
    while (node) {
        node->visits++;

        if (winner == ai_player) {
            node->wins += 1.0f;
        } else {
            node->wins -= 1.0f;
        }

        node = node->parent;
    }
}

MCTSNode* MCTS::best_child(MCTSNode* root) {
    int best_visits = -1;
    MCTSNode* best = nullptr;

    for (int i = 0; i < root->child_count; i++) {
        MCTSNode* child = root->children[i];

        if (child->visits > best_visits) {
            best_visits = child->visits;
            best = child;
        }
    }

    return best;
}

MCTSNode* MCTS::select_with_temperature(MCTSNode* root) {
    if (temperature < 0.01f)
        return best_child(root);

    float weights[MAX_MOVES];
    float sum = 0.0f;

    for (int i = 0; i < root->child_count; i++) {
        float v = (float)root->children[i]->visits;
        float w = powf(v + 1e-6f, 1.0f / temperature);

        weights[i] = w;
        sum += w;
    }

    float r = ((float)rand() / RAND_MAX) * sum;

    for (int i = 0; i < root->child_count; i++) {
        r -= weights[i];
        if (r <= 0.0f)
            return root->children[i];
    }

    return root->children[root->child_count - 1];
}

MCTSResult MCTS::run(SerializedState state) {
    free_tree(root);
    root = create_node(nullptr, -1, -1);

    GameState game(state);
    GameHistory history{};

    Player ai_player = game.current_player;

    for (int i = 0; i < iterations; i++) {
        int history_start = history.size;

        MCTSNode* node = select(root);

        if (!node->fully_expanded)
            expand(node, game);

        if (node->child_count > 0) {
            node = node->children[rand() % node->child_count];
            game.make_move(history, node->move_r, node->move_c);
        }

        Player winner = simulate(game, history);

        backpropagate(node, winner, ai_player);

        while (history.size > history_start)
            game.undo(history);
    }

    MCTSNode* best = select_with_temperature(root);

    if (best)
        return MCTSResult(best->wins/best->visits ,best->move_r, best->move_c);
    return MCTSResult(0.0f, -1, -1);
}