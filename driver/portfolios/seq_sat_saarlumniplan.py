SAS_FILE = "output.sas"
OPTIMAL = False

CONFIGS_STRIPS =  [
    # decstar:ds-sss-oss
    (79, ['fast-downward-decoupled', '--decoupling', 'lp_general(factoring_time_limit=30, memory_limit=7000, add_cg_sccs=true, strategy=mm_approx, min_flexibility=0.8)', '--search', 'lazy_greedy(ff, bound=BOUND)']),
    # scorpion:sys-scp-interesting_non_negative-60s-cartesian-single
    (898, ['fast-downward-conjunctions',  '--heuristic', 'hlm_normalcost=lmcount(lm_rhw(reasonable_orders=true))',
    '--heuristic', 'hff_normalcost=ff(cache_estimates=false, cost_type=PLUSONE)',
    '--search', 'ipc18_iterated([{}])'.format(
        'lazy_iterated_weights_c([hff_normalcost, hlm_normalcost], preferred=[hff_normalcost], cached_heuristic=hff_normalcost, bound=BOUND)'
    )]),
]

CONFIGS_COND_EFFS = [
    # symk:sym-bd
    (1251, ['symk', '--search', 'sym-bd(silent=true)']),
    # scorpion:astar-blind
    (5, ['scorpion', '--search', 'astar(blind())']),
    # scorpion:sys-scp-interesting_general-60s
    (262, ['scorpion', '--search', 'astar(scp_online([projections(sys_scp(max_time=60, max_time_per_restart=6, max_pdb_size=2M, max_collection_size=20M, pattern_type=interesting_general), create_complete_transition_system=true)], saturator=perimstar, max_time=60, max_size=1M, interval=10K, orders=greedy_orders()))']),
]

CONFIGS_AXIOMS = [
    # symk:sym-bd
    (1513, ['symk', '--search', 'sym-bd(silent=true)']),
    # scorpion:astar-blind
    (1, ['scorpion', '--search', 'astar(blind())']),
]



def get_pddl_features(task):
    has_axioms = False
    has_conditional_effects = False
    with open(task) as f:
        in_op = False
        for line in f:
            line = line.strip()
            if line == "begin_rule":
                has_axioms = True

            if line == "begin_operator":
                in_op = True
            elif line == "end_operator":
                in_op = False
            elif in_op:
                parts = line.split()
                if len(parts) >= 6 and all(p.lstrip('-').isdigit() for p in parts):
                    print(f"Task has at least one conditional effect: {line}")
                    has_conditional_effects = True
    return has_axioms, has_conditional_effects


HAS_AXIOMS, HAS_CONDITIONAL_EFFECTS = get_pddl_features(SAS_FILE)

print(f"Task has axioms: {HAS_AXIOMS}")
print(f"Task has conditional effects: {HAS_CONDITIONAL_EFFECTS}")

if HAS_AXIOMS:
    CONFIGS = CONFIGS_AXIOMS
elif HAS_CONDITIONAL_EFFECTS:
    CONFIGS = CONFIGS_COND_EFFS
else:
    CONFIGS = CONFIGS_STRIPS
