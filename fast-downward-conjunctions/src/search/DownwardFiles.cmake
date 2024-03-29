# See http://www.fast-downward.org/ForDevelopers/AddingSourceFiles
# for general information on adding source files and CMake plugins.
#
# All plugins are enabled by default and users can disable them by specifying
#    -DPLUGIN_FOO_ENABLED=FALSE
# The default behavior can be changed so all non-essential plugins are
# disabled by default by specifying
#    -DDISABLE_PLUGINS_BY_DEFAULT=TRUE
# In that case, individual plugins can be enabled with
#    -DPLUGIN_FOO_ENABLED=TRUE
#
# Defining a new plugin:
#    fast_downward_plugin(
#        NAME <NAME>
#        [ DISPLAY_NAME <DISPLAY_NAME> ]
#        [ HELP <HELP> ]
#        SOURCES
#            <FILE_1> [ <FILE_2> ... ]
#        [ DEPENDS <PLUGIN_NAME_1> [ <PLUGIN_NAME_2> ... ] ]
#        [ DEPENDENCY_ONLY ]
#        [ CORE_PLUGIN ]
#    )
#
# <DISPLAY_NAME> defaults to lower case <NAME> and is used to group files
#   in IDEs and for messages.
# <HELP> defaults to <DISPLAY_NAME> and is used to describe the cmake option.
# SOURCES lists the source files that are part of the plugin. Entries are
#   listed without extension. For an entry <file>, both <file>.h and <file>.cc
#   are added if the files exist.
# DEPENDS lists plugins that will be automatically enabled if this plugin is
#   enabled. If the dependency was not enabled before, this will be logged.
# DEPENDENCY_ONLY disables the plugin unless it is needed as a dependency and
#   hides the option to enable the plugin in cmake GUIs like ccmake.
# CORE_PLUGIN always enables the plugin (even if DISABLE_PLUGINS_BY_DEFAULT
#   is used) and hides the option to disable it in CMake GUIs like ccmake.

option(
    DISABLE_PLUGINS_BY_DEFAULT
    "If set to YES only plugins that are specifically enabled will be compiled"
    NO)
# This option should not show up in CMake GUIs like ccmake where all
# plugins are enabled or disabled manually.
mark_as_advanced(DISABLE_PLUGINS_BY_DEFAULT)

fast_downward_plugin(
    NAME CORE_SOURCES
    HELP "Core source files"
    SOURCES
        planner

        abstract_task
        axioms
        causal_graph
        equivalence_relation
        evaluation_context
        evaluation_result
        global_operator
        globals
        global_state
        heuristic_cache
        heuristic
        int_packer
        operator_cost
        option_parser
        option_parser_util
        per_state_information
        plugin
        pruning_method
        priority_queue
        sampling
        scalar_evaluator
        search_engine
        search_node_info
        search_progress
        search_space
        search_statistics
        segmented_vector
        state_id
        state_registry
        task_proxy
        task_tools
        variable_order_finder

        open_lists/open_list
        open_lists/open_list_factory
    DEPENDS SUCCESSOR_GENERATOR
    CORE_PLUGIN
)

fast_downward_plugin(
    NAME SUCCESSOR_GENERATOR
    HELP "Successor generator"
    SOURCES
        task_utils/successor_generator
        task_utils/successor_generator_factory
        task_utils/successor_generator_internals
    DEPENDENCY_ONLY
)

fast_downward_plugin(
    NAME ALTERNATION_OPEN_LIST
    HELP "Open list that alternates between underlying open lists in a round-robin manner"
    SOURCES
        open_lists/alternation_open_list
)

fast_downward_plugin(
    NAME BEST_FIRST_OPEN_LIST
    HELP "Open list that selects the best element according to a single evaluation function"
    SOURCES
        open_lists/standard_scalar_open_list
)

fast_downward_plugin(
    NAME EPSILON_GREEDY_OPEN_LIST
    HELP "Open list that chooses an entry randomly with probability epsilon"
    SOURCES
        open_lists/epsilon_greedy_open_list
)

fast_downward_plugin(
    NAME PARETO_OPEN_LIST
    HELP "Pareto open list"
    SOURCES
        open_lists/pareto_open_list
)

