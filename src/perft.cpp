#include "perft.h"
#include "movegen.h"
#include "util.h"
#include "log.h"

#include <cassert>
#include <iostream>

using namespace pine;

static perft::results current_results;
static void perft_movegen(Position& p, int depth);

std::string perft::results::header() {
	return "| depth |     nodes |   captures |   checks | castles |   time |       nps |\n";
}

std::string perft::results::to_row(int depth) {
	return util::format("| %5d | %9d | %10d | %8d | %7d | %6.2f | %9d |\n", depth, nodes, captures, checks, castles, totaltime, nps);
}

perft::results perft::run(Position& p, int depth) {
	if (depth < 0) {
		throw util::fmterr("Invalid perft depth %d", depth);
	}

	current_results.nodes = 0;
	current_results.captures = 0;
	current_results.castles = 0;
	current_results.checks = 0;

	util::time_point now = util::now();

	perft_movegen(p, depth);

	current_results.totaltime = util::elapsed(now);
	current_results.nps = current_results.nodes / current_results.totaltime;

	return current_results;
}

void perft::write_run(Position& p, int depth, std::ostream& out) {
	out << results::header();

	for (int i = 1; i <= depth; ++i) {
		out << perft::run(p, i).to_row(i);
	}

	out << "end of perft.\n";
}

void perft_movegen(Position& p, int depth) {
	if (!depth) return;

	movegen::Generator g(p);

	for (auto movelist : g.generate_perft()) {
		if (!movelist.size()) continue;

		for (auto next_move : movelist) {
			if (p.make_move(next_move)) {
				/* Move was legal, update perft results */

				if (depth == 1) {
					if (next_move.get(Move::CAPTURE)) current_results.captures++;
					if (next_move.get(Move::CASTLE_KINGSIDE | Move::CASTLE_QUEENSIDE)) {
						current_results.castles++;
					}
					if (p.check()) current_results.checks++;
					current_results.nodes++;
				}

				perft_movegen(p, depth - 1);
			}

			p.unmake_move(next_move);
		}
	}
}