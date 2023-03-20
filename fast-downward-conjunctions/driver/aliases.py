import os

from .util import DRIVER_DIR


PORTFOLIO_DIR = os.path.join(DRIVER_DIR, "portfolios")

ALIASES = {}


_HCFF_UNIT_COST_DEFINITIONS = [
    '--heuristic', 'hcff=cff(seed=-1, cache_estimates=false, cost_type=1)',
    '--heuristic', 'hn=novelty(cache_estimates=false)',
    '--heuristic', 'tmp=novelty_linker(hcff, [hn])'
]

ALIASES['RHC'] = _HCFF_UNIT_COST_DEFINITIONS + [
    '--search', 'ehc_cn(hcff, preferred=hcff, novelty=hn, cost_type=1, seed=-1, w=infinity, search_space_exhaustion=RESTART, restart_in_dead_ends=true, learning_stagnation_threshold=1)'
]

ALIASES['RHC-SC'] = _HCFF_UNIT_COST_DEFINITIONS + [
    '--search', 'ehc_cnsg(hcff, novelty=hn, cost_type=1, always_reevaluate=true, subgoal_aggregation_method=COUNT, path_dependent_subgoals=true, w=1, seed=-1, restart_in_dead_ends=true, learning_stagnation_threshold=1)'
]

ALIASES['GBFS-SCL'] = _HCFF_UNIT_COST_DEFINITIONS + [
    '--search', 'lazy_greedy_rsl(hcff, preferred=[hcff], conjunctions_heuristic=hcff, novelty=hn, cost_type=1, subgoal_aggregation_method=COUNT, path_dependent_subgoals=true, lookahead_weight=1)'
]

ALIASES['LAMA-hCFF'] =  [
    '--heuristic', 'hlm_normalcost=lmcount(lm_rhw(reasonable_orders=true))',
    '--heuristic', 'hcff_normalcost=cff(seed=42, cache_estimates=false, cost_type=PLUSONE)',
    '--heuristic', 'hn_normalcost=novelty(cache_estimates=false)',
    '--heuristic', 'tmp_normalcost=novelty_linker(hcff_normalcost, [hn_normalcost])',
    '--search', 'ipc18_iterated([{}])'.format(
        'lazy_iterated_weights_c([hcff_normalcost, hlm_normalcost], preferred=[hcff_normalcost], conjunctions_heuristic=hcff_normalcost, strategy=maintain_fixed_size_probabilistic(generate_initially=true, initial_removal_mode=UNTIL_BOUND, base_probability=0.02, target_growth_ratio=1.50))'
    )
]

_HFF_UNIT_COST_DEFINITIONS = [
    '--heuristic', 'hff=ff(cache_estimates=false, cost_type=1)'
]

ALIASES['Discount-GBFS-SCL'] = _HFF_UNIT_COST_DEFINITIONS + [
    '--search', 'lazy_greedy_rsl_rainbow(hff, preferred=[hff], relaxed_plan_heuristic=hff, cost_type=1, subgoal_aggregation_method=COUNT, path_dependent_subgoals=true, lookahead_weight=1)'
]

ALIASES['YAHSP'] = _HFF_UNIT_COST_DEFINITIONS + [
    '--search', 'lazy_greedy_yahsp_rainbow(hff, preferred=hff, relaxed_plan_heuristic=hff, cost_type=1)'
]

ALIASES['LAMA-hFF'] =  [
    '--heuristic', 'hlm_normalcost=lmcount(lm_rhw(reasonable_orders=true))',
    '--heuristic', 'hff_normalcost=ff(cache_estimates=false, cost_type=PLUSONE)',
    '--search', 'ipc18_iterated([{}])'.format(
        'lazy_iterated_weights_c([hff_normalcost, hlm_normalcost], preferred=[hff_normalcost], cached_heuristic=hff_normalcost)'
    )
]

