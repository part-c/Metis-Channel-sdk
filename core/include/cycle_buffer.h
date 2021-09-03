#pragma once
// #if USE_BUFFER
#include "simple_timer.h"

#include <mutex>
#include <condition_variable>
#include <string>
using namespace std;

struct cycle_buffer {
  bool is_full_ = false;
  bool is_empty_ = true;
  int32_t r_pos_ = 0; // read position
  int32_t w_pos_ = 0; // write position
  int32_t n_ = 0; // buffer size
  int32_t remain_space_ = 0;
  char* buffer_ = nullptr;
  std::mutex mtx_;
  std::condition_variable cv_;

  /// a timer for rm empty <msgid -> buffer>
  SimpleTimer timer_;

  int32_t size() { return n_ - remain_space_; }
  int32_t remain_space() const { return remain_space_; }

 public:
  ~cycle_buffer();
  cycle_buffer(int32_t n) : n_(n), remain_space_(n) { buffer_ = new char[n_]; }
  void reset();

 public:
  // if i can read length size buffer
  bool can_read(int32_t length);
  bool can_read();
  bool can_remove(double t);

  /**
   * The real data will not be deleted. \n
   * The caller must make sure that can read length size bytes data
   */
  int32_t peek(char* data, int32_t length);

  /**
   */
  int32_t read(char* data, int32_t length);
  int32_t read(string& id, string& data);
  void realloc(int32_t length);
  int32_t write(const char* data, int32_t length);
};
// #endif