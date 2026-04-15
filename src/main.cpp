#include <cstdio>
#include <cstring>
#include <cctype>

#include "components/board.hpp"
#include "components/config.hpp"

int main() 
{
    GameState game;
    GameHistory history{};

    while (true) 
    {
        game.board->draw();

        Player p = game.current_player;

        printf("Gracz %s (np. A3, u=cofnij): ",
            (p == Player::BLACK) ? "(O) połącz: NS" : "(@) połącz: WE");

        char input[16];
        if (!fgets(input, sizeof(input), stdin))
            break;

        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "u") == 0) {
            if (history.size > 0) {
                game.undo(history);
            } else {
                printf("Brak ruchów do cofnięcia!\n");
            }
            continue;
        }

        if (strlen(input) < 2) {
            printf("Podaj ruch w formacie 'A1'\n");
            continue;
        }

        char col_char = toupper(input[0]);
        if (col_char < 'A' || col_char >= 'A' + BOARD_SIZE) {
            printf("Niepoprawna kolumna!\n");
            continue;
        }

        int c = col_char - 'A';

        int r;
        if (sscanf(input + 1, "%d", &r) != 1) {
            printf("Niepoprawny wiersz!\n");
            continue;
        }

        if (r < 0 || r >= BOARD_SIZE) {
            printf("Wiersz poza planszą!\n");
            continue;
        }

        // MOVE
        if (!game.make_move(history, r, c)) {
            printf("Niepoprawny ruch!\n");
            continue;
        }

        // WIN CHECK
        if (game.board->check_win(p)) {
            game.board->draw();
            printf("Wygrywa %s!\n",
                (p == Player::BLACK) ? "(O)" : "(@)");
            break;
        }
    }

    return 0;
}