#ifndef SERVER_H
#define SERVER_H

#include <cstdlib>
#include <iostream>
#include <vector>

/*
 * You may need to define some global variables for the information of the game map here.
 * Although we don't encourage to use global variables in real cpp projects, you may have to use them because the use of
 * class is not taught yet. However, if you are member of A-class or have learnt the use of cpp class, member functions,
 * etc., you're free to modify this structure.
 */
int rows;         // The count of rows of the game map. You MUST NOT modify its name.
int columns;      // The count of columns of the game map. You MUST NOT modify its name.
int total_mines;  // The count of mines of the game map. You MUST NOT modify its name. You should initialize this
                  // variable in function InitMap. It will be used in the advanced task.
int game_state;  // The state of the game, 0 for continuing, 1 for winning, -1 for losing. You MUST NOT modify its name.

// Internal state
static std::vector<std::vector<bool>> is_mine;
static std::vector<std::vector<int>> mine_cnt;
static std::vector<std::vector<bool>> visited;
static std::vector<std::vector<bool>> marked;
static int visited_non_mine;
static int marked_correct;

static bool in_bounds(int r, int c) { return r >= 0 && r < rows && c >= 0 && c < columns; }

static int compute_adj(int r, int c) {
  static const int dr[8] = {-1,-1,-1,0,0,1,1,1};
  static const int dc[8] = {-1,0,1,-1,1,-1,0,1};
  int cnt = 0;
  for (int k = 0; k < 8; ++k) {
    int nr = r + dr[k], nc = c + dc[k];
    if (in_bounds(nr, nc) && is_mine[nr][nc]) ++cnt;
  }
  return cnt;
}

static void flood_fill_zero(int r, int c) {
  // BFS/DFS visiting of zero regions and their borders
  std::vector<std::pair<int,int>> stack;
  stack.emplace_back(r, c);
  while (!stack.empty()) {
    auto [cr, cc] = stack.back();
    stack.pop_back();
    if (!in_bounds(cr, cc)) continue;
    if (visited[cr][cc]) continue;
    if (marked[cr][cc]) continue;
    if (is_mine[cr][cc]) continue;
    visited[cr][cc] = true;
    ++visited_non_mine;
    if (mine_cnt[cr][cc] == 0) {
      for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
          if (dr == 0 && dc == 0) continue;
          int nr = cr + dr, nc = cc + dc;
          if (!in_bounds(nr, nc)) continue;
          if (marked[nr][nc]) continue; // do not change marked cells
          if (!is_mine[nr][nc] && !visited[nr][nc]) {
            if (mine_cnt[nr][nc] == 0) {
              stack.emplace_back(nr, nc);
            } else {
              visited[nr][nc] = true;
              ++visited_non_mine;
            }
          }
        }
      }
    }
  }
}

static void check_victory() {
  if (visited_non_mine == rows * columns - total_mines) {
    game_state = 1;
  }
}

/**
 * @brief The definition of function InitMap()
 *
 * @details This function is designed to read the initial map from stdin. For example, if there is a 3 * 3 map in which
 * mines are located at (0, 1) and (1, 2) (0-based), the stdin would be
 *     3 3
 *     .X.
 *     ...
 *     ..X
 * where X stands for a mine block and . stands for a normal block. After executing this function, your game map
 * would be initialized, with all the blocks unvisited.
 */
void InitMap() {
  std::cin >> rows >> columns;
  is_mine.assign(rows, std::vector<bool>(columns, false));
  mine_cnt.assign(rows, std::vector<int>(columns, 0));
  visited.assign(rows, std::vector<bool>(columns, false));
  marked.assign(rows, std::vector<bool>(columns, false));
  total_mines = 0;
  visited_non_mine = 0;
  marked_correct = 0;
  game_state = 0;
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      char ch; std::cin >> ch;
      is_mine[i][j] = (ch == 'X');
      if (is_mine[i][j]) ++total_mines;
    }
  }
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      mine_cnt[i][j] = compute_adj(i, j);
    }
  }
}

/**
 * @brief The definition of function VisitBlock(int, int)
 *
 * @details This function is designed to visit a block in the game map. We take the 3 * 3 game map above as an example.
 * At the beginning, if you call VisitBlock(0, 0), the return value would be 0 (game continues), and the game map would
 * be
 *     1??
 *     ???
 *     ???
 * If you call VisitBlock(0, 1) after that, the return value would be -1 (game ends and the players loses) , and the game map would be
 *     1X?
 *     ???
 *     ???
 * If you call VisitBlock(0, 2), VisitBlock(2, 0), VisitBlock(1, 2) instead, the return value of the last operation
 * would be 1 (game ends and the player wins), and the game map would be
 *     1@1
 *     122
 *     01@
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 *
 * @note You should edit the value of game_state in this function. Precisely, edit it to
 *    0  if the game continues after visit that block, or that block has already been visited before.
 *    1  if the game ends and the player wins.
 *    -1 if the game ends and the player loses.
 *
 * @note For invalid operation, you should not do anything.
 */
void VisitBlock(int r, int c) {
  if (!in_bounds(r, c) || game_state != 0) return;
  if (visited[r][c] || marked[r][c]) { game_state = 0; return; }
  if (is_mine[r][c]) {
    visited[r][c] = true; // reveal mine
    game_state = -1;
    return;
  }
  if (mine_cnt[r][c] == 0) {
    flood_fill_zero(r, c);
  } else {
    visited[r][c] = true;
    ++visited_non_mine;
  }
  check_victory();
}

