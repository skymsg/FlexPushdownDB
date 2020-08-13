//
// Created by matt on 29/7/20.
//

#ifndef NORMAL_NORMAL_TUPLE_INCLUDE_NORMAL_TUPLE_ARRAYAPPENDER_H
#define NORMAL_NORMAL_TUPLE_INCLUDE_NORMAL_TUPLE_ARRAYAPPENDER_H

#include <arrow/api.h>
#include <tl/expected.hpp>

namespace normal::tuple {

/**
 * Class for building arrays from another array.
 *
 * Mainly just for casting the source array to the appropriate type to access its values, which can
 * be done on construction rather than each append.
 *
 * Subclasses also contain some buffering which seems to be much faster than adding values
 * one by one directly to the destination array.
 */
class ArrayAppender {
public:
  virtual ~ArrayAppender() = default;

  virtual inline void appendValue(const std::shared_ptr<::arrow::Array> &array, int64_t i) = 0;

  virtual tl::expected<std::shared_ptr<arrow::Array>, std::string> finalize() = 0;

protected:
  std::shared_ptr<::arrow::DataType> type_;
  std::shared_ptr<::arrow::ArrayBuilder> builder_;
  size_t expectedSize_;

};

}

#endif //NORMAL_NORMAL_TUPLE_INCLUDE_NORMAL_TUPLE_ARRAYAPPENDER_H