fast_downward_plugin(
    NAME TIEBREAKING_OPEN_LIST
    HELP "Tiebreaking open list"
    SOURCES
        open_lists/tiebreaking_open_list
)

fast_downward_plugin(
    NAME TYPE_BASED_OPEN_LIST
    HELP "Type-based open list"
    SOURCES
        open_lists/type_based_open_list
)

fast_downward_plugin(
    NAME OPTIONS
    HELP "Option parsing and plugin definition"
    SOURCES
        options/any
        options/bounds
        options/doc_printer
        options/doc_store
        options/errors
        options/option_parser
        options/options
        options/parse_tree
        options/predefinitions
        options/plugin
        options/registries
        options/synergy
        options/token_parser
        options/type_documenter
        options/type_namer
    CORE_PLUGIN
)

fast_downward_plugin(
    NAME UTILS
    HELP "System utilities"
    SOURCES
        utils/collections
        utils/countdown_timer
        utils/dynamic_bitset
        utils/hash
        utils/language
        utils/logging
        utils/markup
        utils/math
        utils/memory
        utils/rng
        utils/rng_options
        utils/system
        utils/system_unix
        utils/system_windows
        utils/timer
    CORE_PLUGIN
)

fast_downward_plugin(
    NAME CONST_EVALUATOR
    HELP "The constant evaluator"
    SOURCES
        evaluators/const_evaluator
)

fast_downward_plugin(
    NAME G_EVALUATOR
    HELP "The g-evaluator"
    SOURCES
        evaluators/g_evaluator
)

fast_downward_plugin(
    NAME COMBINING_EVALUATOR
    HELP "The combining evaluator"
    SOURCES
        evaluators/combining_evaluator
)

fast_downward_plugin(
    NAME MAX_EVALUATOR
    HELP "The max evaluator"
    SOURCES
        evaluators/max_evaluator
    DEPENDS COMBINING_EVALUATOR
)

fast_downward_plugin(
    NAME PREF_EVALUATOR
    HELP "The pref evaluator"
    SOURCES
        evaluators/pref_evaluator
)

fast_downward_plugin(
    NAME WEIGHTED_EVALUATOR
    HELP "The weighted evaluator"
    SOURCES
        evaluators/weighted_evaluator
)

fast_downward_plugin(
    NAME SUM_EVALUATOR
    HELP "The sum evaluator"
    SOURCES
        evaluators/sum_evaluator
    DEPENDS COMBINING_EVALUATOR
)

fast_downward_plugin(
    NAME NULL_PRUNING_METHOD
    HELP "Pruning method that does nothing"
    SOURCES
        pruning/null_pruning_method
)

fast_downward_plugin(
    NAME STUBBORN_SETS
    HELP "Base class for all stubborn set partial order reduction methods"
    SOURCES
        pruning/stubborn_sets
    DEPENDENCY_ONLY
)

fast_downward_plugin(
    NAME STUBBORN_SETS_SIMPLE
    HELP "Stubborn sets simple"
    SOURCES
        pruning/stubborn_sets_simple
    DEPENDS STUBBORN_SETS
)

fast_downward_plugin(
    NAME StubbornSetsEC
    HELP "Stubborn set method that dominates expansion core"
    SOURCES
        pruning/stubborn_sets_ec
    DEPENDS STUBBORN_SETS
)

fast_downward_plugin(
    NAME SEARCH_COMMON
    HELP "Basic classes used for all search engines"
    SOURCES
        search_engines/search_common
    DEPENDS G_EVALUATOR SUM_EVALUATOR WEIGHTED_EVALUATOR
    DEPENDENCY_ONLY
)

fast_downward_plugin(
    NAME EAGER_SEARCH
    HELP "Eager search algorithm"
    SOURCES
        search_engines/eager_search
    DEPENDS SEARCH_COMMON NULL_PRUNING_METHOD
)

fast_downward_plugin(
    NAME ENFORCED_HILL_CLIMBING_SEARCH
    HELP "Lazy enforced hill-climbing search algorithm"
    SOURCES
        search_engines/enforced_hill_climbing_search
    DEPENDS SEARCH_COMMON PREF_EVALUATOR G_EVALUATOR
)