/**
 * @brief The definition of function MarkMine(int, int)
 *
 * @details This function is designed to mark a mine in the game map.
 * If the block being marked is a mine, show it as "@".
 * If the block being marked isn't a mine, END THE GAME immediately. (NOTE: This is not the same rule as the real
 * game) And you don't need to
 *
 * For example, if we use the same map as before, and the current state is:
 *     1?1
 *     ???
 *     ???
 * If you call MarkMine(0, 1), you marked the right mine. Then the resulting game map is:
 *     1@1
 *     ???
 *     ???
 * If you call MarkMine(1, 0), you marked the wrong mine(There's no mine in grid (1, 0)).
 * The game_state would be -1 and game ends immediately. The game map would be:
 *     1?1
 *     X??
 *     ???
 * This is different from the Minesweeper you've played. You should beware of that.
 *
 * @param r The row coordinate (0-based) of the block to be marked.
 * @param c The column coordinate (0-based) of the block to be marked.
 *
 * @note You should edit the value of game_state in this function. Precisely, edit it to
 *    0  if the game continues after visit that block, or that block has already been visited before.
 *    1  if the game ends and the player wins.
 *    -1 if the game ends and the player loses.
 *
 * @note For invalid operation, you should not do anything.
 */
void MarkMine(int r, int c) {
  if (!in_bounds(r, c) || game_state != 0) return;
  if (visited[r][c] || marked[r][c]) { game_state = 0; return; }
  marked[r][c] = true;
  if (is_mine[r][c]) {
    ++marked_correct;
    check_victory();
  } else {
    game_state = -1;
  }
}

/**
 * @brief The definition of function AutoExplore(int, int)
 *
 * @details This function is designed to auto-visit adjacent blocks of a certain block.
 * See README.md for more information
 *
 * For example, if we use the same map as before, and the current map is:
 *     ?@?
 *     ?2?
 *     ??@
 * Then auto explore is available only for block (1, 1). If you call AutoExplore(1, 1), the resulting map will be:
 *     1@1
 *     122
 *     01@
 * And the game ends (and player wins).
 */
void AutoExplore(int r, int c) {
  if (!in_bounds(r, c) || game_state != 0) return;
  if (!visited[r][c] || is_mine[r][c]) { game_state = 0; return; }
  int k = mine_cnt[r][c];
  int marked_adj = 0;
  for (int dr = -1; dr <= 1; ++dr) {
    for (int dc = -1; dc <= 1; ++dc) {
      if (dr == 0 && dc == 0) continue;
      int nr = r + dr, nc = c + dc;
      if (!in_bounds(nr, nc)) continue;
      if (marked[nr][nc]) ++marked_adj;
    }
  }
  if (marked_adj == k) {
    for (int dr = -1; dr <= 1; ++dr) {
      for (int dc = -1; dc <= 1; ++dc) {
        if (dr == 0 && dc == 0) continue;
        int nr = r + dr, nc = c + dc;
        if (!in_bounds(nr, nc)) continue;
        if (!marked[nr][nc] && !visited[nr][nc] && !is_mine[nr][nc]) {
          if (mine_cnt[nr][nc] == 0) {
            flood_fill_zero(nr, nc);
          } else {
            visited[nr][nc] = true;
            ++visited_non_mine;
          }
        }
      }
    }
    check_victory();
  }
}

/**
 * @brief The definition of function ExitGame()
 *
 * @details This function is designed to exit the game.
 * It outputs a line according to the result, and a line of two integers, visit_count and marked_mine_count,
 * representing the number of blocks visited and the number of marked mines taken respectively.
 *
 * @note If the player wins, we consider that ALL mines are correctly marked.
 */
void ExitGame() {
  if (game_state == 1) {
    std::cout << "YOU WIN!" << std::endl;
    std::cout << visited_non_mine << " " << total_mines << std::endl;
  } else if (game_state == -1) {
    std::cout << "GAME OVER!" << std::endl;
    std::cout << visited_non_mine << " " << marked_correct << std::endl;
  } else {
    std::cout << "GAME OVER!" << std::endl;
    std::cout << visited_non_mine << " " << marked_correct << std::endl;
  }
  exit(0);  // Exit the game immediately
}

/**
 * @brief The definition of function PrintMap()
 *
 * @details This function is designed to print the game map to stdout. We take the 3 * 3 game map above as an example.
 * At the beginning, if you call PrintMap(), the stdout would be
 *    ???
 *    ???
 *    ???
 * If you call VisitBlock(2, 0) and PrintMap() after that, the stdout would be
 *    ???
 *    12?
 *    01?
 * If you call VisitBlock(0, 1) and PrintMap() after that, the stdout would be
 *    ?X?
 *    12?
 *    01?
 * If the player visits all blocks without mine and call PrintMap() after that, the stdout would be
 *    1@1
 *    122
 *    01@
 * (You may find the global variable game_state useful when implementing this function.)
 *
 * @note Use std::cout to print the game map, especially when you want to try the advanced task!!!
 */
void PrintMap() {
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      char out = '?';
      if (game_state == 1 && is_mine[i][j]) {
        out = '@';
      } else if (visited[i][j]) {
        if (is_mine[i][j]) out = 'X';
        else out = static_cast<char>('0' + mine_cnt[i][j]);
      } else if (marked[i][j]) {
        if (is_mine[i][j]) out = '@';
        else out = 'X';
      } else {
        out = '?';
      }
      std::cout << out;
    }
    std::cout << std::endl;
  }
}

#endif
