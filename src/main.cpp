#include <cstdio>
#include "components/board.hpp"
#include "components/config.hpp"
#include "components/MCTS.hpp"

int main() 
{
    GameState game;
    GameHistory history{};

    MCTS ai_black(50000, 1.5f, 1.0f);
    MCTS ai_white(50000, 1.5f, 1.0f);

    while (true) 
    {
        game.board->draw();

        SerializedState state = game.Serialize();
        Player p = game.current_player;

        if (p == Player::BLACK) {
            printf("AI (O) ruch...\n");
            MCTSMove mv = ai_black.run(state);
            game.make_move(history, mv.r, mv.c);

            if (game.board->check_win(Player::BLACK)) {
                game.board->draw();
                printf("Wygrywa (O)!\n");
                break;
            }
        }
        else {
            printf("AI (@) ruch...\n");
            MCTSMove mv = ai_white.run(state);
            game.make_move(history, mv.r, mv.c);

            if (game.board->check_win(Player::WHITE)) {
                game.board->draw();
                printf("Wygrywa (@)!\n");
                break;
            }
        }
    }

    return 0;
}