fast_downward_plugin(
    NAME ITERATED_SEARCH
    HELP "Iterated search algorithm"
    SOURCES
        search_engines/iterated_search
)

fast_downward_plugin(
    NAME LAZY_SEARCH
    HELP "Lazy search algorithm"
    SOURCES
        search_engines/lazy_search
    DEPENDS SEARCH_COMMON
)

fast_downward_plugin(
    NAME LP_SOLVER
    HELP "Interface to an LP solver"
    SOURCES
        lp/lp_internals
        lp/lp_solver
    DEPENDENCY_ONLY
)

fast_downward_plugin(
    NAME RELAXATION_HEURISTIC
    HELP "The base class for relaxation heuristics"
    SOURCES
        heuristics/relaxation_heuristic
    DEPENDENCY_ONLY
)

fast_downward_plugin(
    NAME ADDITIVE_HEURISTIC
    HELP "The additive heuristic"
    SOURCES
        heuristics/additive_heuristic
    DEPENDS RELAXATION_HEURISTIC
)

fast_downward_plugin(
    NAME BLIND_SEARCH_HEURISTIC
    HELP "The 'blind search' heuristic"
    SOURCES
        heuristics/blind_search_heuristic
)

fast_downward_plugin(
    NAME CONTEXT_ENHANCED_ADDITIVE_HEURISTIC
    HELP "The context-enhanced additive heuristic"
    SOURCES
        heuristics/cea_heuristic
    DEPENDS DOMAIN_TRANSITION_GRAPH
)

fast_downward_plugin(
    NAME CG_HEURISTIC
    HELP "The causal graph heuristic"
    SOURCES heuristics/cg_heuristic
            heuristics/cg_cache
    DEPENDS DOMAIN_TRANSITION_GRAPH
)

fast_downward_plugin(
    NAME DOMAIN_TRANSITION_GRAPH
    HELP "DTGs used by cg and cea heuristic"
    SOURCES
        domain_transition_graph
    DEPENDENCY_ONLY
)

fast_downward_plugin(
    NAME FF_HEURISTIC
    HELP "The FF heuristic (an implementation of the RPG heuristic)"
    SOURCES
        heuristics/ff_heuristic
    DEPENDS ADDITIVE_HEURISTIC
)

fast_downward_plugin(
    NAME GOAL_COUNT_HEURISTIC
    HELP "The goal-counting heuristic"
    SOURCES
        heuristics/goal_count_heuristic
)

fast_downward_plugin(
    NAME HM_HEURISTIC
    HELP "The h^m heuristic"
    SOURCES
        heuristics/hm_heuristic
)

fast_downward_plugin(
    NAME LANDMARK_CUT_HEURISTIC
    HELP "The LM-cut heuristic"
    SOURCES
        heuristics/lm_cut_heuristic
        heuristics/lm_cut_landmarks
)

fast_downward_plugin(
    NAME MAX_HEURISTIC
    HELP "The Max heuristic"
    SOURCES
        heuristics/max_heuristic
    DEPENDS RELAXATION_HEURISTIC
)

fast_downward_plugin(
    NAME CORE_TASKS
    HELP "Core task transformations"
    SOURCES
        tasks/cost_adapted_task
        tasks/delegating_task
        tasks/root_task
    CORE_PLUGIN
)

fast_downward_plugin(
    NAME EXTRA_TASKS
    HELP "Non-core task transformations"
    SOURCES
        tasks/domain_abstracted_task
        tasks/domain_abstracted_task_factory
        tasks/modified_goals_task
        tasks/modified_operator_costs_task
    DEPENDENCY_ONLY
)

fast_downward_plugin(
    NAME CEGAR
    HELP "Plugin containing the code for CEGAR heuristics"
    SOURCES
        cegar/abstraction
        cegar/abstract_search
        cegar/abstract_state
        cegar/additive_cartesian_heuristic
        cegar/arc
        cegar/cartesian_heuristic_function
        cegar/cost_saturation
        cegar/domains
        cegar/refinement_hierarchy
        cegar/split_selector
        cegar/subtask_generators
        cegar/utils
        cegar/utils_landmarks
    DEPENDS ADDITIVE_HEURISTIC EXTRA_TASKS LANDMARKS
)

