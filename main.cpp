#include <fcntl.h>
#include <iomanip>
#include <sstream>
#include <stdio.h>

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
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0]
              << " <camera: int | str> <use_gpio: 0 | 1>" << std::endl;
    exit(EXIT_FAILURE);
  }

  // open camera
  int camera = atoi(argv[1]);
  cv::VideoCapture cap(camera);
  if (!cap.isOpened()) {
    std::cerr << "Error opening camera!" << std::endl;
    exit(EXIT_FAILURE);
  }
  cap.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);
  cap.set(cv::CAP_PROP_FPS, 40);

  // start mjpeg streamer
  nadjieb::MJPEGStreamer streamer;
  streamer.start(3000);

  // load model
  NanoDet detector =
      NanoDet("./nanodet/nanodet.param", "./nanodet/nanodet.bin");

  // setup gpio
  bool use_gpio = atoi(argv[2]);
  int fd;
  if (use_gpio) {
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1) {
      std::cerr << "Failed to open export for writing!" << std::endl;
      exit(EXIT_FAILURE);
    }
    write(fd, "24", 2);
    close(fd);
    fd = open("/sys/class/gpio/gpio24/direction", O_WRONLY);
    if (fd == -1) {
      std::cerr << "Failed to open direction for writing!" << std::endl;
      exit(EXIT_FAILURE);
    }
    if (write(fd, "out", 3) != 3) {
      std::cerr << "Failed to write to direction!" << std::endl;
      exit(EXIT_FAILURE);
    }
    fd = open("/sys/class/gpio/gpio24/value", O_WRONLY);
    if (fd == -1) {
      std::cerr << "Failed to open value for writing!" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  while (streamer.isRunning()) {
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) {
      std::cerr << "Failed to grab frame!" << std::endl;
      exit(EXIT_FAILURE);
    }

    // rotate
    // cv::Mat rotated = frame.clone();
    cv::Mat rotated;
    cv::rotate(frame, rotated, cv::ROTATE_180);

    // resize
    cv::Mat resized;
    resize_uniform(rotated, resized, cv::Size(WIDTH, HEIGHT));

    // detect
    auto results = detector.detect(resized, 0.5, 0.6);

    // draw bbox
    detector.draw_debug_bboxes(resized, results);

    std::vector<uchar> buf_final;
    cv::imencode(".jpg", resized, buf_final, {cv::IMWRITE_JPEG_QUALITY, 80});
    streamer.publish("/frame", std::string(buf_final.begin(), buf_final.end()));

    // write gpio
    if (use_gpio) {
      if (results.size() > 0) {
        write(fd, "1", 1);
      } else {
        write(fd, "0", 1);
      }
    }
  }

  streamer.stop();

  // close gpio
  close(fd);
  fd = open("/sys/class/gpio/unexport", O_WRONLY);
  if (fd == -1) {
    std::cerr << "Failed to open unexport for writing!" << std::endl;
    exit(EXIT_FAILURE);
  }
  if (write(fd, "24", 2) != 2) {
    std::cerr << "Failed to write to unexport!" << std::endl;
    exit(EXIT_FAILURE);
  }
  close(fd);
}
