//
// Created by matt on 29/4/20.
//

#ifndef NORMAL_NORMAL_PUSHDOWN_INCLUDE_NORMAL_PUSHDOWN_JOIN_HASHTABLEMESSAGE_H
#define NORMAL_NORMAL_PUSHDOWN_INCLUDE_NORMAL_PUSHDOWN_JOIN_HASHTABLEMESSAGE_H

#include <memory>
#include <unordered_map>

#include <arrow/scalar.h>

#include <normal/core/message/Message.h>

class HashTableMessage : public normal::core::message::Message {

public:
  HashTableMessage(const std::shared_ptr<std::unordered_multimap<std::shared_ptr<arrow::Scalar>, long>> &hashTable,
				   const std::string &sender);
  const std::shared_ptr<std::unordered_multimap<std::shared_ptr<arrow::Scalar>, long>> &getHashtable() const;
private:
  std::shared_ptr<std::unordered_multimap<std::shared_ptr<arrow::Scalar>, long>> hashtable_;

};

#endif //NORMAL_NORMAL_PUSHDOWN_INCLUDE_NORMAL_PUSHDOWN_JOIN_HASHTABLEMESSAGE_H
