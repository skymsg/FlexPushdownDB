//
// Created by matt on 16/4/20.
//

#ifndef NORMAL_NORMAL_PLAN_INCLUDE_NORMAL_PLAN_OPERATORTYPES_H
#define NORMAL_NORMAL_PLAN_INCLUDE_NORMAL_PLAN_OPERATORTYPES_H

#include <memory>
#include "ScanOperatorType.h"
#include "ProjectOperatorType.h"
#include "AggregateOperatorType.h"
#include "CollateOperatorType.h"

class OperatorTypes {
public:
  static std::shared_ptr<ScanOperatorType> scanOperatorType();
  static std::shared_ptr<ProjectOperatorType> projectOperatorType();
  static std::shared_ptr<AggregateOperatorType> aggregateOperatorType();
  static std::shared_ptr<CollateOperatorType> collateOperatorType();
};

#endif //NORMAL_NORMAL_PLAN_INCLUDE_NORMAL_PLAN_OPERATORTYPES_H
