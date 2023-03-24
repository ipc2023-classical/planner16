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
    # TODO GBFS-SCL (hCFF)
    ((180, 1000), ['fast-downward-conjunctions',  'TODO']),
    # decoupled search: general factorings
    ((0, 600), ['fast-downward-decoupled', "--decoupling", # => skipped if solution has been found before
           "lp_general(search_type=sat, factoring_time_limit=30, memory_limit=7500, add_cg_sccs=true, strategy=mm_approx, min_flexibility=0.8)",
           "--heuristic",
           "hff=ff(cost_type=one)",
           "--search",
           "lazy_greedy([hff],"
           "            preferred=[hff], bound=BOUND, cost_type=one)"]),
    # TODO LAMA-hCFF
    ((1570, 100), ['fast-downward-conjunctions',  'TODO']),
]

CONFIGS_ADL = [
    # TODO GBFS-SCL (FF)
    ((720,720), ['fast-downward-conjunctions',  'TODO']),
    # TODO YAHSP
    ((0, 540), ['fast-downward-conjunctions', "TODO"]),
    # TODO LAMA config
    ((1080, 540), ['fast-downward-conjunctions',  'TODO']),
]


