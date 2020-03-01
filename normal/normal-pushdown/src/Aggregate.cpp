//
// Created by matt on 11/12/19.
//

#include "normal/pushdown/Aggregate.h"

#include <string>
#include <utility>
#include <memory>
#include <iostream>
#include <spdlog/spdlog.h>

#include "normal/core/Operator.h"
#include "normal/core/TupleMessage.h"
#include "normal/core/Message.h"
#include "normal/pushdown/AggregateExpression.h"

Aggregate::Aggregate(std::string name, std::vector<std::unique_ptr<AggregateExpression>> expressions)
    : Operator(std::move(name)) {
  m_expressions = std::move(expressions);
}

void Aggregate::onStart() {

}

void Aggregate::onStop() {

}

void Aggregate::onReceive(const Message& msg) {

  auto tupleMessage =dynamic_cast<const TupleMessage&>(msg);
  auto tupleSet = tupleMessage.data();

  if(inputTupleSet == nullptr){
    inputTupleSet = tupleMessage.data();
  }
  else{
    auto tables = std::vector<std::shared_ptr<arrow::Table>>();
    std::shared_ptr<arrow::Table> table;
    tables.push_back(tupleMessage.data()->getTable());
    tables.push_back(inputTupleSet->getTable());
    arrow::ConcatenateTables(tables, &table);
    inputTupleSet->setTable(table);
  }
}

void Aggregate::onComplete(const Operator &op) {

  std::shared_ptr<TupleSet> aggregateTupleSet = nullptr;

  // FIXME: Only supports one expression at mo
  for (auto &expr : m_expressions) {
    aggregateTupleSet = expr->apply(inputTupleSet, aggregateTupleSet);
  }

  TupleMessage outMessage(aggregateTupleSet);
  ctx()->tell(outMessage);

  ctx()->complete();
}