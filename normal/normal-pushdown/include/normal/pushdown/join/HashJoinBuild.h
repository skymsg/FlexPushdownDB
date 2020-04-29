//
// Created by matt on 29/4/20.
//

#ifndef NORMAL_NORMAL_PUSHDOWN_INCLUDE_NORMAL_PUSHDOWN_JOIN_HASHJOINBUILD_H
#define NORMAL_NORMAL_PUSHDOWN_INCLUDE_NORMAL_PUSHDOWN_JOIN_HASHJOINBUILD_H

#include <unordered_map>

#include <arrow/scalar.h>

#include <normal/core/Operator.h>
#include <normal/core/message/TupleMessage.h>
#include <normal/core/message/CompleteMessage.h>

namespace normal::pushdown::join {

/**
 * Operator implementing build phase of a hash join
 *
 * Builds a hash table of tuples from one of the relations in a join (ideally the smaller relation). That hashtable is
 * then used in the probe phase to select rows to add to the final joined relation.
 *
 */
class HashJoinBuild : public normal::core::Operator {

public:
  explicit HashJoinBuild(const std::string &name, std::string columnName);

  void onReceive(const core::message::Envelope &msg) override;

private:

  /**
   * The column to hash on
   */
  std::string columnName_;

  /**
   * The hashtable
   */
  std::shared_ptr<std::unordered_multimap<std::shared_ptr<arrow::Scalar>, long>> hashtable_;

  void onStart();
  void onTuple(core::message::TupleMessage msg);
  void onComplete(core::message::CompleteMessage msg);

};

}

#endif //NORMAL_NORMAL_PUSHDOWN_INCLUDE_NORMAL_PUSHDOWN_JOIN_HASHJOINBUILD_H
