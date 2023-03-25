SAS_FILE = "output.sas"
OPTIMAL = True

_HCFF_UNIT_COST_DEFINITIONS = [
    '--heuristic', 'hcff=cff(seed=-1, cache_estimates=false, cost_type=1)',
    '--heuristic', 'hn=novelty(cache_estimates=false)',
    '--heuristic', 'tmp=novelty_linker(hcff, [hn])'
]

CONFIGS_STRIPS =  [
    # GBFS-SCL (hCFF)
    (2, ['fast-downward-conjunctions'] + _HCFF_UNIT_COST_DEFINITIONS + [
       '--search', 'lazy_greedy_rsl(hcff, preferred=[hcff], conjunctions_heuristic=hcff, novelty=hn, cost_type=1, subgoal_aggregation_method=SUM, path_dependent_subgoals=false, lookahead_weight=infinity)'
    ]),
    # decoupled search: inverted-fork factorings
    (2, ['fast-downward-decoupled', "--decoupling",
           "ifork(search_type=sat, max_leaf_size=100000)",
           "--heuristic",
           "hff=ff(cost_type=one)",
           "--search",
           "lazy_greedy([hff], "
           "      cost_type=one, "
           "      preferred=[hff])"]),
    # decoupled search: fork factorings
    (2, ['fast-downward-decoupled', "--decoupling",
           "fork(search_type=sat, pruning=cost_frontier(irrelevance=TRANSITIONS), max_leaf_size=100000)",
           "--heuristic",
           "hff=ff(cost_type=one)",
           "--search",
           "lazy_greedy([hff], "
           "      cost_type=one, "
           "      preferred=[hff])"]),
    # RHC-SC (hCFF)
    (2, ['fast-downward-conjunctions'] + _HCFF_UNIT_COST_DEFINITIONS + [
       '--search', 'ehc_cnsg(hcff, novelty=hn, cost_type=1, always_reevaluate=true, subgoal_aggregation_method=SUM, path_dependent_subgoals=false, w=infinity, seed=-1, restart_in_dead_ends=true, learning_stagnation_threshold=1)'
    ]),
    # GBFS-SCL (hCFF)
    (90, ['fast-downward-conjunctions'] + _HCFF_UNIT_COST_DEFINITIONS + [
       '--search', 'lazy_greedy_rsl(hcff, preferred=[hcff], conjunctions_heuristic=hcff, novelty=hn, cost_type=1, subgoal_aggregation_method=COUNT, path_dependent_subgoals=true, lookahead_weight=1)'
    ]),
    # decoupled search: general factorings
    (120, ['fast-downward-decoupled', "--decoupling",
           "lp_general(search_type=sat, factoring_time_limit=30, memory_limit=7500, add_cg_sccs=true, strategy=mm_approx, min_flexibility=0.8)",
           "--heuristic",
           "hff=ff(cost_type=one)",
           "--search",
           "lazy_greedy([hff],"
           "            preferred=[hff], cost_type=one)"]),
    # RHC-SC (hCFF)
    (82, ['fast-downward-conjunctions'] + _HCFF_UNIT_COST_DEFINITIONS + [
       '--search', 'ehc_cnsg(hcff, novelty=hn, cost_type=1, always_reevaluate=true, subgoal_aggregation_method=COUNT, path_dependent_subgoals=true, w=1, seed=-1, restart_in_dead_ends=true, learning_stagnation_threshold=1)'
    ]),
]

_SHORT_TIMEOUT = 2
_GBFS_SCL_TIMEOUT = 120
_YAHSP_TIMEOUT = 90

CONFIGS_ADL = [(1, [
    'fast-downward-conjunctions',
    '--heuristic', 'hff=ff(cache_estimates=false, cost_type=1)',
    '--heuristic', 'hlm=lmcount(lm_rhw(reasonable_orders=true), cost_type=1)',
    '--search', 'ipc18_iterated([{}, {}, {}, {}, {}, {}], continue_on_solve=false, continue_on_fail=true)'.format(
        # GBFS-SCL (fast)
        f'lazy_greedy_rsl_rainbow(hff, preferred=[hff], relaxed_plan_heuristic=hff, cost_type=1, subgoal_aggregation_method=SUM, path_dependent_subgoals=false, lookahead_weight=infinity, max_time={_SHORT_TIMEOUT})',
        # YAHSP (fast)
        f'lazy_greedy_yahsp_rainbow(hff, preferred=hff, relaxed_plan_heuristic=hff, cost_type=1, max_time={_SHORT_TIMEOUT})',
        # LAMA-first (fast)
        f'lazy_greedy([hff, hlm], preferred=[hff], cost_type=1, max_time={_SHORT_TIMEOUT})',
        # GBFS-SCL
        f'lazy_greedy_rsl_rainbow(hff, preferred=[hff], relaxed_plan_heuristic=hff, cost_type=1, subgoal_aggregation_method=COUNT, path_dependent_subgoals=true, lookahead_weight=1, max_time={_GBFS_SCL_TIMEOUT})',
        # YAHSP
        f'lazy_greedy_yahsp_rainbow(hff, preferred=hff, relaxed_plan_heuristic=hff, cost_type=1, max_time={_YAHSP_TIMEOUT})',
        # LAMA-first
        'lazy_greedy([hff, hlm], preferred=[hff], cost_type=1)'
    )
])]
