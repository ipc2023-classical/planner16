#include "lazy_search.h"

#include "factoring.h"
#include "g_evaluator.h"
#include "heuristic.h"
#include "option_parser.h"
#include "plugin.h"
#include "open_lists/alternation_open_list.h"
#include "open_lists/open_list_plugins.h"
#include "open_lists/standard_scalar_open_list.h"
#include "task_utils/successor_generator.h"
#include "sum_evaluator.h"
#include "compliant_paths/pruning_options.h"
#include "weighted_evaluator.h"

#include <algorithm>
#include <limits>

using namespace std;

static const int DEFAULT_LAZY_BOOST = 1000;

LazySearch::LazySearch(const Options &opts)
    : SearchEngine(opts),
      open_list(opts.get<shared_ptr<OpenList<OpenListEntryLazy>>>("open")),
      reopen_closed_nodes(opts.get<bool>("reopen_closed")),
      randomize_successors(opts.get<bool>("randomize_successors")),
      preferred_successors_first(opts.get<bool>("preferred_successors_first")),
      current_state(g_initial_state()),
      current_predecessor_id(StateID::no_state),
      current_operator(OperatorID::no_operator),
      current_g(0),
      current_real_g(0) {
}

void LazySearch::set_pref_operator_heuristics(
    vector<shared_ptr<Evaluator>> &heur) {
    preferred_operator_heuristics = heur;
}

void LazySearch::initialize() {
    //TODO children classes should output which kind of search
    cout << "Conducting lazy best first search, (real) bound = " << bound << endl;

    assert(open_list != NULL);
    set<Heuristic*> hset;
    open_list->get_involved_heuristics(hset);

    for (Heuristic *h : hset) {
        estimate_heuristics.push_back(h);
        search_progress.add_heuristic(h);
    }

    // add heuristics that are used for preferred operators (in case they are
    // not also used in the open list)
    for (auto h : preferred_operator_heuristics){
        h->get_involved_heuristics(hset);
    }

    for (Heuristic *h : hset) {
        heuristics.push_back(h);
    }
    assert(!heuristics.empty());
}

void LazySearch::get_successor_operators(vector<OperatorID> &ops) {
    assert(ops.empty());
    vector<OperatorID> all_operators;
    vector<OperatorID> preferred_operators;

    g_successor_generator->generate_applicable_ops(
        current_state, all_operators);

    for (shared_ptr<Evaluator> h : preferred_operator_heuristics) {
        if (!h->is_dead_end())
            h->get_preferred_operators(preferred_operators);
    }

    if (randomize_successors) {
        random_shuffle(all_operators.begin(), all_operators.end());
        // Note that preferred_operators can contain duplicates that are
        // only filtered out later, which gives operators "preferred
        // multiple times" a higher chance to be ordered early.
        random_shuffle(preferred_operators.begin(), preferred_operators.end());
    }

    if (preferred_successors_first) {
        for (OperatorID op_id : preferred_operators) {
            if (!g_operators[op_id].is_marked()) {
                ops.push_back(op_id);
                g_operators[op_id].mark();
            }
        }

        for (OperatorID id : all_operators)
            if (!g_operators[id].is_marked())
                ops.push_back(id);
    } else {
        for (OperatorID id : preferred_operators)
            if (!g_operators[id].is_marked())
                g_operators[id].mark();
        ops.swap(all_operators);
    }
}

void LazySearch::generate_successors() {
    vector<OperatorID> operators;
    get_successor_operators(operators);
    search_progress.inc_generated(operators.size());

    for (OperatorID id : operators) {
        int new_g = current_g + get_adjusted_cost(id);
        int new_real_g = current_real_g + g_operators[id].get_cost();
        bool is_preferred = g_operators[id].is_marked();
        if (is_preferred)
            g_operators[id].unmark();
        if (new_real_g < bound) {
            open_list->evaluate(new_g, is_preferred);
            open_list->insert(
                make_pair(current_state.get_id(), id));
        }
    }
}

SearchStatus LazySearch::fetch_next_state() {
    if (open_list->empty()) {
        cout << "Completely explored state space -- no solution!" << endl;
        return FAILED;
    }

    OpenListEntryLazy next = open_list->remove_min();

    current_predecessor_id = next.first;
    current_operator = next.second;
    GlobalState current_predecessor = g_state_registry->lookup_state(current_predecessor_id);
    assert(g_operators[current_operator].is_applicable(current_predecessor));
    current_state = g_state_registry->get_successor_state(current_predecessor, g_operators[current_operator]);

    SearchNode pred_node = search_space.get_node(current_predecessor);
    current_g = pred_node.get_g() + get_adjusted_cost(current_operator);
    current_real_g = pred_node.get_real_g() + g_operators[current_operator].get_cost();

    return IN_PROGRESS;
}

