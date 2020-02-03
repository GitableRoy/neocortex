#pragma once

/*
 * Occupancy bitboard.
 */

#include "types.h"

namespace nc2 {
    class Occboard {
        public:
            Occboard(); /* Empty */
            static Occboard standard(); /* Standard board init */

            void flip(u8 s);

            u64 get_board();
            u8 get_rank(u8 r);
            u8 get_file(u8 f);
            u8 get_diag(u8 d);
            u8 get_antidiag(u8 d);

            bool test(u8 s);

            static u8 to_rocc(u8 occ_byte);
        private:
            u8 diags[15], antidiags[15], ranks[8], files[8];
            u64 board;
    };
}
