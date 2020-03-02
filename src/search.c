#include "search.h"
#include "tt.h"

static nc_eval _nc_search_q(nc_position* p, nc_eval alpha, nc_eval beta, nc_timepoint max_time);
static nc_eval _nc_search_pv(nc_position* p, int depth, nc_eval alpha, nc_eval beta, nc_movelist* pv_out, nc_timepoint max_time);

static int _nc_search_nodes;
static int _nc_search_leaves;
static nc_timepoint _nc_search_start;
static nc_timepoint _nc_search_end;

nc_eval nc_search(nc_position* root, int depth, nc_movelist* pv_out, nc_timepoint max_time) {
    _nc_search_nodes = 0;
    _nc_search_leaves = 0;

    _nc_search_start = nc_timer_current();
    nc_eval ret = _nc_search_pv(root, depth, NC_EVAL_MIN, NC_EVAL_MAX, pv_out, max_time);
    _nc_search_end = nc_timer_current();

    return ret;
}

nc_eval _nc_search_q(nc_position* p, nc_eval alpha, nc_eval beta, nc_timepoint max_time) {
    ++_nc_search_nodes;

    if (nc_position_is_quiet(p) || (max_time && (nc_timer_current() >= max_time))) {
        return nc_position_get_score(p);
    }

    nc_movelist next_moves;
    nc_movelist_clear(&next_moves);
    nc_position_legal_moves(p, &next_moves);

    nc_eval best_score = NC_EVAL_MIN;

    if (!next_moves.len) {
        /* Color to move is in checkmate or stalemate. Do a check test quick. */
        if (p->states[p->ply].check) {
            /* Checkmate! */
            return NC_EVAL_MIN;
        } else {
            /* Stalemate! */
            return NC_SEARCH_CONTEMPT;
        }
    }

    for (int i = 0; i < next_moves.len; ++i) {
        nc_move cur = next_moves.moves[i];

        nc_position_make_move(p, cur);
        nc_eval score = -_nc_search_q(p, -beta, -alpha, max_time);
        nc_position_unmake_move(p, cur);

        if (score > best_score) best_score = score;
        if (score > alpha) alpha = score;
        if (alpha >= beta) break;
    }

    return nc_eval_parent(best_score);
}