SearchStatus LazySearch::step() {
    // Invariants:
    // - current_state is the next state for which we want to compute the heuristic.
    // - current_predecessor is a permanent pointer to the predecessor of that state.
    // - current_operator is the operator which leads to current_state from predecessor.
    // - current_g is the g value of the current state according to the cost_type
    // - current_g is the g value of the current state (using real costs)


    SearchNode node = search_space.get_node(current_state);
    bool reopen = reopen_closed_nodes && (current_g < node.get_g()) && !node.is_dead_end() && !node.is_new();

    if (g_factoring && PruningOptions::ignore_current_state()){
        reopen = false;
    }

    if (node.is_new() || reopen) {
        StateID dummy_id = current_predecessor_id;
        // HACK! HACK! we do this because SearchNode has no default/copy constructor
        if (dummy_id == StateID::no_state) {
            dummy_id = g_initial_state().get_id();
        }
        GlobalState parent_state = g_state_registry->lookup_state(dummy_id);
        SearchNode parent_node = search_space.get_node(parent_state);

        for (size_t i = 0; i < heuristics.size(); ++i) {
            if (current_operator != OperatorID::no_operator) {
                heuristics[i]->reach_state(parent_state, current_operator, current_state);
            }
            heuristics[i]->evaluate(current_state);
        }
        search_progress.inc_evaluated_states();
        search_progress.inc_evaluations(heuristics.size());
        open_list->evaluate(current_g, false);
        if (!open_list->is_dead_end()) {
            // We use the value of the first heuristic, because SearchSpace only
            // supported storing one heuristic value
            int h = heuristics[0]->get_value();
            if (reopen) {
                node.reopen(parent_node, current_operator);
                search_progress.inc_reopened();
            } else if (current_predecessor_id == StateID::no_state) {
                node.open_initial(h);
                search_progress.get_initial_h_values();
            } else {
                node.open(h, parent_node, current_operator);
            }
            node.close();
            if (check_goal_and_set_plan(current_state))
                return SOLVED;
            if (search_progress.check_h_progress(current_g)) {
                reward_progress();
            }
            generate_successors();
            search_progress.inc_expanded();
        } else {
            node.mark_as_dead_end();
            search_progress.inc_dead_ends();
        }
    }
    return fetch_next_state();
}

void LazySearch::reward_progress() {
    // Boost the "preferred operator" open lists somewhat whenever
    open_list->boost_preferred();
}

void LazySearch::statistics() const {
    search_progress.print_statistics();
    search_space.statistics();
}


static void _add_succ_order_options(OptionParser &parser) {
    vector<string> options;
    parser.add_option<bool>(
        "randomize_successors",
        "randomize the order in which successors are generated",
        "false");
    parser.add_option<bool>(
        "preferred_successors_first",
        "consider preferred operators first",
        "false");
    parser.document_note(
        "Successor ordering",
        "When using randomize_successors=true and "
        "preferred_successors_first=true, randomization happens before "
        "preferred operators are moved to the front.");
}

static shared_ptr<SearchEngine> _parse(OptionParser &parser) {
    parser.document_synopsis("Lazy best first search", "");

    parser.add_option<shared_ptr<OpenList<OpenListEntryLazy>>>("open", "open list");
    parser.add_option<bool>("reopen_closed",
                            "reopen closed nodes", "false");
    parser.add_list_option<shared_ptr<Evaluator>>(
        "preferred",
        "use preferred operators of these heuristics", "[]");
    _add_succ_order_options(parser);
    SearchEngine::add_options_to_parser(parser);
    Options opts = parser.parse();

    if (!parser.dry_run()) {
        shared_ptr<LazySearch> engine = make_shared<LazySearch>(opts);
        vector<shared_ptr<Evaluator>> preferred_list =
            opts.get_list<shared_ptr<Evaluator>>("preferred");
        engine->set_pref_operator_heuristics(preferred_list);
        return engine;
    }

    return 0;
}


