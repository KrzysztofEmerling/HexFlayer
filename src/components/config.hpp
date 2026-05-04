#pragma once

// wielkość plansz 11 - turniejowa (największa)
constexpr int BOARD_SIZE = 11; 

// układ siatki
constexpr int dr[6] = {-1, -1, 0, 0, 1, 1};
constexpr int dc[6] = {0, 1, -1, 1, -1, 0};

// Tabele Snapshotów
constexpr int MAX_HISTORY = 512;
constexpr int MAX_CHANGES = 2048;

