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
  // 2.5) Subset inference (simple 1-2 pattern)
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (!(seen[i][j] >= '0' && seen[i][j] <= '8')) continue;
      int numA = seen[i][j] - '0';
      int markedA = count_neighbors(i, j, '@');
      int needA = numA - markedA;
      // Collect unknown neighbors of A
      std::vector<std::pair<int,int>> uA;
      for (int dr = -1; dr <= 1; ++dr) for (int dc = -1; dc <= 1; ++dc) {
        if (dr==0 && dc==0) continue; int nr=i+dr,nc=j+dc; if (nr<0||nr>=rows||nc<0||nc>=columns) continue; if (seen[nr][nc]=='?') uA.emplace_back(nr,nc);
      }
      if (uA.empty()) continue;
      for (int r2 = std::max(0,i-1); r2 <= std::min(rows-1,i+1); ++r2) {
        for (int c2 = std::max(0,j-1); c2 <= std::min(columns-1,j+1); ++c2) {
          if (r2==i && c2==j) continue;
          if (!(seen[r2][c2] >= '0' && seen[r2][c2] <= '8')) continue;
          int numB = seen[r2][c2] - '0';
          int markedB = count_neighbors(r2, c2, '@');
          int needB = numB - markedB;
          std::vector<std::pair<int,int>> uB;
          for (int dr = -1; dr <= 1; ++dr) for (int dc = -1; dc <= 1; ++dc) {
            if (dr==0 && dc==0) continue; int nr=r2+dr,nc=c2+dc; if (nr<0||nr>=rows||nc<0||nc>=columns) continue; if (seen[nr][nc]=='?') uB.emplace_back(nr,nc);
          }
          if (uB.empty()) continue;
          // Check if uA subset of uB
          bool subset = true;
          for (auto &p : uA) {
            bool found=false; for (auto &q: uB){ if (q.first==p.first && q.second==p.second){ found=true; break; } }
            if (!found){ subset=false; break; }
          }
          if (!subset) continue;
          int diff = needB - needA;
          if (diff > 0) {
            // Then cells in uB - uA are mines if size equals diff
            std::vector<std::pair<int,int>> diffset;
            for (auto &q: uB) {
              bool inA=false; for (auto &p: uA){ if (p.first==q.first && p.second==q.second){ inA=true; break; } }
              if (!inA) diffset.push_back(q);
            }
            if ((int)diffset.size() == diff) {
              auto tgt = diffset[0];
              Execute(tgt.first, tgt.second, 1);
              return;
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