nc_eval _nc_search_pv(nc_position* p, int depth, nc_eval alpha, nc_eval beta, nc_movelist* pv_out, nc_timepoint max_time) {
    nc_move best_move = NC_MOVE_NULL;
    nc_eval best_score = NC_EVAL_MIN;

    ++_nc_search_nodes;

    nc_movelist best_pv, current_pv;
    nc_movelist_clear(pv_out);
    nc_movelist_clear(&best_pv);

    if (nc_position_is_repetition(p)) {
        return NC_SEARCH_CONTEMPT;
    }

    if (depth <= 0 || (max_time && nc_timer_current() >= max_time)) {
        return _nc_search_q(p, alpha, beta, max_time);
    }

    nc_eval alpha_orig = alpha;

    nc_ttentry* tt = nc_tt_lookup(p->key);
    nc_move pv_move = NC_MOVE_NULL;

    if (tt->key == p->key && tt->depth >= depth) {
        if (tt->type == NC_TT_EXACT) {
            /* Walk to the end of the pv. */
            nc_position_make_move(p, tt->bestmove);

            _nc_search_pv(p, depth - 1, -beta, -alpha, &best_pv, max_time);
            nc_movelist_push(pv_out, tt->bestmove);
            nc_movelist_concat(pv_out, &best_pv);

            nc_position_unmake_move(p, tt->bestmove);

            return tt->score;
        } else if (tt->type == NC_TT_LOWERBOUND) {
            if (tt->score > alpha) alpha = tt->score;
        } else if (tt->type == NC_TT_UPPERBOUND) {
            if (tt->score < beta) beta = tt->score;
        }
    }

    /* Grab PV move from TT entry */
    if (tt->key == p->key && tt->type == NC_TT_EXACT) {
        pv_move = tt->bestmove;
    }

    nc_movelist next_moves;
    nc_movelist_clear(&next_moves);
    nc_position_legal_moves(p, &next_moves);

    if (!next_moves.len) {
        /* Color to move is in checkmate or stalemate. Do a check test quick. */
        if (p->states[p->ply].check) {
            /* Checkmate! */
            return NC_EVAL_MIN;
        } else {
            /* Stalemate! */
            return NC_SEARCH_CONTEMPT;
        }
    }

    /* Perform move ordering */

    /* First, score each move for ordering. */
    nc_eval move_scores[NC_MOVELIST_LEN];
    for (int i = 0; i < next_moves.len; ++i) {
        if (next_moves.moves[i] == pv_move) move_scores[i] = NC_EVAL_MAX;

        nc_position_make_move(p, next_moves.moves[i]);
        move_scores[i] = -nc_position_get_score(p);
        nc_position_unmake_move(p, next_moves.moves[i]);
    }

    /* Perform a decreasing selection sort  */
    for (int i = 0; i < next_moves.len - 1; ++i) {
        nc_eval best = move_scores[i];
        int swap_with = i;

        for (int j = i + 1; j < next_moves.len; ++j) {
            if (move_scores[i] > best) {
                best = move_scores[i];
                swap_with = j;
            }
        }

        if (i != swap_with) {
            nc_move tmp = next_moves.moves[i];
            next_moves.moves[i] = next_moves.moves[swap_with];
            next_moves.moves[swap_with] = tmp;
        }
    }

    /* Walk through the ordered moves */
    for (int i = 0; i < next_moves.len; ++i) {
        nc_move cur = next_moves.moves[i];

        /* Check search extension */
        int next_depth = (cur & NC_CHECK) ? depth : depth - 1;

        /* Grab current eval */
        nc_eval current_heuristic = nc_position_get_score(p);

        nc_position_make_move(p, cur);

        nc_eval score;

        int asp_alpha = current_heuristic - NC_SEARCH_ASPIRATION;
        int asp_beta = current_heuristic + NC_SEARCH_ASPIRATION;

        /* Perform aspiration search */

        int failed;
        int reset_alpha = 0, reset_beta = 0;
        do {
            failed = 0;
            score = -_nc_search_pv(p, next_depth, -asp_beta, -asp_alpha, &current_pv, max_time);

            /* If fails, release aspiration bounds on direction of failure */
            if (score < asp_alpha && !reset_alpha) {
                asp_alpha = alpha;
                reset_alpha = 1;
                failed = 1;
            }

            if (score > asp_beta && !reset_beta) {
                asp_beta = beta;
                failed = 1;
                reset_beta = 1;
            }
        } while (failed);

        nc_position_unmake_move(p, cur);

        if (score > best_score) {
            best_move = cur;
            best_score = score;
            memcpy(&best_pv, &current_pv, sizeof current_pv);
        }

        if (score >= alpha) {
            alpha = score;
        }

        if (alpha >= beta) break;
    }

    /* Store search result back into ttable */
    tt->score = best_score;
    tt->bestmove = best_move;
    tt->depth = depth;
    tt->key = p->key;

    if (best_score <= alpha_orig) {
        tt->type = NC_TT_UPPERBOUND;
    } else if (best_score >= beta) {
        tt->type = NC_TT_LOWERBOUND;
    } else {
        tt->type = NC_TT_EXACT;
    }

    /* Generate output pv */
    nc_movelist_push(pv_out, best_move);
    nc_movelist_concat(pv_out, &best_pv);

    return nc_eval_parent(alpha);
}

int nc_search_get_nodes() {
    return _nc_search_nodes;
}

int nc_search_get_nps() {
    float seconds = nc_timer_elapsed_s(_nc_search_start);
    return (int) ((float) _nc_search_nodes / (seconds + 0.000001f));
}

int nc_search_get_time() {
    return (_nc_search_end - _nc_search_start) / (CLOCKS_PER_SEC / 1000);
}
