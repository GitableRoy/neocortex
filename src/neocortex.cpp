#include "log.h"
#include "lookup.h"
#include "bitboard.h"
#include "position.h"
#include "move.h"
#include "game.h"

#include <iostream>

int main(int argc, char** argv) {
    nc::log_init();
    nc_info("Started neocortex.");

    nc::lookup::init();
    nc_info("Built attack lookups.");

    /* Start a game and run it from the command line. */
    nc::Game g;

    std::cout << "Started a game. Enter in some moves:\n";
    while (1) {
        auto x = g.get_legal_next_moves();

        if (x.size() < 4) {
            std::cout << "[";
            for (auto m : x) {
                std::cout << " " << m.first.to_string() << " ";
            }
            std::cout << "]";
        } else {
            std::cout << "(" << std::to_string(x.size()) << " legal moves)";
        }

        std::cout << "> ";
        std::string inp;

        std::cin >> inp;

        try {
            g.apply(inp);

            std::cout << "Applied matched move " << inp << "\n";
            std::cout << "New game FEN: " << g.get_current_position().get_fen() << "\n";
        } catch (std::exception& e) {
            std::cerr << "Caught exception: " << e.what() << "\n";
        }
    }

    return 0;
}
