#include "landmark_count_heuristic.h"

#include "landmark_factory.h"

#include "../option_parser.h"
#include "../plugin.h"
#include "../lp/lp_solver.h"
#include "../task_utils/successor_generator.h"
#include "../utils/memory.h"
#include "../utils/system.h"

#include <cmath>
#include <limits>
#include <unordered_map>

using namespace std;
using utils::ExitCode;

namespace landmarks {
LandmarkCountHeuristic::LandmarkCountHeuristic(const options::Options &opts)
    : Heuristic(opts),
      exploration(opts),
      use_preferred_operators(opts.get<bool>("pref")),
      ff_search_disjunctive_lms(false),
      conditional_effects_supported(
          opts.get<LandmarkFactory *>("lm_factory")->supports_conditional_effects()),
      use_cost_sharing(opts.get<bool>("admissible")) {
    cout << "Initializing landmarks count heuristic..." << endl;
    LandmarkFactory *lm_graph_factory = opts.get<LandmarkFactory *>("lm_factory");
    lgraph = lm_graph_factory->compute_lm_graph(exploration);
    bool reasonable_orders = lm_graph_factory->use_reasonable_orders();
    lm_status_manager = utils::make_unique_ptr<LandmarkStatusManager>(*lgraph);

    if (opts.get<bool>("admissible")) {
        if (reasonable_orders) {
            cerr << "Reasonable orderings should not be used for admissible heuristics" << endl;
            utils::exit_with(ExitCode::SEARCH_INPUT_ERROR);
        } else if (has_axioms()) {
            cerr << "cost partitioning does not support axioms" << endl;
            utils::exit_with(ExitCode::SEARCH_UNSUPPORTED);
        } else if (has_conditional_effects() && !conditional_effects_supported) {
            cerr << "conditional effects not supported by the landmark generation method" << endl;
            utils::exit_with(ExitCode::SEARCH_UNSUPPORTED);
        }
        if (opts.get<bool>("optimal")) {
            lm_cost_assignment = utils::make_unique_ptr<LandmarkEfficientOptimalSharedCostAssignment>(
                *lgraph,
                OperatorCost(opts.get_enum("cost_type")),
                lp::LPSolverType(opts.get_enum("lpsolver")));
        } else {
            lm_cost_assignment = utils::make_unique_ptr<LandmarkUniformSharedCostAssignment>(
                *lgraph, opts.get<bool>("alm"),
                OperatorCost(opts.get_enum("cost_type")));
        }
    } else {
        lm_cost_assignment = 0;
    }
}

void LandmarkCountHeuristic::set_exploration_goals(const GlobalState &state) {
    // Set additional goals for FF exploration
    vector<pair<int, int>> lm_leaves;
    LandmarkSet result;
    const vector<bool> &reached_lms_v = lm_status_manager->get_reached_landmarks(state);
    convert_lms(result, reached_lms_v);
    collect_lm_leaves(ff_search_disjunctive_lms, result, lm_leaves);
    exploration.set_additional_goals(lm_leaves);
}

int LandmarkCountHeuristic::get_heuristic_value(const GlobalState &state) {
    double epsilon = 0.01;

    // Need explicit test to see if state is a goal state. The landmark
    // heuristic may compute h != 0 for a goal state if landmarks are
    // achieved before their parents in the landmarks graph (because
    // they do not get counted as reached in that case). However, we
    // must return 0 for a goal state.

    bool dead_end = lm_status_manager->update_lm_status(state);

    if (dead_end) {
        return DEAD_END;
    }

    int h = -1;

    if (use_cost_sharing) {
        double h_val = lm_cost_assignment->cost_sharing_h_value();
        h = static_cast<int>(ceil(h_val - epsilon));
    } else {
        lgraph->count_costs();

        int total_cost = lgraph->cost_of_landmarks();
        int reached_cost = lgraph->get_reached_cost();
        int needed_cost = lgraph->get_needed_cost();

        h = total_cost - reached_cost + needed_cost;
    }

    // Two plausibility tests in debug mode.
    assert(h >= 0);

    return h;
}

int LandmarkCountHeuristic::compute_heuristic(const GlobalState &state) {
    bool goal_reached = test_goal(state);
    if (goal_reached)
        return 0;

    int h = get_heuristic_value(state);

    // no (need for) helpful actions, return
    if (!use_preferred_operators) {
        return h;
    }

    // Try generating helpful actions (those that lead to new leaf LM in the
    // next step). If all LMs have been reached before or no new ones can be
    // reached within next step, helpful actions are those occuring in a plan
    // to achieve one of the LM leaves.

    LandmarkSet reached_lms;
    vector<bool> &reached_lms_v = lm_status_manager->get_reached_landmarks(state);
    convert_lms(reached_lms, reached_lms_v);

    int num_reached = reached_lms.size();
    if (num_reached == lgraph->number_of_landmarks()
        || !generate_helpful_actions(state, reached_lms)) {
        set_exploration_goals(state);

        // Use FF to plan to a landmark leaf
        vector<pair<int, int>> leaves;
        collect_lm_leaves(ff_search_disjunctive_lms, reached_lms, leaves);
        if (!exploration.plan_for_disj(leaves, state)) {
            exploration.exported_ops.clear();
            return DEAD_END;
        }
        for (const GlobalOperator *exported_op : exploration.exported_ops) {
            set_preferred(exported_op);
        }
        exploration.exported_ops.clear();
    }

    return h;
}

void LandmarkCountHeuristic::collect_lm_leaves(bool disjunctive_lms,
                                               LandmarkSet &reached_lms, vector<pair<int, int>> &leaves) {
    for (const LandmarkNode *node_p : lgraph->get_nodes()) {
        if (!disjunctive_lms && node_p->disjunctive)
            continue;

        if (reached_lms.find(node_p) == reached_lms.end()
            && !check_node_orders_disobeyed(*node_p, reached_lms)) {
            for (size_t i = 0; i < node_p->vars.size(); ++i) {
                pair<int, int> node_prop = make_pair(node_p->vars[i],
                                                     node_p->vals[i]);
                leaves.push_back(node_prop);
            }
        }
    }
}

bool LandmarkCountHeuristic::check_node_orders_disobeyed(const LandmarkNode &node,
                                                         const LandmarkSet &reached) const {
    for (const auto &parent : node.parents) {
        if (reached.count(parent.first) == 0) {
            return true;
        }
    }
    return false;
}

bool LandmarkCountHeuristic::generate_helpful_actions(const GlobalState &state,
                                                      const LandmarkSet &reached) {
    /* Find actions that achieve new landmark leaves. If no such action exist,
     return false. If a simple landmark can be achieved, return only operators
     that achieve simple landmarks, else return operators that achieve
     disjunctive landmarks */
    vector<const GlobalOperator *> all_operators;
    g_successor_generator->generate_applicable_ops(state, all_operators);
    vector<const GlobalOperator *> ha_simple;
    vector<const GlobalOperator *> ha_disj;

    for (size_t i = 0; i < all_operators.size(); ++i) {
        const vector<GlobalEffect> &effects = all_operators[i]->get_effects();
        for (size_t j = 0; j < effects.size(); ++j) {
            if (!effects[j].does_fire(state))
                continue;
            const FactPair fact(effects[j].var, effects[j].val);
            LandmarkNode *lm_p = lgraph->get_landmark(fact);
            if (lm_p != 0 && landmark_is_interesting(state, reached, *lm_p)) {
                if (lm_p->disjunctive) {
                    ha_disj.push_back(all_operators[i]);
                } else
                    ha_simple.push_back(all_operators[i]);
            }
        }
    }
    if (ha_disj.empty() && ha_simple.empty())
        return false;

    if (ha_simple.empty()) {
        for (size_t i = 0; i < ha_disj.size(); ++i) {
            set_preferred(ha_disj[i]);
        }
    } else {
        for (size_t i = 0; i < ha_simple.size(); ++i) {
            set_preferred(ha_simple[i]);
        }
    }
    return true;
}

bool LandmarkCountHeuristic::landmark_is_interesting(const GlobalState &s,
                                                     const LandmarkSet &reached, LandmarkNode &lm) const {
    /* A landmark is interesting if it hasn't been reached before and
     its parents have all been reached, or if all landmarks have been
     reached before, the LM is a goal, and it's not true at moment */

    int num_reached = reached.size();
    if (num_reached != lgraph->number_of_landmarks()) {
        if (reached.find(&lm) != reached.end())
            return false;
        else
            return !check_node_orders_disobeyed(lm, reached);
    }
    return lm.is_goal() && !lm.is_true_in_state(s);
}

void LandmarkCountHeuristic::notify_initial_state(const GlobalState &initial_state) {
    lm_status_manager->set_landmarks_for_initial_state(initial_state);
}

bool LandmarkCountHeuristic::notify_state_transition(
    const GlobalState &parent_state, const GlobalOperator &op,
    const GlobalState &state) {
    lm_status_manager->update_reached_lms(parent_state, op, state);
    /* TODO: The return value "true" signals that the LM set of this state
             has changed and the h value should be recomputed. It's not
             wrong to always return true, but it may be more efficient to
             check that the LM set has actually changed. */
    if (cache_h_values) {
        heuristic_cache[state].dirty = true;
    }
    return true;
}

bool LandmarkCountHeuristic::dead_ends_are_reliable() const {
    if (use_cost_sharing) {
        // admissible = true
        return true;
    }

    // admissible = false
    return !has_axioms() && (!has_conditional_effects()
                             || conditional_effects_supported);
}

void LandmarkCountHeuristic::convert_lms(LandmarkSet &lms_set,
                                         const vector<bool> &lms_vec) {
    // This function exists purely so we don't have to change all the
    // functions in this class that use LandmarkSets for the reached LMs
    // (HACK).

    for (size_t i = 0; i < lms_vec.size(); ++i)
        if (lms_vec[i])
            lms_set.insert(lgraph->get_lm_for_index(i));
}


static Heuristic *_parse(OptionParser &parser) {
    parser.document_synopsis("Landmark-count heuristic",
                             "See also Synergy");
    parser.document_note(
        "Note",
        "to use ``optimal=true``, you must build the planner with LP support. "
        "See LPBuildInstructions.");
    parser.document_note(
        "Optimal search",
        "when using landmarks for optimal search (``admissible=true``), "
        "you probably also want to enable the mpd option of the A* algorithm "
        "to improve heuristic estimates");
    parser.document_note(
        "cost_type parameter",
        "only used when ``admissible=true`` (see LandmarkFactory)");
    parser.document_language_support("action costs",
                                     "supported");
    parser.document_language_support("conditional_effects",
                                     "supported if the LandmarkFactory supports "
                                     "them; otherwise ignored with "
                                     "``admissible=false`` and not allowed with "
                                     "``admissible=true``");
    parser.document_language_support("axioms",
                                     "ignored with ``admissible=false``; not "
                                     "allowed with ``admissible=true``");
    parser.document_property("admissible",
                             "yes if ``admissible=true``");
    // TODO: this was "yes with admissible=true and optimal cost
    // partitioning; otherwise no" before.
    parser.document_property("consistent",
                             "complicated; needs further thought");
    parser.document_property("safe",
                             "yes except on tasks with axioms or on tasks with "
                             "conditional effects when using a LandmarkFactory "
                             "not supporting them");
    parser.document_property("preferred operators",
                             "yes (if enabled; see ``pref`` option)");

    parser.add_option<LandmarkFactory *>(
        "lm_factory",
        "the set of landmarks to use for this heuristic. "
        "The set of landmarks can be specified here, "
        "or predefined (see LandmarkFactory).");
    parser.add_option<bool>("admissible", "get admissible estimate", "false");
    parser.add_option<bool>(
        "optimal",
        "use optimal (LP-based) cost sharing "
        "(only makes sense with ``admissible=true``)", "false");
    parser.add_option<bool>("pref", "identify preferred operators "
                            "(see OptionCaveats#Using_preferred_operators_"
                            "with_the_lmcount_heuristic)", "false");
    parser.add_option<bool>("alm", "use action landmarks", "true");
    lp::add_lp_solver_option_to_parser(parser);
    Heuristic::add_options_to_parser(parser);
    Options opts = parser.parse();

    if (parser.dry_run())
        return nullptr;
    else
        return new LandmarkCountHeuristic(opts);
}

static Plugin<Heuristic> _plugin(
    "lmcount", _parse);
}
