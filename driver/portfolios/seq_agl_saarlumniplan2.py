SAS_FILE = "output.sas"
OPTIMAL = True

CONFIGS_STRIPS =  [ # TODO runtimes
    # TODO RHC-SC (hCFF)
    ((10,10), ['fast-downward-conjunctions',  'TODO']),
    # decoupled search: inverted-fork factorings
    ((0, 10), ['fast-downward-decoupled', "--decoupling",
           "ifork(search_type=sat, max_leaf_size=100000)",
           "--heuristic",
           "hff=ff(cost_type=one)",
           "--search",
           "lazy_greedy([hff], "
           "      cost_type=one, "
           "      preferred=[hff])"]),
    # decoupled search: fork factorings
    ((0, 10), ['fast-downward-decoupled', "--decoupling",
           "fork(search_type=sat, pruning=cost_frontier(irrelevance=TRANSITIONS), max_leaf_size=100000)",
           "--heuristic",
           "hff=ff(cost_type=one)",
           "--search",
           "lazy_greedy([hff], "
           "      cost_type=one, "
           "      preferred=[hff])"]),
    # decoupled search: general factorings
    ((0, 10), ['fast-downward-decoupled', "--decoupling",
           "lp_general(search_type=sat, factoring_time_limit=30, memory_limit=7500, add_cg_sccs=true, strategy=mm_approx, min_flexibility=0.8)",
           "--heuristic",
           "hff=ff(cost_type=one)",
           "--search",
           "lazy_greedy([hff],"
           "            preferred=[hff], cost_type=one)"]),
    # TODO GBFS-SCL (hCFF)
    ((10,10), ['fast-downward-conjunctions',  'TODO']),
]

CONFIGS_ADL = [
    # TODO GBFS-SCL (hFF)
    ((120,120), ['fast-downward-conjunctions',  'TODO']),
    # TODO YAHSP
    ((0, 90), ['fast-downward-conjunctions', "TODO"]),
    # TODO LAMA config
    ((180, 90), ['fast-downward-conjunctions',  'TODO']),
]