static shared_ptr<SearchEngine> _parse_greedy(OptionParser &parser) {
    parser.document_synopsis("Greedy search (lazy)", "");
    parser.document_note(
        "Open lists",
        "In most cases, lazy greedy best first search uses "
        "an alternation open list with one queue for each evaluator. "
        "If preferred operator heuristics are used, it adds an "
        "extra queue for each of these evaluators that includes "
        "only the nodes that are generated with a preferred operator. "
        "If only one evaluator and no preferred operator heuristic is used, "
        "the search does not use an alternation open list "
        "but a standard open list with only one queue.");
    parser.document_note(
        "Equivalent statements using general lazy search",
        "\n```\n--heuristic h2=eval2\n"
        "--search lazy_greedy([eval1, h2], preferred=h2, boost=100)\n```\n"
        "is equivalent to\n"
        "```\n--heuristic h1=eval1 --heuristic h2=eval2\n"
        "--search lazy(alt([single(h1), single(h1, pref_only=true), single(h2),\n"
        "                  single(h2, pref_only=true)], boost=100),\n"
        "              preferred=h2)\n```\n"
        "------------------------------------------------------------\n"
        "```\n--search lazy_greedy([eval1, eval2], boost=100)\n```\n"
        "is equivalent to\n"
        "```\n--search lazy(alt([single(eval1), single(eval2)], boost=100))\n```\n"
        "------------------------------------------------------------\n"
        "```\n--heuristic h1=eval1\n--search lazy_greedy(h1, preferred=h1)\n```\n"
        "is equivalent to\n"
        "```\n--heuristic h1=eval1\n"
        "--search lazy(alt([single(h1), single(h1, pref_only=true)], boost=1000),\n"
        "              preferred=h1)\n```\n"
        "------------------------------------------------------------\n"
        "```\n--search lazy_greedy(eval1)\n```\n"
        "is equivalent to\n"
        "```\n--search lazy(single(eval1))\n```\n",
        true);

    parser.add_list_option<shared_ptr<Evaluator>>("evals", "scalar evaluators");
    parser.add_list_option<shared_ptr<Evaluator>>(
        "preferred",
        "use preferred operators of these heuristics", "[]");
    parser.add_option<bool>("reopen_closed",
                            "reopen closed nodes", "false");
    parser.add_option<int>(
        "boost",
        "boost value for alternation queues that are restricted "
        "to preferred operator nodes",
        to_string(DEFAULT_LAZY_BOOST));
    _add_succ_order_options(parser);
    SearchEngine::add_options_to_parser(parser);
    Options opts = parser.parse();

    if (!parser.dry_run()) {
        vector<shared_ptr<Evaluator>> evals =
            opts.get_list<shared_ptr<Evaluator>>("evals");
        vector<shared_ptr<Evaluator>> preferred_list =
            opts.get_list<shared_ptr<Evaluator>>("preferred");
        shared_ptr<OpenList<OpenListEntryLazy>> open;
        if ((evals.size() == 1) && preferred_list.empty()) {
            open = make_shared<StandardScalarOpenList<OpenListEntryLazy>>(evals[0],
                                                                 false);
        } else {
            vector<shared_ptr<OpenList<OpenListEntryLazy>>> inner_lists;
            for (size_t i = 0; i < evals.size(); ++i) {
                inner_lists.push_back(
                        make_shared<StandardScalarOpenList<OpenListEntryLazy>>(evals[i],
                                                                  false));
                if (!preferred_list.empty()) {
                    inner_lists.push_back(
                            make_shared<StandardScalarOpenList<OpenListEntryLazy>>(evals[i],
                                                                      true));
                }
            }
            open = make_shared<AlternationOpenList<OpenListEntryLazy>>(
                inner_lists, opts.get<int>("boost"));
        }
        opts.set("open", open);
        shared_ptr<LazySearch> engine = make_shared<LazySearch>(opts);
        engine->set_pref_operator_heuristics(preferred_list);
        return engine;
    }
    return 0;
}

