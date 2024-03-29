#ifndef MAX_EVALUATOR_H
#define MAX_EVALUATOR_H

#include "combining_evaluator.h"

#include <vector>

class MaxEvaluator : public CombiningEvaluator {
protected:
    virtual int combine_values(const std::vector<int> &values);
public:
    MaxEvaluator(const std::vector<std::shared_ptr<Evaluator>> &subevaluators);
    ~MaxEvaluator();
};

#endif