ALIASES['GBFS-SCL-SAT'] = _HCFF_UNIT_COST_DEFINITIONS + [
    '--heuristic', 'hlm_normalcost=lmcount(lm_rhw(reasonable_orders=true))',
    '--if-unit-cost',
    '--search', 'ipc18_iterated([{}, {}], delete_after_phase_heuristics=[hn], delete_after_phase_phases=[0])'.format(
        'lazy_greedy_rsl(hcff, preferred=[hcff], conjunctions_heuristic=hcff, novelty=hn, subgoal_aggregation_method=COUNT, path_dependent_subgoals=true, lookahead_weight=1)',
        'lazy_iterated_weights_c([hcff, hlm_normalcost], preferred=[hcff], conjunctions_heuristic=hcff, strategy=maintain_fixed_size_probabilistic(generate_initially=true, initial_removal_mode=UNTIL_BOUND, base_probability=0.02, target_growth_ratio=1.50))'
    ),
    '--if-non-unit-cost',
    '--heuristic', 'hcff_normalcost=cff(seed=42, cache_estimates=false, cost_type=PLUSONE)',
    '--heuristic', 'hn_normalcost=novelty(cache_estimates=false)',
    '--heuristic', 'tmp_normalcost=novelty_linker(hcff_normalcost, [hn_normalcost])',
    '--search', 'ipc18_iterated([{}, {}, {}], delete_after_phase_heuristics=[hcff, hn, tmp, hn_normalcost], delete_after_phase_phases=[0, 0, 0, 1])'.format(
        'lazy_greedy_rsl(hcff, preferred=[hcff], conjunctions_heuristic=hcff, novelty=hn, cost_type=1, subgoal_aggregation_method=COUNT, path_dependent_subgoals=true, lookahead_weight=1)',
        'lazy_greedy_rsl(hcff_normalcost, preferred=[hcff_normalcost], conjunctions_heuristic=hcff_normalcost, novelty=hn_normalcost, subgoal_aggregation_method=COUNT, path_dependent_subgoals=true, lookahead_weight=1)',
        'lazy_iterated_weights_c([hcff_normalcost, hlm_normalcost], preferred=[hcff_normalcost], conjunctions_heuristic=hcff_normalcost, strategy=maintain_fixed_size_probabilistic(generate_initially=true, initial_removal_mode=UNTIL_BOUND, base_probability=0.02, target_growth_ratio=1.50))'
    ),
    '--always'
]

ALIASES["seq-sat-fd-autotune-1"] = [
    "--heuristic", "hff=ff(transform=adapt_costs(one))",
    "--heuristic", "hcea=cea()",
    "--heuristic", "hcg=cg(transform=adapt_costs(plusone))",
    "--heuristic", "hgc=goalcount()",
    "--heuristic", "hAdd=add()",
    "--search", """iterated([
lazy(alt([single(sum([g(),weight(hff,10)])),
          single(sum([g(),weight(hff,10)]),pref_only=true)],
         boost=2000),
     preferred=[hff],reopen_closed=false,cost_type=one),
lazy(alt([single(sum([g(),weight(hAdd,7)])),
          single(sum([g(),weight(hAdd,7)]),pref_only=true),
          single(sum([g(),weight(hcg,7)])),
          single(sum([g(),weight(hcg,7)]),pref_only=true),
          single(sum([g(),weight(hcea,7)])),
          single(sum([g(),weight(hcea,7)]),pref_only=true),
          single(sum([g(),weight(hgc,7)])),
          single(sum([g(),weight(hgc,7)]),pref_only=true)],
         boost=1000),
     preferred=[hcea,hgc],reopen_closed=false,cost_type=one),
lazy(alt([tiebreaking([sum([g(),weight(hAdd,3)]),hAdd]),
          tiebreaking([sum([g(),weight(hAdd,3)]),hAdd],pref_only=true),
          tiebreaking([sum([g(),weight(hcg,3)]),hcg]),
          tiebreaking([sum([g(),weight(hcg,3)]),hcg],pref_only=true),
          tiebreaking([sum([g(),weight(hcea,3)]),hcea]),
          tiebreaking([sum([g(),weight(hcea,3)]),hcea],pref_only=true),
          tiebreaking([sum([g(),weight(hgc,3)]),hgc]),
          tiebreaking([sum([g(),weight(hgc,3)]),hgc],pref_only=true)],
         boost=5000),
     preferred=[hcea,hgc],reopen_closed=false,cost_type=normal),
eager(alt([tiebreaking([sum([g(),weight(hAdd,10)]),hAdd]),
           tiebreaking([sum([g(),weight(hAdd,10)]),hAdd],pref_only=true),
           tiebreaking([sum([g(),weight(hcg,10)]),hcg]),
           tiebreaking([sum([g(),weight(hcg,10)]),hcg],pref_only=true),
           tiebreaking([sum([g(),weight(hcea,10)]),hcea]),
           tiebreaking([sum([g(),weight(hcea,10)]),hcea],pref_only=true),
           tiebreaking([sum([g(),weight(hgc,10)]),hgc]),
           tiebreaking([sum([g(),weight(hgc,10)]),hgc],pref_only=true)],
          boost=500),
      preferred=[hcea,hgc],reopen_closed=true,cost_type=normal)
],repeat_last=true,continue_on_fail=true)"""]

