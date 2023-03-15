#include "opencv2/imgproc.hpp"
#include <opencv2/core/core.hpp>

void resize_uniform(cv::Mat &src, cv::Mat &dst, cv::Size dst_size) {
  int w = src.cols;
  int h = src.rows;
  int dst_w = dst_size.width;
  int dst_h = dst_size.height;
  dst = cv::Mat(cv::Size(dst_w, dst_h), CV_8UC3, cv::Scalar(0));

  float ratio_src = w * 1.0 / h;
  float ratio_dst = dst_w * 1.0 / dst_h;

  int tmp_w = 0;
  int tmp_h = 0;
  if (ratio_src > ratio_dst) {
    tmp_w = dst_w;
    tmp_h = floor((dst_w * 1.0 / w) * h);
  } else if (ratio_src < ratio_dst) {
    tmp_h = dst_h;
    tmp_w = floor((dst_h * 1.0 / h) * w);
  } else {
    cv::resize(src, dst, dst_size);

    return;
  }

  cv::Mat tmp;
  cv::resize(src, tmp, cv::Size(tmp_w, tmp_h));

  if (tmp_w != dst_w) {
    int index_w = floor((dst_w - tmp_w) / 2.0);
    for (int i = 0; i < dst_h; i++) {
      memcpy(dst.data + i * dst_w * 3 + index_w * 3, tmp.data + i * tmp_w * 3,
             tmp_w * 3);
    }
  } else if (tmp_h != dst_h) {
    int index_h = floor((dst_h - tmp_h) / 2.0);
    memcpy(dst.data + index_h * dst_w * 3, tmp.data, tmp_w * tmp_h * 3);
  } else {
    exit(EXIT_FAILURE);
  }
}
