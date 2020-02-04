#include <iostream>
#include <string>
#include <vector>

#include "unit_test.h"
#include "lookup_pawn.h"
#include "lookup_king.h"
#include "lookup_knight.h"
#include "lookup_bishop.h"
#include "lookup_rook.h"
#include "tests/test_dummy.h"
#include "square.h"
#include "piece.h"
#include "occ.h"

int main(int argc, char** argv) {
    nc2::lookup::initialize_pawn_lookup();
    nc2::lookup::initialize_knight_lookup();
    nc2::lookup::initialize_king_lookup();
    nc2::lookup::initialize_rook_lookup();
    nc2::lookup::initialize_bishop_lookup();

    std::cerr << "lookups ready\n";

    nc2::Occboard occ = nc2::Occboard::standard();

    for (auto m : nc2::lookup::rook_moves(nc2::square::at(3, 3), &occ)) {
        std::cout << "rook move : " << m.to_string() << "\n";
    }

    if (argc == 2 && std::string(argv[1]) == "test") {
        /* Run all tests. */

        int count = 0, ok = 0, failed = 0;
        std::vector<nc2::UnitTest*> tests;

        tests.push_back(new nc2::tests::DummyTest());

        for (auto t : tests) {
            ++count;

            try {
                t->run();

                std::cerr << t->get_name() << " OK\n";
                ++ok;
            } catch (std::exception& e) {
                std::cerr << t->get_name() << " FAILED : " << e.what() << "\n";
                ++failed;
            }
        }

        std::cerr << "Done testing. " << failed << " out of " << count << " tests failed.\n";

        if (failed) return -1;
        return 0;
    }

    return 0;
}