SAS_FILE = "output.sas"
OPTIMAL = False

CONFIGS_STRIPS =  [
    # decoupled search: inverted-fork factorings
    ((50, 50), ['fast-downward-decoupled', "--decoupling",
           "ifork(search_type=sat, max_leaf_size=100000)",
           "--heuristic",
           "hff=ff(cost_type=one)",
           "--search",
           "lazy_greedy([hff], "
           "      cost_type=one, bound=BOUND,"
           "      preferred=[hff])"]),
    # decoupled search: fork factorings
    ((0, 50), ['fast-downward-decoupled', "--decoupling",
           "fork(search_type=sat, pruning=cost_frontier(irrelevance=TRANSITIONS), max_leaf_size=100000)",
           "--heuristic",
           "hff=ff(cost_type=one)",
           "--search",
           "lazy_greedy([hff], "
           "      cost_type=one, bound=BOUND,"
           "      preferred=[hff])"]),
    # GBFS-SCL (hCFF)
    ((180, 1000), [
        'fast-downward-conjunctions',
        '--heuristic', 'hcff=cff(seed=-1, cache_estimates=false, cost_type=1)',
        '--heuristic', 'hn=novelty(cache_estimates=false)',
        '--heuristic', 'tmp=novelty_linker(hcff, [hn])'
        '--search', 'lazy_greedy_rsl(hcff, preferred=[hcff], conjunctions_heuristic=hcff, novelty=hn, cost_type=1, subgoal_aggregation_method=COUNT, path_dependent_subgoals=true, lookahead_weight=1, bound=BOUND)'
    ]),
    # decoupled search: general factorings
    ((0, 600), ['fast-downward-decoupled', "--decoupling", # => skipped if solution has been found before
           "lp_general(search_type=sat, factoring_time_limit=30, memory_limit=7500, add_cg_sccs=true, strategy=mm_approx, min_flexibility=0.8)",
           "--heuristic",
           "hff=ff(cost_type=one)",
           "--search",
           "lazy_greedy([hff],"
           "            preferred=[hff], bound=BOUND, cost_type=one)"]),
    # LAMA-hCFF
    ((1570, 100), [
        'fast-downward-conjunctions',
        '--heuristic', 'hlm_normalcost=lmcount(lm_rhw(reasonable_orders=true))',
        '--heuristic', 'hcff_normalcost=cff(seed=42, cache_estimates=false, cost_type=PLUSONE)',
        '--search', 'ipc18_iterated([{}])'.format(
            'lazy_iterated_weights_c([hcff_normalcost, hlm_normalcost], preferred=[hcff_normalcost], conjunctions_heuristic=hcff_normalcost, strategy=maintain_fixed_size_probabilistic(generate_initially=true, base_probability=0.02, target_growth_ratio=1.50), bound=BOUND)'
        )
    ]),
]

_GBFS_SCL_TIMEOUT = 720
_YAHSP_TIMEOUT = 480

CONFIGS_ADL = [(1, [
    'fast-downward-conjunctions',
    '--heuristic', 'hff=ff(cache_estimates=false, cost_type=1)'
    '--heuristic', 'hlm_normalcost=lmcount(lm_rhw(reasonable_orders=true))',
    '--heuristic', 'hff_normalcost=ff(cache_estimates=false, cost_type=PLUSONE)',
    '--search', 'ipc18_iterated([{}, {}, {}], delete_after_phase_heuristics=[hff], delete_after_phase_phases=[1], skip_if_solved=[1], continue_on_fail=true, bound=BOUND)'.format(
        # GBFS-SCL
        f'lazy_greedy_rsl_rainbow(hff, preferred=[hff], relaxed_plan_heuristic=hff, cost_type=1, subgoal_aggregation_method=COUNT, path_dependent_subgoals=true, lookahead_weight=1, max_time={_GBFS_SCL_TIMEOUT})',
        # YAHSP
        f'lazy_greedy_yahsp_rainbow(hff, preferred=hff, relaxed_plan_heuristic=hff, cost_type=1, max_time={_YAHSP_TIMEOUT})',
        # LAMA
        'lazy_iterated_weights_c([hff_normalcost, hlm_normalcost], preferred=[hff_normalcost], cached_heuristic=hff_normalcost)'
    )
])]