ALIASES["seq-sat-fd-autotune-2"] = [
    "--heuristic", "hcea=cea(transform=adapt_costs(plusone))",
    "--heuristic", "hcg=cg(transform=adapt_costs(one))",
    "--heuristic", "hgc=goalcount(transform=adapt_costs(plusone))",
    "--heuristic", "hff=ff()",
    "--search", """iterated([
ehc(hcea,preferred=[hcea],preferred_usage=0,cost_type=normal),
lazy(alt([single(sum([weight(g(),2),weight(hff,3)])),
          single(sum([weight(g(),2),weight(hff,3)]),pref_only=true),
          single(sum([weight(g(),2),weight(hcg,3)])),
          single(sum([weight(g(),2),weight(hcg,3)]),pref_only=true),
          single(sum([weight(g(),2),weight(hcea,3)])),
          single(sum([weight(g(),2),weight(hcea,3)]),pref_only=true),
          single(sum([weight(g(),2),weight(hgc,3)])),
          single(sum([weight(g(),2),weight(hgc,3)]),pref_only=true)],
         boost=200),
     preferred=[hcea,hgc],reopen_closed=false,cost_type=one),
lazy(alt([single(sum([g(),weight(hff,5)])),
          single(sum([g(),weight(hff,5)]),pref_only=true),
          single(sum([g(),weight(hcg,5)])),
          single(sum([g(),weight(hcg,5)]),pref_only=true),
          single(sum([g(),weight(hcea,5)])),
          single(sum([g(),weight(hcea,5)]),pref_only=true),
          single(sum([g(),weight(hgc,5)])),
          single(sum([g(),weight(hgc,5)]),pref_only=true)],
         boost=5000),
     preferred=[hcea,hgc],reopen_closed=true,cost_type=normal),
lazy(alt([single(sum([g(),weight(hff,2)])),
          single(sum([g(),weight(hff,2)]),pref_only=true),
          single(sum([g(),weight(hcg,2)])),
          single(sum([g(),weight(hcg,2)]),pref_only=true),
          single(sum([g(),weight(hcea,2)])),
          single(sum([g(),weight(hcea,2)]),pref_only=true),
          single(sum([g(),weight(hgc,2)])),
          single(sum([g(),weight(hgc,2)]),pref_only=true)],
         boost=1000),
     preferred=[hcea,hgc],reopen_closed=true,cost_type=one)
],repeat_last=true,continue_on_fail=true)"""]

