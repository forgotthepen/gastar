#include "gastar/gastar.hpp"
#include <iostream>
#include <exception>
#include <stdexcept> // std::runtime_error
#include <string>
#include <vector>
#include <utility> // std::pair
#include <cmath>
#include <thread>
#include <chrono>


gastar::gastar astar;
std::vector<std::vector<char>> grid = {
  { 1, 1, 1, 1, 1, 0, 0, 1, },
  { 1, 1, 1, 1, 1, 0, 0, 1, },
  { 1, 1, 0, 0, 1, 0, 0, 1, },
  { 1, 1, 0, 0, 1, 1, 1, 1, },
  { 1, 1, 0, 0, 1, 1, 1, 1, },
  { 1, 1, 1, 1, 1, 0, 0, 1, },
  { 1, 1, 1, 1, 1, 0, 0, 1, },
  { 1, 1, 1, 1, 1, 0, 0, 1, },
};

void draw() {
  for (auto &row : grid) {
    for (auto col_val : row) {
      char tile = '?';
      if (1 == col_val) {
        tile = '_';
      } else if (0 == col_val) {
        tile = '#';
      } else if (2 == col_val) {
        tile = '|';
      }
      std::cout << tile << ' ';
    }
    std::cout <<'\n';
  }
}

struct s_neighbor {
  enum class e_dir {
    UP, UP_RIGHT,
    RIGHT, RIGHT_DOWN,
    DOWN, DOWN_LEFT,
    LEFT, LEFT_UP,
    // last
    RESET,
  };
  const void *current = nullptr;
  e_dir dir = e_dir::UP;

  std::pair<unsigned /*y*/, unsigned /*x*/> get_coords(const void *node) const {
    for (unsigned row_idx = 0; row_idx < grid.size(); ++row_idx) {
      auto &row = grid[row_idx];
      if (node >= &row.front() && node <= &row.back()) {
        auto y_coord = reinterpret_cast<const char*>(node) - reinterpret_cast<const char*>(&row.front());
        return { row_idx, y_coord };
      }
    }

    throw std::runtime_error("invalid node, not found on grid");
  }

  const void* operator() (const void *node) {
    if (current != node) {
      current = node;
      dir = e_dir::UP;
    }
    
    const auto coords = get_coords(node);
    const unsigned y_idx = coords.first;
    const unsigned x_idx = coords.second;
    while(true) {
      switch (dir) {
      case e_dir::UP: {
        dir = e_dir::UP_RIGHT;
        if (y_idx > 0) {
          return &grid[y_idx - 1][x_idx];
        }
      }
      break;

      case e_dir::UP_RIGHT: {
        dir = e_dir::RIGHT;
        if (y_idx > 0 && x_idx < grid.front().size() - 1) {
          return &grid[y_idx - 1][x_idx + 1];
        }
      }
      break;

      case e_dir::RIGHT: {
        dir = e_dir::RIGHT_DOWN;
        if (x_idx < grid.front().size() - 1) {
          return &grid[y_idx][x_idx + 1];
        }
      }
      break;

      case e_dir::RIGHT_DOWN: {
        dir = e_dir::DOWN;
        if (x_idx < grid.front().size() - 1 && y_idx < grid.size() - 1) {
          return &grid[y_idx + 1][x_idx + 1];
        }
      }
      break;

      case e_dir::DOWN: {
        dir = e_dir::DOWN_LEFT;
        if (y_idx < grid.size() - 1) {
          return &grid[y_idx + 1][x_idx];
        }
      }
      break;

      case e_dir::DOWN_LEFT: {
        dir = e_dir::LEFT;
        if (y_idx < grid.size() - 1 && x_idx > 0) {
          return &grid[y_idx + 1][x_idx - 1];
        }
      }
      break;

      case e_dir::LEFT: {
        dir = e_dir::LEFT_UP;
        if (x_idx > 0) {
          return &grid[y_idx][x_idx - 1];
        }
      }
      break;

      case e_dir::LEFT_UP: {
        dir = e_dir::RESET;
        if (x_idx > 0 && y_idx > 0) {
          return &grid[y_idx - 1][x_idx - 1];
        }
      }
      break;

      case e_dir::RESET: {
        current = nullptr;
        return nullptr;
      }
      break;
      }
    }
  }
};

int main() {
  std::cout << "hello\n" << '\n';
  try {
    astar.setup(
      &grid.front().front(), &grid.back().back(),
      // distance
      [](const void *from, const void *to, const void*, const void*){
        auto p1 = reinterpret_cast<const char*>(from);
        auto p2 = reinterpret_cast<const char*>(to);
        return (unsigned)(10 * sqrt(*p1 * *p2));
      },
      // heuristics
      [](const void *node, const void*, const void*, const void *end){
        auto p1 = reinterpret_cast<const char*>(node);
        auto p2 = reinterpret_cast<const char*>(end);
        return (unsigned)(10 * sqrt(*p1 * *p2));
      },
      // neighbor
      s_neighbor{},
      // usable
      [](const void *node, const void*, const void*, const void*){
        auto ppp = reinterpret_cast<const char*>(node);
        return *ppp != 0;
      }
    );

    draw();
    bool done = false;

    do {
      done = astar.solve_next(false);
    } while (!done);

    if (astar.is_solved()) {
      astar.reconstruct_end([](const void *node){
        auto point = reinterpret_cast<char *>(const_cast<void *>(node));
        *point = 2;
        std::cout << "\n\n";
        draw();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
      });
    } else if (astar.is_done()) {
      astar.reconstruct_current([](const void *node){
        auto point = reinterpret_cast<char *>(const_cast<void *>(node));
        *point = 2;
        std::cout << "\n\n";
        draw();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
      });
    } else {
      throw std::runtime_error("path not solved!");
    }

    // draw();
  } catch (const std::exception &ex) {
    std::cerr << "Error: " << ex.what() << '\n';
    return 1;
  }


  std::cout << "\n\nbye!!" << '\n';
  return 0;
}
