#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <charconv>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <ranges>
#include <stdexcept>
#include <string_view>

#include "../include/grid.h"
#include "SDL3/SDL_scancode.h"

// Constants and structs
constexpr int WINDOW_HEIGHT = 720;
constexpr int WINDOW_WIDTH = 720;
constexpr int CELL_NUMBER = 100;
constexpr int CELL_SIZE = WINDOW_WIDTH / 100;
constexpr int PADDING = (WINDOW_WIDTH % 100)/2;
typedef struct Color
{
    int r;
    int g;
    int b;
    int a;
} Color;

// Forward Declarations
void initGrid(Grid grid);
void drawGrid(Grid grid);
bool has_option(const std::vector<std::string_view> &args, const std::string_view &option_name);
std::string_view get_option(const std::vector<std::string_view> &args, const std::string_view &option_name);

// Make Window and Renderer availabe to everything in this file
SDL_Window *window;
SDL_Renderer *renderer;

int main(int argc, char *argv[])
{
    const std::vector<std::string_view> args(argv, argv + argc);
    bool nogui = has_option(args, "-nogui");
    bool fast = has_option(args, "-fast");
    std::string_view max_cycles_str = get_option(args, "-c");
    int max_cycles{-1};
    if (max_cycles_str != "")
    {
        auto result = std::from_chars(max_cycles_str.data(), max_cycles_str.data() + max_cycles_str.size(), max_cycles);
        if (result.ec == std::errc::invalid_argument)
        {
            std::cerr << "Please enter an integer\n";
            std::exit(-1);
        }
        else if (result.ec == std::errc::result_out_of_range)
        {
            std::cerr << "Number too large to fit in an int.\n";
            std::exit(-1);
        }
        else if (result.ptr != (max_cycles_str.data() + max_cycles_str.size()))
        {
            std::cerr << "Please enter an integer\n";
            std::exit(-1);
        }
    }
    std::string_view filename = get_option(args, "-f");
    std::string full_filename;
    if (filename == "")
    {
        full_filename = "output.csv";
    }
    else
    {
        full_filename = std::string(filename) + std::string(".csv");
    }
    // Initialize SDL
    if (!nogui)
    {
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
            return 1;
        }

        // Create a window
        window = SDL_CreateWindow("CH482: Entropic Game Of Life", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
        if (!window)
        {
            std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return 1;
        }

        // Create a renderer
        renderer = SDL_CreateRenderer(window, 0);
        if (!renderer)
        {
            std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }
    }
    Grid grid = Grid(CELL_NUMBER);
    Grid update_grid = grid;
    std::ofstream outFile;
    outFile.open(full_filename);
    outFile << "Cycles,Count_0,Count_1,Count_2,Count_3,Count_4,Count_5,Count_6,Count_7,Count_8,Count_9,Count_10,Total_"
               "Mass\n";

    // Main loop
    bool run = true;
    SDL_Event event;

    std::map<SDL_Scancode, bool> keyState;
    size_t cycles{1};
    while (run)
    {
        if (grid.running)
        {
            if (max_cycles > 0)
            {
                if (cycles >= max_cycles - 1)
                {
                    run = false;
                }
                cycles += 1;
            }
        }

        // Handle events
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                run = false;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN)
            {
                keyState[event.key.scancode] = true;
            }
            else if (event.type == SDL_EVENT_KEY_UP)
            {
                keyState[event.key.scancode] = false;
            }
        }

        if (keyState[SDL_SCANCODE_R])
        {
            grid.set_rand();
        }
        else if (keyState[SDL_SCANCODE_RETURN])
        {
            grid.start();
        }
        else if (keyState[SDL_SCANCODE_SPACE])
        {
            grid.stop();
        }
        else if (keyState[SDL_SCANCODE_BACKSPACE])
        {
            grid.clear();
        }

        // Update and draw grid

        if (nogui)
        {
            grid.set_rand();
        }

        drawGrid(grid);

        if (nogui)
        {
            grid.start();
        }

        grid.update_grid();

        if (!fast)
        {
            SDL_Delay(100);
        }

        if (grid.running && max_cycles > 0)
        {
            // Output data
            if (outFile.is_open())
            {
                auto mv = std::ranges::join_view(grid.cells);
                std::map<int, int> mass_counts;
                auto total_mass = std::accumulate(std::begin(mv), std::end(mv), 0,
                                                  [](auto sum, auto cell) { return sum + cell.mass; });
                for (auto &cell : mv)
                {
                    mass_counts[cell.mass] += 1;
                }
                outFile << cycles << ",";
                /*
                                std::cout << cycles << std::endl;
                                // std::cout << "Total Mass: " << total_mass << std::endl;
                                double entropy{0};
                                for (auto &mass : mass_counts)
                                {
                                    // std::cout << mass.first << ", " << mass.second << std::endl;
                                    entropy -=
                                        (static_cast<double>(mass.second) / 1600) *
                   std::log2(static_cast<double>(mass.second) / 1600);
                                }
                */

                for (size_t i = 0; i < 11; i++)
                {
                    if (mass_counts.count(i) == 0)
                    {
                        outFile << 0 << ",";
                    }
                    else
                    {
                        outFile << mass_counts.at(i) << ",";
                    }
                }
                outFile << total_mass << "\n";
                //                std::cout << entropy << std::endl;
            }
        }
    }

    // Cleanup
    outFile.close();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

// Function Definitions
void drawGrid(Grid grid)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    Color cell_color;
    for (auto i = 0; i < grid.size; i++)
    {
        for (auto j = 0; j < grid.size; j++)
        {
            if (grid.cells[i][j].alive)
            {
                int mass = grid.cells[i][j].mass;
                float norm = static_cast<float>(mass) / (1 + mass);
                int alpha = static_cast<int>(255 * norm);
                cell_color = {0, 255, 0, alpha};
            }
            else
            {
                cell_color = {55, 55, 55, 255};
            }
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, cell_color.r, cell_color.g, cell_color.b, cell_color.a);
            SDL_FRect rect = {static_cast<float>(PADDING + j * CELL_SIZE), static_cast<float>(PADDING + i * CELL_SIZE),
                              static_cast<float>(CELL_SIZE - 1), static_cast<float>(CELL_SIZE - 1)};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    // Update screen
    SDL_RenderPresent(renderer);
}

void parse(int argc, char *argv[])
{
    if (argc > 64)
    {
        throw std::runtime_error("Too many input parameters!");
    }
    const std::vector<std::string_view> args(argv + 1, argv + argc);
}

std::string_view get_option(const std::vector<std::string_view> &args, const std::string_view &option_name)
{
    for (auto it = args.begin(), end = args.end(); it != end; it++)
    {
        if (*it == option_name)
        {
            if (it + 1 != end)
            {
                return *(it + 1);
            }
        }
    }
    return "";
}

bool has_option(const std::vector<std::string_view> &args, const std::string_view &option_name)
{
    for (auto it = args.begin(), end = args.end(); it != end; it++)
    {
        if (*it == option_name)
            return true;
    }
    return false;
}
