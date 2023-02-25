#include <io.h>
#include <memory>

#include <tinylog/Buffer.h>

namespace tinylog {

Buffer::Buffer(uint64_t capacity)
{
  pt_data_ = nb::chcalloc(capacity);
  size_ = 0;
  capacity_ = capacity;
}

Buffer::~Buffer()
{
  nb_free(pt_data_);
}

/*
 * Append time and log to buffer.
 * This function should be locked.
 * return value:
 * 0  : success
 * -1 : fail, buffer full
 */
int32_t Buffer::TryAppend(struct tm* pt_time, int64_t u_sec, const char* file_name, int32_t line,
  const char* func_name, std::string& str_log_level, const char* log_data)
{
  /*
   * date: 11 byte
   * time: 13 byte
   * line number: at most 5 byte
   * log level: 9 byte
   */
  std::string::size_type append_len = 24 + strlen(file_name) + 5 + strlen(func_name) + 9 + strlen(log_data);

  if (append_len + size_ > capacity_)
  {
    return -1;
  }

  int n_append = sprintf(pt_data_ + size_, "%d-%02d-%02d %02d:%02d:%02d.%.03ld %s %d %s %s %s\n",
      pt_time->tm_year + 1900, pt_time->tm_mon + 1, pt_time->tm_mday,
      pt_time->tm_hour, pt_time->tm_min, pt_time->tm_sec, (long)(u_sec / 1000),
      file_name, line, func_name, str_log_level.c_str(),
      log_data);

  if (n_append > 0)
  {
    size_ += n_append;
  }

  return 0;
}

int32_t Buffer::TryAppend(const void* pt_log, int32_t ToWrite)
{
  /*
   * date: 11 byte
   * time: 13 byte
   * line number: at most 5 byte
   * log level: 9 byte
  */
  size_t append_len = ToWrite; // 24 + strlen(pt_file) + 5 + strlen(pt_func) + 9 + strlen(pt_log);

  if (append_len + size_ > capacity_)
  {
    return -1;
  }
  // TODO: libmemcpy_memmove
  memmove(pt_data_ + size_, pt_log, ToWrite);
  int32_t n_append = ToWrite;
  if (n_append > 0)
  {
    size_ += n_append;
  }

  return 0;
}

void Buffer::Clear()
{
  size_ = 0;
}

size_t Buffer::Size() const
{
  return size_;
}

size_t Buffer::Capacity() const
{
  return capacity_;
}

int32_t Buffer::Flush(FILE* file)
{
  size_t n_write = 0;
  while ((n_write = fwrite(pt_data_, 1, size_ - n_write, file)) != 0)
  {
    if ((n_write < 0) && (errno != EINTR))
    {
      // error
      break;
    }
    else if (n_write == size_)
    {
      // All write
      break;
    }
    else if (n_write > 0)
    {
      // Half write
    }
  }

  // error
  if (n_write < 0)
    return -1;

  fflush(file);
  return 0;
}

} // namespace tinylog