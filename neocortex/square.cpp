/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "square.h"
#include "util.h"

using namespace neocortex;

int square::from_uci(std::string uci) {
	if (uci == "-") {
		return square::null;
	} else {
		if (uci.size() != 2)
		  throw util::logerr("Cannot make square from uci: %s", uci.c_str());
		return at(uci[1] - '1', uci[0] - 'a');
	}
}

std::string square::to_uci(int sq) {
	if (sq == square::null) {
		return "-";
	}

	std::string output;

	output += 'a' + file(sq);
	output += '1' + rank(sq);

	return output;
}