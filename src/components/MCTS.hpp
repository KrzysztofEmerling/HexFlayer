#pragma once

#include "components/board.hpp"
#include "components/config.hpp"

constexpr int MAX_MOVES = BOARD_SIZE * BOARD_SIZE;

struct MCTSNode {
    int move_r;
    int move_c;

    int visits;
    float wins;
    float prior;

    int child_count;
    MCTSNode* children[MAX_MOVES];

    MCTSNode* parent;
    bool fully_expanded;
};

struct MCTSMove {
    int r, c;
};

class MCTS {
public:
    MCTS(int iterations, float c_puct = 1.5f, float temperature = 0.5f);
    ~MCTS();

    MCTSMove run(SerializedState state);

private:
    int iterations;
    float c_puct;
    float temperature;

    MCTSNode* root;

    MCTSNode* create_node(MCTSNode* parent, int r, int c);

    MCTSNode* select(MCTSNode* node);
    void expand(MCTSNode* node, GameState& game);
    Player simulate(GameState& game, GameHistory& history);
    void backpropagate(MCTSNode* node, Player winner, Player ai_player);

    MCTSNode* best_child(MCTSNode* root);
    MCTSNode* select_with_temperature(MCTSNode* root);
    void free_tree(MCTSNode* node);

    float get_prior(GameState& game, int r, int c);
    // heuristics
    float center_bias(int r, int c);
    int count_neighbors(const BoardState* b, int r, int c, Player p);
    // float goal_bias(int r, int c, Player p);

};