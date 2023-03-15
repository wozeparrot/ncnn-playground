#ifndef UTILS_H
#define UTILS_H

#include <opencv2/core/core.hpp>

void resize_uniform(cv::Mat &src, cv::Mat &dst, cv::Size dst_size);

inline float fast_exp(float x) {
  union {
    uint32_t i;
    float f;
  } v{};
  v.i = (1 << 23) * (1.4426950409 * x + 126.93490512f);
  return v.f;
}

inline float sigmoid(float x) { return 1.0f / (1.0f + fast_exp(-x)); }
template <typename T>
int activation_function_softmax(const T *src, T *dst, int length) {
  const T alpha = *std::max_element(src, src + length);
  T denominator{0};

  for (int i = 0; i < length; ++i) {
    dst[i] = fast_exp(src[i] - alpha);
    denominator += dst[i];
  }

  for (int i = 0; i < length; ++i) {
    dst[i] /= denominator;
  }

  return 0;
}

#endif
