#pragma once
#include <vector>

struct Cell
{
    bool alive;
    int mass;
    int age;
    int neighbors;
};

class Grid
{
  public:
    std::vector<std::vector<Cell>> cells;
    std::vector<std::vector<Cell>> new_cells;
    int size;
    bool running;
    Grid(int grid_size);
    Grid(Grid &&) = default;
    Grid(const Grid &) = default;
    Grid &operator=(Grid &&) = default;
    Grid &operator=(const Grid &) = default;
    ~Grid();

    void set_rand();
    void update_grid();
    int check_neighbors(size_t row, size_t column);
    void start();
    void stop();
    void clear();

  private:
};