static shared_ptr<SearchEngine> _parse_weighted_astar(OptionParser &parser) {
    parser.document_synopsis(
        "(Weighted) A* search (lazy)",
        "Weighted A* is a special case of lazy best first search.");
    parser.document_note(
        "Open lists",
        "In the general case, it uses an alternation open list "
        "with one queue for each evaluator h that ranks the nodes "
        "by g + w * h. If preferred operator heuristics are used, "
        "it adds for each of the evaluators another such queue that "
        "only inserts nodes that are generated by preferred operators. "
        "In the special case with only one evaluator and no preferred "
        "operator heuristics, it uses a single queue that "
        "is ranked by g + w * h. ");
    parser.document_note(
        "Equivalent statements using general lazy search",
        "\n```\n--heuristic h1=eval1\n"
        "--search lazy_wastar([h1, eval2], w=2, preferred=h1,\n"
        "                     bound=100, boost=500)\n```\n"
        "is equivalent to\n"
        "```\n--heuristic h1=eval1 --heuristic h2=eval2\n"
        "--search lazy(alt([single(sum([g(), weight(h1, 2)])),\n"
        "                   single(sum([g(), weight(h1, 2)]), pref_only=true),\n"
        "                   single(sum([g(), weight(h2, 2)])),\n"
        "                   single(sum([g(), weight(h2, 2)]), pref_only=true)],\n"
        "                  boost=500),\n"
        "              preferred=h1, reopen_closed=true, bound=100)\n```\n"
        "------------------------------------------------------------\n"
        "```\n--search lazy_wastar([eval1, eval2], w=2, bound=100)\n```\n"
        "is equivalent to\n"
        "```\n--search lazy(alt([single(sum([g(), weight(eval1, 2)])),\n"
        "                   single(sum([g(), weight(eval2, 2)]))],\n"
        "                  boost=1000),\n"
        "              reopen_closed=true, bound=100)\n```\n"
        "------------------------------------------------------------\n"
        "```\n--search lazy_wastar([eval1, eval2], bound=100, boost=0)\n```\n"
        "is equivalent to\n"
        "```\n--search lazy(alt([single(sum([g(), eval1])),\n"
        "                   single(sum([g(), eval2]))])\n"
        "              reopen_closed=true, bound=100)\n```\n"
        "------------------------------------------------------------\n"
        "```\n--search lazy_wastar(eval1, w=2)\n```\n"
        "is equivalent to\n"
        "```\n--search lazy(single(sum([g(), weight(eval1, 2)])), reopen_closed=true)\n```\n",
        true);

    parser.add_list_option<shared_ptr<Evaluator>>("evals", "scalar evaluators");
    parser.add_list_option<shared_ptr<Evaluator>>(
        "preferred",
        "use preferred operators of these heuristics", "[]");
    parser.add_option<bool>("reopen_closed", "reopen closed nodes", "true");
    parser.add_option<int>("boost",
                           "boost value for preferred operator open lists",
                           to_string(DEFAULT_LAZY_BOOST));
    parser.add_option<int>("w", "heuristic weight", "1");
    _add_succ_order_options(parser);
    SearchEngine::add_options_to_parser(parser);
    Options opts = parser.parse();

    opts.verify_list_non_empty<shared_ptr<Evaluator>>("evals");

    if (!parser.dry_run()) {
        vector<shared_ptr<Evaluator>> evals = opts.get_list<shared_ptr<Evaluator>>("evals");
        vector<shared_ptr<Evaluator>> preferred_list =
            opts.get_list<shared_ptr<Evaluator>>("preferred");
        vector<shared_ptr<OpenList<OpenListEntryLazy>>> inner_lists;
        for (size_t i = 0; i < evals.size(); ++i) {
            shared_ptr<GEvaluator> g = make_shared<GEvaluator>();
            vector<shared_ptr<Evaluator>> sum_evals;
            sum_evals.push_back(g);
            if (opts.get<int>("w") == 1) {
                sum_evals.push_back(evals[i]);
            } else {
                shared_ptr<WeightedEvaluator> w = make_shared<WeightedEvaluator>(
                    evals[i],
                    opts.get<int>("w"));
                sum_evals.push_back(w);
            }
            shared_ptr<SumEvaluator> f_eval = make_shared<SumEvaluator>(sum_evals);

            inner_lists.push_back(
                make_shared<StandardScalarOpenList<OpenListEntryLazy>>(f_eval, false));

            if (!preferred_list.empty()) {
                inner_lists.push_back(
                    make_shared<StandardScalarOpenList<OpenListEntryLazy>>(f_eval,
                                                                  true));
            }
        }
        shared_ptr<OpenList<OpenListEntryLazy>> open;
        if (inner_lists.size() == 1) {
            open = inner_lists[0];
        } else {
            open = make_shared<AlternationOpenList<OpenListEntryLazy>>(
                inner_lists, opts.get<int>("boost"));
        }

        opts.set("open", open);

        shared_ptr<LazySearch> engine = make_shared<LazySearch>(opts);
        engine->set_pref_operator_heuristics(preferred_list);
        return engine;
    }
    return 0;
}

static Plugin<SearchEngine> _plugin("lazy", _parse);
static Plugin<SearchEngine> _plugin_greedy("lazy_greedy", _parse_greedy);
static Plugin<SearchEngine> _plugin_weighted_astar("lazy_wastar", _parse_weighted_astar);
