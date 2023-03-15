#ifndef NANODET_H
#define NANODET_H

#include <ncnn/net.h>
#include <opencv2/core/core.hpp>

#include "types.hpp"

class NanoDet {
public:
  NanoDet(const char *param, const char *bin);
  ~NanoDet();

  int input_size[2] = {416, 416};
  int num_class = 80;
  int reg_max = 7;
  std::vector<int> strides = {8, 16, 32, 64};

  std::vector<BoundBox> detect(const cv::Mat &image, float threshold,
                               float nms);

  void draw_debug_bboxes(const cv::Mat &image, const cv::Mat &out,
                         const std::vector<BoundBox> &bboxes);

  constexpr static const char *labels[] = {
      "person",        "bicycle",      "car",
      "moto rycle",    "airplane",     "bus",
      "train",         "truck",        "boat",
      "traffic light", "fire hydrant", "stop sign",
      "parking meter", "bench",        "bird",
      "cat",           "dog",          "horse",
      "sheep",         "cow",          "elephant",
      "bear",          "zebra",        "giraffe",
      "backpack",      "umbrella",     "handbag",
      "tie",           "suitcase",     "frisbee",
      "skis",          "snowboard",    "sports ball",
      "kite",          "baseball bat", "baseball glove",
      "skateboard",    "surfboard",    "tennis racket",
      "bottle",        "wine glass",   "cup",
      "fork",          "knife",        "spoon",
      "bowl",          "banana",       "apple",
      "sandwich",      "orange",       "broccoli",
      "carrot",        "hot dog",      "pizza",
      "donut",         "cake",         "chair",
      "couch",         "potted plant", "bed",
      "dining table",  "toilet",       "tv",
      "laptop",        "mouse",        "remote",
      "keyboard",      "cell phone",   "microwave",
      "oven",          "toaster",      "sink",
      "refrigerator",  "book",         "clock",
      "vase",          "scissors",     "teddy bear",
      "hair drier",    "toothbrush"};

  constexpr static int color_list[80][3] = {
      {216, 82, 24},   {236, 176, 31},  {125, 46, 141},  {118, 171, 47},
      {76, 189, 237},  {238, 19, 46},   {76, 76, 76},    {153, 153, 153},
      {255, 0, 0},     {255, 127, 0},   {190, 190, 0},   {0, 255, 0},
      {0, 0, 255},     {170, 0, 255},   {84, 84, 0},     {84, 170, 0},
      {84, 255, 0},    {170, 84, 0},    {170, 170, 0},   {170, 255, 0},
      {255, 84, 0},    {255, 170, 0},   {255, 255, 0},   {0, 84, 127},
      {0, 170, 127},   {0, 255, 127},   {84, 0, 127},    {84, 84, 127},
      {84, 170, 127},  {84, 255, 127},  {170, 0, 127},   {170, 84, 127},
      {170, 170, 127}, {170, 255, 127}, {255, 0, 127},   {255, 84, 127},
      {255, 170, 127}, {255, 255, 127}, {0, 84, 255},    {0, 170, 255},
      {0, 255, 255},   {84, 0, 255},    {84, 84, 255},   {84, 170, 255},
      {84, 255, 255},  {170, 0, 255},   {170, 84, 255},  {170, 170, 255},
      {170, 255, 255}, {255, 0, 255},   {255, 84, 255},  {255, 170, 255},
      {42, 0, 0},      {84, 0, 0},      {127, 0, 0},     {170, 0, 0},
      {212, 0, 0},     {255, 0, 0},     {0, 42, 0},      {0, 84, 0},
      {0, 127, 0},     {0, 170, 0},     {0, 212, 0},     {0, 255, 0},
      {0, 0, 42},      {0, 0, 84},      {0, 0, 127},     {0, 0, 170},
      {0, 0, 212},     {0, 0, 255},     {0, 0, 0},       {36, 36, 36},
      {72, 72, 72},    {109, 109, 109}, {145, 145, 145}, {182, 182, 182},
      {218, 218, 218}, {0, 113, 188},   {80, 182, 188},  {127, 127, 0},
  };

private:
  ncnn::Net *net;

  void preprocess(const cv::Mat &image, ncnn::Mat &in);
  void decode_infer(ncnn::Mat &feats, std::vector<CenterPrior> &center_priors,
                    float threshold,
                    std::vector<std::vector<BoundBox>> &results);
  BoundBox disPred2Bbox(const float *&dfl_det, int label, float score, int x,
                        int y, int stride);
  static void nms(std::vector<BoundBox> &boxes, float threshold);
};

#endif
