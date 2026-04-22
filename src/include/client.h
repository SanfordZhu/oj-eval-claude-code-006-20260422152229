#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>
#include <vector>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

void Execute(int r, int c, int type);

static std::vector<std::vector<char>> seen;
static bool ended;

void InitGame() {
  ended = false;
  seen.assign(rows, std::vector<char>(columns, '?'));
  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
}

void ReadMap() {
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      char ch; std::cin >> ch;
      seen[i][j] = ch;
    }
  }
}

// Helper to count neighbors by predicate
static int count_neighbors(int r, int c, char target) {
  int cnt = 0;
  for (int dr = -1; dr <= 1; ++dr) {
    for (int dc = -1; dc <= 1; ++dc) {
      if (dr == 0 && dc == 0) continue;
      int nr = r + dr, nc = c + dc;
      if (nr < 0 || nr >= rows || nc < 0 || nc >= columns) continue;
      if (seen[nr][nc] == target) ++cnt;
    }
  }
  return cnt;
}

void Decide() {
  // 1) Auto-explore whenever possible
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (seen[i][j] >= '0' && seen[i][j] <= '8') {
        int num = seen[i][j] - '0';
        int marked_adj = count_neighbors(i, j, '@');
        int unknown_adj = count_neighbors(i, j, '?');
        if (marked_adj == num && unknown_adj > 0) {
          Execute(i, j, 2);
          return;
        }
      }
    }
  }
  // 2) If unknown_adj == number - marked_adj, mark a mine
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (seen[i][j] >= '0' && seen[i][j] <= '8') {
        int num = seen[i][j] - '0';
        int marked_adj = count_neighbors(i, j, '@');
        int unknown_adj = count_neighbors(i, j, '?');
        if (unknown_adj > 0 && num - marked_adj == unknown_adj) {
          for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
              if (dr == 0 && dc == 0) continue;
              int nr = i + dr, nc = j + dc;
              if (nr < 0 || nr >= rows || nc < 0 || nc >= columns) continue;
              if (seen[nr][nc] == '?') { Execute(nr, nc, 1); return; }
            }
          }
        }
      }
    }
  }
  // 3) Otherwise, visit an unknown neighbor near a discovered number, else any unknown
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (seen[i][j] == '?') {
        bool near_number = false;
        for (int dr = -1; dr <= 1 && !near_number; ++dr) {
          for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            int nr = i + dr, nc = j + dc;
            if (nr < 0 || nr >= rows || nc < 0 || nc >= columns) continue;
            if (seen[nr][nc] >= '0' && seen[nr][nc] <= '8') { near_number = true; break; }
          }
        }
        if (near_number) { Execute(i, j, 0); return; }
      }
    }
  }
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (seen[i][j] == '?') { Execute(i, j, 0); return; }
    }
  }
}

#endif
