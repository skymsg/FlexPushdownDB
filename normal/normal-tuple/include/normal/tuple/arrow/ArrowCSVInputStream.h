//
// Created by Matt Woicik on 2/10/21.
//

#ifndef NORMAL_NORMAL_CORE_INCLUDE_NORMAL_CORE_ARROW_NEWREADABLE_H
#define NORMAL_NORMAL_CORE_INCLUDE_NORMAL_CORE_ARROW_NEWREADABLE_H

#include <arrow/io/interfaces.h>

class ArrowCSVInputStream : public arrow::io::InputStream {
  public:
    explicit ArrowCSVInputStream(std::basic_iostream<char, std::char_traits<char>> &file);
    ~ArrowCSVInputStream();

    /// \brief Read data from current file position.
    ///
    /// Read at most `nbytes` from the current file position into `out`.
    /// The number of bytes read is returned.
    arrow::Result<int64_t> Read(int64_t nbytes, void* out) override;

    /// \brief Read data from current file position.
    ///
    /// Read at most `nbytes` from the current file position. Less bytes may
    /// be read if EOF is reached. This method updates the current file position.
    ///
    /// In some cases (e.g. a memory-mapped file), this method may avoid a
    /// memory copy.
    arrow::Result<std::shared_ptr<arrow::Buffer>> Read(int64_t nbytes) override;

    arrow::Status Close() override;

    /// \brief Return the position in this stream
    arrow::Result<int64_t> Tell() const override;

    /// \brief Return whether the stream is closed
    bool closed() const override;




  protected:
    std::basic_iostream<char, std::char_traits<char>>& underlyingFile_;
    int64_t position_ = 0;
    std::vector<char*> allocations_;
};

#endif //NORMAL_NORMAL_CORE_INCLUDE_NORMAL_CORE_ARROW_NEWREADABLE_H