fast_downward_plugin(
    NAME MAS_HEURISTIC
    HELP "The Merge-and-Shrink heuristic"
    SOURCES
        merge_and_shrink/distances
        merge_and_shrink/factored_transition_system
        merge_and_shrink/fts_factory
        merge_and_shrink/heuristic_representation
        merge_and_shrink/label_equivalence_relation
        merge_and_shrink/label_reduction
        merge_and_shrink/labels
        merge_and_shrink/merge_and_shrink_heuristic
        merge_and_shrink/merge_scoring_function
        merge_and_shrink/merge_scoring_function_dfp
        merge_and_shrink/merge_scoring_function_goal_relevance
        merge_and_shrink/merge_scoring_function_single_random
        merge_and_shrink/merge_scoring_function_total_order
        merge_and_shrink/merge_selector
        merge_and_shrink/merge_selector_score_based_filtering
        merge_and_shrink/merge_strategy
        merge_and_shrink/merge_strategy_aliases
        merge_and_shrink/merge_strategy_factory
        merge_and_shrink/merge_strategy_factory_precomputed
        merge_and_shrink/merge_strategy_factory_stateless
        merge_and_shrink/merge_strategy_precomputed
        merge_and_shrink/merge_strategy_stateless
        merge_and_shrink/merge_tree
        merge_and_shrink/merge_tree_factory
        merge_and_shrink/merge_tree_factory_linear
        merge_and_shrink/shrink_bisimulation
        merge_and_shrink/shrink_bucket_based
        merge_and_shrink/shrink_fh
        merge_and_shrink/shrink_random
        merge_and_shrink/shrink_strategy
        merge_and_shrink/transition_system
        merge_and_shrink/types
)

fast_downward_plugin(
    NAME LANDMARKS
    HELP "Plugin containing the code to reason with landmarks"
    SOURCES
        landmarks/exploration
        landmarks/h_m_landmarks
        landmarks/lama_ff_synergy
        landmarks/landmark_cost_assignment
        landmarks/landmark_count_heuristic
        landmarks/landmark_factory
        landmarks/landmark_factory_rpg_exhaust
        landmarks/landmark_factory_rpg_sasp
        landmarks/landmark_factory_zhu_givan
        landmarks/landmark_graph
        landmarks/landmark_graph_merged
        landmarks/landmark_status_manager
        landmarks/util
    DEPENDS LP_SOLVER
)

fast_downward_plugin(
    NAME OPERATOR_COUNTING
    HELP "Plugin containing the code for operator counting heuristics"
    SOURCES
        operator_counting/constraint_generator
        operator_counting/lm_cut_constraints
        operator_counting/operator_counting_heuristic
        operator_counting/pho_constraints
        operator_counting/state_equation_constraints
    DEPENDS LP_SOLVER LANDMARK_CUT_HEURISTIC PDBS
)

fast_downward_plugin(
    NAME PDBS
    HELP "Plugin containing the code for PDBs"
    SOURCES
        pdbs/canonical_pdbs
        pdbs/canonical_pdbs_heuristic
        pdbs/dominance_pruning
        pdbs/incremental_canonical_pdbs
        pdbs/match_tree
        pdbs/max_additive_pdb_sets
        pdbs/max_cliques
        pdbs/pattern_collection_information
        pdbs/pattern_database
        pdbs/pattern_collection_generator_combo
        pdbs/pattern_collection_generator_genetic
        pdbs/pattern_collection_generator_hillclimbing
        pdbs/pattern_collection_generator_manual
        pdbs/pattern_collection_generator_systematic
        pdbs/pattern_generator_greedy
        pdbs/pattern_generator_manual
        pdbs/pattern_generator
        pdbs/pdb_heuristic
        pdbs/types
        pdbs/validation
        pdbs/zero_one_pdbs
        pdbs/zero_one_pdbs_heuristic
)

