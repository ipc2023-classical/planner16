#ifndef HEURISTICS_FF_HEURISTIC_H
#define HEURISTICS_FF_HEURISTIC_H

#include "additive_heuristic.h"

#include <vector>

namespace ff_heuristic {
using relaxation_heuristic::PropID;
using relaxation_heuristic::OpID;

using relaxation_heuristic::NO_OP;

using relaxation_heuristic::Proposition;
using relaxation_heuristic::UnaryOperator;

/*
  TODO: In a better world, this should not derive from
        AdditiveHeuristic. Rather, the common parts should be
        implemented in a common base class. That refactoring could be
        made at the same time at which we also unify this with the
        other relaxation heuristics and the additional FF heuristic
        implementation in the landmark code.
*/
class FFHeuristic : public additive_heuristic::AdditiveHeuristic {
    // Relaxed plans are represented as a set of operators implemented
    // as a bit vector.
    using RelaxedPlan = std::vector<bool>;
    RelaxedPlan relaxed_plan;
    // this is used to keep track of the leaf states used by decoupled search
    std::vector<int> leaf_state_costs;
    int current_leaf_cost;
    void mark_preferred_operators_and_relaxed_plan(
        const GlobalState &state, PropID goal_id);
    void setup_exploration_queue_decoupled_leaf_states(const GlobalState &state);
protected:
    virtual int compute_heuristic(const GlobalState &state) override;
public:
    explicit FFHeuristic(const options::Options &opts);
};
}

#endif
