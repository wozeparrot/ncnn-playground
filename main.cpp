#include <iomanip>
#include <sstream>

#include <nadjieb/mjpeg_streamer.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "nanodet/nanodet.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "utils.hpp"

const int WIDTH = NanoDet::input_size[0];
const int HEIGHT = NanoDet::input_size[1];

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <camera>" << std::endl;
    exit(EXIT_FAILURE);
  }

  int camera = atoi(argv[1]);
  cv::VideoCapture cap(camera);
  if (!cap.isOpened()) {
    std::cerr << "Error opening camera!" << std::endl;
    exit(EXIT_FAILURE);
  }
  cap.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);
  cap.set(cv::CAP_PROP_FPS, 40);

  nadjieb::MJPEGStreamer streamer;
  streamer.start(3000);

  NanoDet detector =
      NanoDet("./nanodet/nanodet.param", "./nanodet/nanodet.bin");

  while (streamer.isRunning()) {
    // calculate fps
    static int64_t last = cv::getTickCount();
    int64_t now = cv::getTickCount();
    double fps = cv::getTickFrequency() / (now - last);
    last = now;

    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) {
      std::cerr << "Failed to grab frame!" << std::endl;
      exit(EXIT_FAILURE);
    }

    // rotate
    cv::Mat rotated = frame.clone();
    cv::rotate(frame, rotated, cv::ROTATE_180);

    // resize
    cv::Mat resized;
    resize_uniform(rotated, resized, cv::Size(WIDTH, HEIGHT));

    // detect
    auto results = detector.detect(resized, 0.5, 0.6);

    // draw bbox
    detector.draw_debug_bboxes(resized, results);

    // write fps
    // convert fps to string limited to 2 decimal places
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << fps;
    std::string fps_str = ss.str();
    cv::putText(resized, fps_str, cv::Point(0, 14), cv::FONT_HERSHEY_SIMPLEX,
                0.55, cv::Scalar(255, 0, 255), 2);

    std::vector<uchar> buf_final;
    cv::imencode(".jpg", resized, buf_final, {cv::IMWRITE_JPEG_QUALITY, 90});
    streamer.publish("/frame", std::string(buf_final.begin(), buf_final.end()));
  }

  streamer.stop();
}
