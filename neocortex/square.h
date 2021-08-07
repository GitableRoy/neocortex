/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#pragma once

#include "util.h"

#include <string>

namespace neocortex {
	namespace square {
		constexpr int null = -1;

		/**
		 * Tests if a square is non-null.
		 *
		 * @param sq Input square.
		 * @return true if square is not null, false otherwise.
		 */
		inline bool is_valid(int sq) {
			return sq >= 0 && sq < 64;
		}

		/**
		 * Gets a square from board coordinates.
		 *
		 * @param rank Input rank (0-7 inclusive).
		 * @param file Input file (0-7 inclusive).
		 */
		inline int at(int rank, int file) {
			if (rank < 0 || rank >= 8 || file < 0 || file >= 8)
			  throw util::logerr("Cannot get a square at: (%d, %d)", rank, file);
			return rank * 8 + file;
		}

		/**
		 * Parses a square from a FEN/UCI string.
		 *
		 * @param Input string.
		 * @return Encoded square.
		 */
		int from_uci(std::string uci);

		/**
		 * Gets a square's rank.
		 *
		 * @param sq Input square.
		 * @return Square rank index.
		 */
		inline int rank(int sq) {
			if (!is_valid(sq))
				throw util::logerr("Cannot get rank with invalid square: %d", sq);
			return sq >> 3;
		}

		/**
		 * Gets a square's file.
		 *
		 * @param sq Input square.
		 * @return Square file index.
		 */
		inline int file(int sq) {
			if (!is_valid(sq)) 
			  throw util::logerr("Cannot get file with invalid square: %d", sq);
			return sq & 7;
		}

		/**
		 * Converts a square to a FEN/UCI string.
		 *
		 * @param sq Input square.
		 * @return FEN string.
		 */
		std::string to_uci(int sq);
	}
}
