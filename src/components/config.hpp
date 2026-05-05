#pragma once

// wielkość plansz 11 - turniejowa (największa)
constexpr int BOARD_SIZE = 11; 

// układ siatki
constexpr int dr[6] = {-1, -1, 0, 0, 1, 1};
constexpr int dc[6] = {0, 1, -1, 1, -1, 0};

// Tabele Snapshotów
constexpr int MAX_HISTORY = 512;
constexpr int MAX_CHANGES = 2048;

//Ustawiania symulacji MCTS
constexpr int MCTS_ITERS = 10000;
constexpr float MCTS_C = 2.0f;
constexpr float MCTS_TEMP = 0.95f;

//mpi
constexpr int MASTER = 0;
constexpr int TAG_WORK   = 1;
constexpr int TAG_RESULT = 2;
constexpr int TAG_STOP   = 3;

