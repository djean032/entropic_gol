#include "../include/grid.h"
#include <chrono>
#include <random>

Grid::Grid(int grid_size)
{
    for (auto i = 0; i < grid_size; i++)
    {
        std::vector<Cell> row_cells;
        for (auto j = 0; j < grid_size; j++)
        {
            Cell tmp_cell = Cell{false, 0, 0};
            row_cells.push_back(tmp_cell);
        }
        cells.push_back(row_cells);
    }
    size = grid_size;
    new_cells = cells;
    running = false;
}

Grid::~Grid()
{
}

void Grid::set_rand()
{
    if (!running)
    {
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator(seed);
        std::uniform_int_distribution<int> distributionInteger(1, 3);
        std::bernoulli_distribution distributionBool(0.25);
        for (auto i = 0; i < this->size / 4; i++)
        {
            for (auto j = 0; j < this->size / 4; j++)
            {
                cells[i][j].alive = distributionBool(generator);
                cells[i][j].age = 0;
                if (cells[i][j].alive)
                {
                    cells[i][j].mass = distributionInteger(generator);
                }
                else
                {
                    cells[i][j].mass = 0;
                }
            }
        }
    }
}

void Grid::update_grid()
{
    if (running)
    {
        new_cells = cells;
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine generator(seed);
        std::bernoulli_distribution alive(0.008);
        std::bernoulli_distribution kill(0.99);
        for (auto i = 0; i < this->size; i++)
        {
            for (auto j = 0; j < this->size; j++)
            {
                int count = Grid::check_neighbors(i, j);
                bool cell_value = cells[i][j].alive;
                if (cell_value)
                {
                    if (count > 5 || count < 2)
                    {
                        new_cells[i][j].alive = alive(generator);
                        if (!new_cells[i][j].alive)
                        {
                            new_cells[i][j].mass = 0;
                            new_cells[i][j].age = 0;
                        }
                    }
                    else if (cells[i][j].age > 12)
                    {
                        new_cells[i][j].alive = alive(generator);
                    }
                    else
                    {
                        new_cells[i][j].alive = kill(generator);
                        if (new_cells[i][j].alive)
                        {
                            if (cells[i][j].mass < 10)
                            {
                                new_cells[i][j].mass = cells[i][j].mass + 1;
                            }
                            else
                            {
                                new_cells[i][j].mass = cells[i][j].mass;
                            }
                            new_cells[i][j].age = cells[i][j].age + 1;
                        }
                        else
                        {
                            new_cells[i][j].mass = 0;
                            new_cells[i][j].age = 0;
                        }
                    }
                }
                else
                {
                    if (count == 3)
                    {
                        new_cells[i][j].alive = kill(generator);
                        if (new_cells[i][j].alive)
                        {
                            new_cells[i][j].mass = 1;
                            new_cells[i][j].age = 1;
                        }
                    }
                    else
                    {
                        new_cells[i][j].alive = alive(generator);
                        if (!new_cells[i][j].alive)
                        {
                            new_cells[i][j].mass = 0;
                            new_cells[i][j].age = 0;
                        }
                    }
                }
            }
        }
        cells = new_cells;
    }
}

int Grid::check_neighbors(size_t row, size_t column)
{
    int count{0};
    int check_numbers[3]{-1, 0, 1};
    for (auto &ridx : check_numbers)
    {
        for (auto &cidx : check_numbers)
        {
            if ((row + ridx) > (this->size - 1) || (row + ridx) < 0 || (column + cidx) > (this->size - 1) ||
                (column + cidx) < 0)
            {
                continue;
            }
            else if (cells[row + ridx][column + cidx].alive)
            {
                count += 1;
            }
        }
    }
    cells[row][column].neighbors = count;
    return count;
}

void Grid::start()
{
    running = true;
}

void Grid::stop()
{
    running = false;
}

void Grid::clear()
{
    for (auto i = 0; i < this->size; i++)
    {
        for (auto j = 0; j < this->size; j++)
        {
            cells[i][j].alive = false;
            cells[i][j].age = 0;
            cells[i][j].mass = 0;
        }
    }
}
