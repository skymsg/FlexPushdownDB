//
// Created by matt on 1/4/20.
//

#include "normal/plan/CollateLogicalOperator.h"

#include <normal/pushdown/Collate.h>

std::shared_ptr<normal::core::Operator> normal::plan::CollateLogicalOperator::toOperator() {
  return std::make_shared<normal::pushdown::Collate>("collate");
}