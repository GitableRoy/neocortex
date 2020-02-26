#pragma once

/*
 * Evaluation heuristic
 */

#include "position.h"

#include <limits.h>

#define NC_EVAL_MATE_THRESHOLD 512
#define NC_EVAL_MIN (INT_MIN + 10)
#define NC_EVAL_MAX (INT_MAX - 10)

typedef int nc_eval;

nc_eval nc_eval_position(nc_position* p);
const char* nc_eval_tostr(nc_eval score);