def _get_lama(**kwargs):
    return [
        "--if-unit-cost",
        "--heuristic",
        "hlm=lmcount(lm_rhw(reasonable_orders=true),pref={pref})".format(**kwargs),
        "--heuristic", "hff=ff()",
        "--search", """iterated([
                         lazy_greedy([hff,hlm],preferred=[hff,hlm]),
                         lazy_wastar([hff,hlm],preferred=[hff,hlm],w=5),
                         lazy_wastar([hff,hlm],preferred=[hff,hlm],w=3),
                         lazy_wastar([hff,hlm],preferred=[hff,hlm],w=2),
                         lazy_wastar([hff,hlm],preferred=[hff,hlm],w=1)
                         ],repeat_last=true,continue_on_fail=true)""",
        "--if-non-unit-cost",
        "--heuristic",
        "hlm1=lmcount(lm_rhw(reasonable_orders=true),transform=adapt_costs(one),pref={pref})".format(**kwargs),
        "--heuristic", "hff1=ff(transform=adapt_costs(one))",
        "--heuristic",
        "hlm2=lmcount(lm_rhw(reasonable_orders=true),transform=adapt_costs(plusone),pref={pref})".format(**kwargs),
        "--heuristic", "hff2=ff(transform=adapt_costs(plusone))",
        "--search", """iterated([
                         lazy_greedy([hff1,hlm1],preferred=[hff1,hlm1],
                                     cost_type=one,reopen_closed=false),
                         lazy_greedy([hff2,hlm2],preferred=[hff2,hlm2],
                                     reopen_closed=false),
                         lazy_wastar([hff2,hlm2],preferred=[hff2,hlm2],w=5),
                         lazy_wastar([hff2,hlm2],preferred=[hff2,hlm2],w=3),
                         lazy_wastar([hff2,hlm2],preferred=[hff2,hlm2],w=2),
                         lazy_wastar([hff2,hlm2],preferred=[hff2,hlm2],w=1)
                         ],repeat_last=true,continue_on_fail=true)""",
        # Append --always to be on the safe side if we want to append
        # additional options later.
        "--always"]

ALIASES["seq-sat-lama-2011"] = _get_lama(pref="true")
ALIASES["lama"] = _get_lama(pref="false")

ALIASES["lama-first"] = [
    "--heuristic",
    "hlm=lmcount(lm_factory=lm_rhw(reasonable_orders=true),transform=adapt_costs(one),pref=false)",
    "--heuristic", "hff=ff(transform=adapt_costs(one))",
    "--search", """lazy_greedy([hff,hlm],preferred=[hff,hlm],
                               cost_type=one,reopen_closed=false)"""]

ALIASES["seq-opt-bjolp"] = [
    "--heuristic",
    "lmc=lmcount(lm_merged([lm_rhw(),lm_hm(m=1)]),admissible=true)",
    "--search",
    "astar(lmc,lazy_evaluator=lmc)"]

ALIASES["seq-opt-lmcut"] = [
    "--search", "astar(lmcut())"]


PORTFOLIOS = {}
for portfolio in os.listdir(PORTFOLIO_DIR):
    name, ext = os.path.splitext(portfolio)
    assert ext == ".py", portfolio
    PORTFOLIOS[name.replace("_", "-")] = os.path.join(PORTFOLIO_DIR, portfolio)


def show_aliases():
    for alias in sorted(list(ALIASES) + list(PORTFOLIOS)):
        print(alias)


def set_options_for_alias(alias_name, args):
    """
    If alias_name is an alias for a configuration, set args.search_options
    to the corresponding command-line arguments. If it is an alias for a
    portfolio, set args.portfolio to the path to the portfolio file.
    Otherwise raise KeyError.
    """
    assert not args.search_options
    assert not args.portfolio

    if alias_name in ALIASES:
        args.search_options = [x.replace(" ", "").replace("\n", "")
                               for x in ALIASES[alias_name]]
    elif alias_name in PORTFOLIOS:
        args.portfolio = PORTFOLIOS[alias_name]
    else:
        raise KeyError(alias_name)
