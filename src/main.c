#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "components/board.h"

int main() 
{
    int n = 11; 

    GameState* game = game_create(n);
    GameHistory history = {0};

    while (1) 
    {
        board_draw(game->board);

        Player p = game->current_player;
        printf("Gracz %s (np. A3, u=cofnij): ",
            (p == BLACK) ? "BLACK (O)" : "WHITE (@)");

        char input[16];
        if (!fgets(input, sizeof(input), stdin))
            break;
        input[strcspn(input, "\n")] = 0;
        if (strcmp(input, "u") == 0) {
            if (history.size > 0) {
                game_undo(game, &history);
            } else {
                printf("Brak ruchów do cofnięcia!\n");
            }
            continue;
        }

        if (strlen(input) < 2) {
            printf("Podaj ruch w formacie 'A1'");
            continue;
        }
        char col_char = toupper(input[0]);
        if (col_char < 'A' || col_char >= 'A' + game->board->n) {
            printf("Niepoprawna kolumna!\n");
            continue;
        }
        int c = col_char - 'A';
        int r;
        if (sscanf(input + 1, "%d", &r) != 1) {
            printf("Niepoprawny wiersz!\n");
            continue;
        }

        if (r < 0 || r >= game->board->n) {
            printf("Wiersz poza planszą!\n");
            continue;
        }
        if (!game_make_move(game, &history, r, c)) {
            printf("Niepoprawny ruch!\n");
            continue;
        }


        if (board_check_win(game->board, p)) {
            board_draw(game->board);
            printf("WYGRYWA %s!\n",
                (p == BLACK) ? "BLACK (O)" : "WHITE (@)");
            break;
        }
    }

    game_free(game);
    return 0;
}