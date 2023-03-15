#include <nadjieb/mjpeg_streamer.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "nanodet/nanodet.hpp"
#include "opencv2/core.hpp"
#include "utils.hpp"

const int WIDTH = 416;
const int HEIGHT = 416;

int main() {
  cv::VideoCapture cap(1);
  if (!cap.isOpened()) {
    std::cerr << "Error opening camera!" << std::endl;
    exit(EXIT_FAILURE);
  }

  nadjieb::MJPEGStreamer streamer;
  streamer.start(29999);

  NanoDet detector =
      NanoDet("./nanodet/nanodet.param", "./nanodet/nanodet.bin");

  while (streamer.isRunning()) {
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) {
      std::cerr << "Failed to grab frame!" << std::endl;
      exit(EXIT_FAILURE);
    }

    cv::Mat resized;
    resize_uniform(frame, resized, cv::Size(WIDTH, HEIGHT));

    auto results = detector.detect(resized, 0.5, 0.4);

    cv::Mat final = resized.clone();
    detector.draw_debug_bboxes(frame, final, results);

    std::vector<uchar> buf_final;
    cv::imencode(".jpg", final, buf_final, {cv::IMWRITE_JPEG_QUALITY, 90});
    streamer.publish("/frame", std::string(buf_final.begin(), buf_final.end()));
  }

  streamer.stop();
}
