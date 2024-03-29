#ifndef MERGE_AND_SHRINK_MERGE_STRATEGY_CLUSTERING_H
#define MERGE_AND_SHRINK_MERGE_STRATEGY_CLUSTERING_H

#include "merge_strategy.h"

#include <memory>
#include <vector>

class TaskProxy;

namespace merge_and_shrink {
class MergeSelector;
class MergeTreeFactory;
class MergeTree;
class MergeStrategyClustering : public MergeStrategy {
    const TaskProxy &task_proxy;
    std::shared_ptr<MergeTreeFactory> merge_tree_factory;
    std::shared_ptr<MergeSelector> merge_selector;
    std::vector<std::vector<int>> non_singleton_cg_sccs;
    std::vector<int> indices_of_merged_sccs;

    // Active "merge strategies" while merging a set of indices
    std::unique_ptr<MergeTree> current_merge_tree;
    std::vector<int> current_ts_indices;
public:
    MergeStrategyClustering(
        const FactoredTransitionSystem &fts,
        const TaskProxy &task_proxy,
        const std::shared_ptr<MergeTreeFactory> &merge_tree_factory,
        const std::shared_ptr<MergeSelector> &merge_selector,
        std::vector<std::vector<int>> non_singleton_cg_sccs,
        std::vector<int> indices_of_merged_sccs);
    virtual ~MergeStrategyClustering() override;
    virtual std::pair<int, int> get_next() override;
};
}

#endif
