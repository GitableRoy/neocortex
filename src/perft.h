#pragma once

#include "position.h"

namespace pine {
	namespace perft {
		struct results {
			unsigned long nodes;
			unsigned long checks;
			unsigned long captures;
			unsigned long castles;
			unsigned long nps;

			double totaltime;

			std::string to_row(int for_depth);
			static std::string header();
		};

		results run(Position& p, int depth); /* CANNOT be used from multiple threads. */

		void write_run(Position& p, int depth, std::ostream& out);
	}
}