fast_downward_plugin(
    NAME POTENTIALS
    HELP "Plugin containing the code for potential heuristics"
    SOURCES
        potentials/diverse_potential_heuristics
        potentials/potential_function
        potentials/potential_heuristic
        potentials/potential_max_heuristic
        potentials/potential_optimizer
        potentials/sample_based_potential_heuristics
        potentials/single_potential_heuristics
        potentials/util
    DEPENDS LP_SOLVER
)

fast_downward_plugin(
    NAME BOOST
    HELP "Boost dependency plugin"
    DEPENDENCY_ONLY
)

fast_downward_plugin(
    NAME CONJUNCTIONS
    HELP "Plugin containing the code for partial delete relaxation heuristics with explicit conjunctions"
    SOURCES
        # hCFF heuristic and conjunction utilities
        conjunctions/conflict_extraction
        conjunctions/conjunctions
        conjunctions/conjunctions_heuristic
        conjunctions/conjunctions_subgoal_count_heuristic
        conjunctions/conjunctions_subset_generator
        conjunctions/generation_strategy
        conjunctions/novelty_heuristic
        conjunctions/novelty_linker
        conjunctions/utils

        # search engines with online refinement
        conjunctions/search_engines/enforced_hill_climbing_novelty_relaxed_subgoals_search
        conjunctions/search_engines/enforced_hill_climbing_novelty_relaxed_subgoals_search_lazy
        conjunctions/search_engines/enforced_hill_climbing_novelty_search
        conjunctions/search_engines/enforced_hill_climbing_search
        conjunctions/search_engines/ipc18_iterated_search
        conjunctions/search_engines/lazy_search
        conjunctions/search_engines/lazy_search_local_lookahead
        conjunctions/search_engines/lazy_search_min_set
        conjunctions/search_engines/lazy_search_min_set_continue
        conjunctions/search_engines/lazy_search_min_set_restart
        conjunctions/search_engines/lazy_search_simple
        conjunctions/search_engines/online_learning_search_engine
        conjunctions/search_engines/relaxed_plan_search

        # GBFS with lookahead based on relaxed subgoals and YAHSP
        conjunctions/search_engines/lazy_search_dual_lookahead
        conjunctions/search_engines/lazy_search_relaxed_subgoals_lookahead
        conjunctions/search_engines/lazy_search_relaxed_subgoals_lookahead_bounded_expansions
        conjunctions/search_engines/lazy_search_yahsp_lookahead
        conjunctions/search_engines/yahsp_lookahead
        heuristics/subgoal_count_heuristic
        search_engines/gbfs_relaxed_subgoals_lookahead
        search_engines/gbfs_yahsp_lookahead
    DEPENDS BOOST SEARCH_COMMON PREF_EVALUATOR BEST_FIRST_OPEN_LIST TIEBREAKING_OPEN_LIST ALTERNATION_OPEN_LIST
)

fast_downward_plugin(
    NAME GREY
    HELP "Plugin containing the code for grey planning heuristics"
    SOURCES
        grey/DAG_new_agenda
        grey/dtg_operators
        grey/graph_algorithms/scc
        grey/graph_algorithms/topological_sort
        grey/graph_algorithms/transitive_closure
        grey/grey_heuristic
        grey/grey_initial_state_search
        grey/grey_operator
        grey/learning/PDB_state_space_sample
        grey/learning/probe_state_space_sample
        grey/learning/state_space_sample
        grey/variable_ordering
    DEPENDS ADDITIVE_HEURISTIC
)

fast_downward_plugin(
    NAME RED_BLACK
    HELP "Plugin containing the code for red-black planning heuristics"
    SOURCES
        red_black/dtg_operators
        red_black/red_black_DAG_fact_following_heuristic
        red_black/red_black_operator
    DEPENDS ADDITIVE_HEURISTIC GREY
)


fast_downward_add_plugin_sources(PLANNER_SOURCES)

# The order in PLANNER_SOURCES influences the order in which object
# files are given to the linker, which can have a significant influence
# on performance (see issue67). The general recommendation seems to be
# to list files that define functions after files that use them.
# We approximate this by reversing the list, which will put the plugins
# first, followed by the core files, followed by the main file.
# This is certainly not optimal, but works well enough in practice.
list(REVERSE PLANNER_SOURCES)
