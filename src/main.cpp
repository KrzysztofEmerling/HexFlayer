// kąpilacja MPI: mpic++ -std=c++23 -Wall -Wextra src/main.cpp src/components/board.cpp src/components/dsu.cpp src/components/MCTS.cpp -I src -o HexFlayer

// Odpalenie: mpiexec -f nodes -n 16 ./HexFlayer 

#include <mpi.h>
#include <vector>
#include <random>
#include <cstdio>

#include "components/board.hpp"
#include "components/config.hpp"
#include "components/MCTS.hpp"

struct Task {
    SerializedState state;
    int r, c;
};


void generate_root_tasks(const SerializedState& state, std::vector<Task>& tasks) {
    GameState game(state);

    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (game.board->at(r, c) != Player::EMPTY)
                continue;

            GameHistory h{};
            game.make_fast_move(h, r, c);

            Task t;
            t.state = game.Serialize();
            t.r = r;
            t.c = c;
            tasks.push_back(t);

            game.undo(h);
        }
    }
}

void worker_loop() {
    MCTS mcts(MCTS_ITERS, MCTS_C, MCTS_TEMP);

    while (true) {
        Task task;
        MPI_Status status;

        MPI_Recv(&task, sizeof(Task), MPI_BYTE,
                 MASTER, MPI_ANY_TAG,
                 MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == TAG_STOP)
            break;

        MCTSResult res = mcts.run(task.state);
        res.r = task.r;
        res.c = task.c;

        MPI_Send(&res, sizeof(MCTSResult), MPI_BYTE,
                 MASTER, TAG_RESULT, MPI_COMM_WORLD);
    }
}

void master_loop(int world_size) {
    GameState game;
    GameHistory history{};

    while (true) {
        game.board->draw();

        SerializedState current_state = game.Serialize();
        std::vector<Task> tasks;
        generate_root_tasks(current_state, tasks);

        if (tasks.empty()) {
            printf("Brak ruchów\n");
            break;
        }

        int next_task = 0;
        int active = 0;

        std::vector<MCTSResult> results;
        results.reserve(tasks.size());

        // ===== initial dispatch =====
        for (int i = 1; i < world_size && next_task < (int)tasks.size(); i++) {
            MPI_Send(&tasks[next_task++], sizeof(Task), MPI_BYTE,
                     i, TAG_WORK, MPI_COMM_WORLD);
            active++;
        }

        // ===== dynamic scheduling =====
        while (active > 0) {
            MCTSResult res;
            MPI_Status status;

            MPI_Recv(&res, sizeof(MCTSResult), MPI_BYTE,
                     MPI_ANY_SOURCE, TAG_RESULT,
                     MPI_COMM_WORLD, &status);

            results.push_back(res);

            int worker = status.MPI_SOURCE;

            if (next_task < (int)tasks.size()) {
                MPI_Send(&tasks[next_task++], sizeof(Task), MPI_BYTE,
                         worker, TAG_WORK, MPI_COMM_WORLD);
            } else {
                active--;
            }
        }

        float bestScore = -1e9f;
        std::vector<std::pair<int,int>> bestMoves;

        for (auto& r : results) {
            float score = -r.Q;

            if (score > bestScore) {
                bestScore = score;
                bestMoves.clear();
                bestMoves.emplace_back(r.r, r.c);
            }
            else if (score == bestScore) {
                bestMoves.emplace_back(r.r, r.c);
            }
        }
        int idx = rand() % bestMoves.size();
        int best_r = bestMoves[idx].first;
        int best_c = bestMoves[idx].second;

        printf("Ruch: (%d,%d)\n", best_r, best_c);

        game.make_move_fast(history, best_r, best_c);

        Player last =
            (game.current_player == Player::BLACK)
            ? Player::WHITE : Player::BLACK;

        if (game.board->check_win(last)) {
            game.board->draw();
            printf("Wygrywa %s\n",
                   last == Player::BLACK ? "@" : "O");
            break;
        }
    }

    for (int i = 1; i < world_size; i++) {
        MPI_Send(nullptr, 0, MPI_BYTE,
                 i, TAG_STOP, MPI_COMM_WORLD);
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == MASTER) {
        master_loop(size);
    } else {
        worker_loop();
    }

    MPI_Finalize();
    return 0;
}