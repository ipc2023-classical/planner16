#ifndef CONJUNCTIONS_ENFORCED_HILL_CLIMBING_SEARCH_H
#define CONJUNCTIONS_ENFORCED_HILL_CLIMBING_SEARCH_H

#include "online_learning_search_engine.h"

#include "../evaluation_context.h"
#include "../search_engine.h"
#include "../open_lists/open_list.h"

#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>
#include <random>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(default: 4800 4512 4706 4100 4127 4702 4239 4996 4456 4458 4505)
#endif

namespace options {
class Options;
}

namespace conjunctions {
// Enforced hill-climbing with deferred evaluation and online learning of explicit conjunctions in local minima.
class EnforcedHillClimbingSearch : public OnlineLearningSearchEngine {
public:
	explicit EnforcedHillClimbingSearch(const options::Options &opts);
	~EnforcedHillClimbingSearch() override;

	void print_statistics() const override;

	enum class PreferredUsage {
		PRUNE_BY_PREFERRED,
		RANK_PREFERRED_FIRST
	};

protected:
	void initialize() override;
	SearchStatus step() override;

private:
	auto get_successors(EvaluationContext &eval_context) -> std::vector<const GlobalOperator *>;
	void expand(EvaluationContext &eval_context, SearchSpace &current_search_space);
	void reach_state(const GlobalState &parent, const GlobalOperator &op, const GlobalState &state);
	auto ehc(SearchSpace &current_search_space) -> SearchStatus;

	std::unique_ptr<EdgeOpenList> open_list;

	ConjunctionsHeuristic *heuristic;
	std::vector<Heuristic *> preferred_operator_heuristics;
	std::set<Heuristic *> heuristics;
	bool use_preferred;
	PreferredUsage preferred_usage;

	EvaluationContext current_eval_context;
	int current_real_g;

	// Statistics
	std::map<int, std::pair<int, int>> d_counts;
	int last_num_expanded;

	std::shared_ptr<ConjunctionGenerationStrategy> conjunctions_strategy;

	struct EHCCStatistics {
		EHCCStatistics() :
			num_ehc_phases(0),
			num_dead_ends_during_learning(0),
			num_no_better_state_after_learning(0),
			max_stagnation_count(0),
			total_stagnation_count(0),
			num_saved_evaluations(0),
			num_search_space_exhaustion(0),
			num_dead_ends(0),
			total_dead_end_backjump_length(0),
			num_expansions_in_ehc_phases_with_refinement(0) {}

		int num_ehc_phases;
		int num_dead_ends_during_learning;
		int num_no_better_state_after_learning;
		int max_stagnation_count;
		int total_stagnation_count;
		int num_saved_evaluations;
		int num_search_space_exhaustion;
		int num_dead_ends;
		int total_dead_end_backjump_length;
		int num_expansions_in_ehc_phases_with_refinement;

		auto get_avg_expansions_per_ehc_phase(int num_total_expansions) const -> double {
			return num_ehc_phases != 0 ? num_total_expansions / static_cast<double>(num_ehc_phases) : 0.;
		}

		auto get_avg_expansions_per_ehc_phase_with_refinement(int num_refinement_phases) const -> double {
			return num_refinement_phases != 0 ? num_expansions_in_ehc_phases_with_refinement / static_cast<double>(num_refinement_phases) : 0.;
		}

		auto get_avg_stagnation_count(int num_learning_calls) const -> double {
			return total_stagnation_count != 0 ? num_learning_calls / static_cast<double>(total_stagnation_count) : 0.;
		}
	} ehcc_statistics;

	void print_ehcc_statistics() const;
	void print_intermediate_statistics(const ConjunctionsHeuristic &) const override;

	auto evaluate_if_neccessary(EvaluationContext &eval_context, const GlobalState &parent_state, const GlobalOperator &last_op) -> int;
	auto evaluate_if_neccessary(EvaluationContext &eval_context) -> int;

	auto escape_local_minimum() -> SearchStatus;

	auto handle_safe_dead_end() -> SearchStatus;
	auto handle_search_space_exhaustion() -> SearchStatus;

	auto escape_dead_end(const SearchNode &node) -> SearchStatus;
	auto escape_potential_dead_end() -> SearchStatus;

	auto restart() -> SearchStatus;
	auto restart_in_parent() -> SearchStatus;

	const int bfs_bound;

	// counter for how often learning was repeated in the same state in a row
	int learning_stagnation_counter;

	std::unordered_map<StateID, std::pair<int, std::vector<const GlobalOperator *>>> heuristic_cache;

	void update_eval_context(EvaluationContext &eval_context, const decltype(heuristic_cache)::mapped_type &cache_entry);

	int bfs_lowest_h_value;
	bool solved;

	bool k_cutoff;

	bool sse_backjumping;

	// settings
	const bool no_learning;
	const bool restart_in_dead_ends;
	const bool always_reevaluate;
	const bool enable_heuristic_cache;
	const bool randomize_successors;

	const int learning_stagnation_threshold;
	const bool learning_stagnation_restart;

	enum class SearchSpaceExhaustion {
		CONTINUE,
		RESTART,
		BACKJUMP
	} const search_space_exhaustion;

	void mark_current_state_unsafe_dead_end();

	const bool unsafe_pruning_sse;
	const bool unsafe_pruning_ls;
	const bool force_unsafe_pruning_complete;

	const double max_growth;

	std::unordered_set<StateID> excluded_states;
	std::unordered_set<StateID> unsafe_dead_ends;

	// contains the unsafe dead ends that are currently addressed by backjumping
	std::unordered_set<StateID> current_unsafe_dead_ends;

	// states in which we found a solution during learning which was rejected due to the bound
	std::unordered_set<StateID> current_solved_unmodified;

	// random number generator for successor randomization
	std::mt19937 urng;
};
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
