#include "searchctl.h"
#include "util.h"
#include "timer.h"
#include "search.h"

#include <pthread.h>
#include <errno.h>

static pthread_t _nc_searchthread;
static nc_position _nc_search_root;
static int _nc_search_active;
static int _nc_search_abort;

void* nc_searchctl_worker(void* opts);

void nc_searchctl_init() {
	nc_debug("Initialized searchctl.");
}

void nc_searchctl_position(nc_position p) {
	if (_nc_search_active) {
		nc_searchctl_stop();
	}

	_nc_search_root = p;
	nc_debug("Loaded searchctl root.");
}

void nc_searchctl_go(nc_searchopts* opts) {
	if (_nc_search_active) {
		nc_searchctl_stop();
	}

	nc_debug("Starting search.");

	if (pthread_create(&_nc_searchthread, NULL, nc_searchctl_worker, (void*) opts)) {
		nc_error("Failed to start search thread: %s", strerror(errno));
	}
}

void nc_searchctl_stop() {
	if (!_nc_search_active) {
		nc_error("Search stop called when no search active.");
		return;
	}

	nc_debug("Stopping search thread.");

	_nc_search_abort = 1;

	pthread_join(_nc_searchthread, NULL);
	nc_debug("Joined search thread.");

	_nc_search_abort = 0;
	_nc_search_active = 0;
}

void* nc_searchctl_worker(void* a) {
	nc_searchopts* opts = (nc_searchopts*) a;
	nc_debug("In searchctl worker.");
	nc_debug("opts: wtime %d btime %d winc %d binc %d movestogo %d depth %d nodes %d mate %d movetime %d infinite %d",
		opts->wtime, opts->btime, opts->winc, opts->binc, opts->movestogo, opts->depth, opts->nodes, opts->mate, opts->movetime, opts->infinite);

	nc_move bestmove = NC_MOVE_NULL;

	/* Start iterative deepening search. */
	for (int cur_depth = 1;; ++cur_depth) {
		/* Compute a maximum time for the search if necessary. */
		nc_timepoint max_time = 0;

		if (cur_depth > 1) {
			if (opts->movetime) {
				max_time = nc_timer_futurems(opts->movetime);
			} else if (!opts->depth && !opts->infinite) {
				int ourtime = opts->wtime;
				int ourinc = opts->winc;

				if (_nc_search_root.color_to_move == NC_BLACK) {
					ourtime = opts->btime;
					ourinc = opts->binc;
				}

				/* !!!! TODO: actual time management !!!! */
				max_time = nc_timer_futurems(ourtime + (ourinc * ourinc * ourinc) / 4); // what in tarnation
			}
		}

		nc_movelist pv_line;

		/* Try a search at cur_depth. */
		nc_eval search_score = nc_search(&_nc_search_root, cur_depth, &pv_line, max_time);

		if (nc_search_incomplete()) {
			break;
		}

		/* Write search result info. */
		printf("info depth %d time %d nodes %d nps %d score %s pv",
			cur_depth,
			nc_search_get_time(),
			nc_search_get_nodes(),
			nc_search_get_nps(),
			nc_eval_tostr(search_score)
		);

		for (int i = 0; i < pv_line.len; ++i) {
			printf(" %s", nc_move_tostr(pv_line.moves[i]));
		}

		printf("\n");

		bestmove = pv_line.moves[0];

		if (cur_depth == opts->depth || _nc_search_abort) {
			break;
		}
	}

	/* Finally, write the bestmove for uci. */
	printf("bestmove %s\n", nc_move_tostr(bestmove));

	return NULL;
}