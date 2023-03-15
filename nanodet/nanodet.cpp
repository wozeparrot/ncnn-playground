#include "nanodet.hpp"
#include "utils.hpp"
#include <opencv2/imgproc.hpp>

static void
generate_grid_center_priors(const int input_height, const int input_width,
                            std::vector<int> &strides,
                            std::vector<CenterPrior> &center_priors) {
  for (int i = 0; i < (int)strides.size(); i++) {
    int stride = strides[i];
    int feat_w = ceil((float)input_width / stride);
    int feat_h = ceil((float)input_height / stride);
    for (int y = 0; y < feat_h; y++) {
      for (int x = 0; x < feat_w; x++) {
        CenterPrior ct;
        ct.x = x;
        ct.y = y;
        ct.stride = stride;
        center_priors.push_back(ct);
      }
    }
  }
}

NanoDet::NanoDet(const char *param, const char *bin) {
  this->net = new ncnn::Net();

  this->net->load_param(param);
  this->net->load_model(bin);
}

NanoDet::~NanoDet() { delete this->net; }

void NanoDet::preprocess(const cv::Mat &image, ncnn::Mat &in) {
  int img_w = image.cols;
  int img_h = image.rows;

  in = ncnn::Mat::from_pixels(image.data, ncnn::Mat::PIXEL_BGR, img_w, img_h);

  const float mean_vals[3] = {103.53f, 116.28f, 123.675f};
  const float norm_vals[3] = {0.017429f, 0.017507f, 0.017125f};
  in.substract_mean_normalize(mean_vals, norm_vals);
}

std::vector<BoundBox> NanoDet::detect(const cv::Mat &image, float threshold,
                                      float nms) {
  ncnn::Mat input;
  preprocess(image, input);

  auto ex = this->net->create_extractor();
  ex.input("data", input);

  ncnn::Mat out;
  ex.extract("output", out);

  std::vector<CenterPrior> center_priors;
  generate_grid_center_priors(this->input_size[0], this->input_size[1],
                              this->strides, center_priors);

  std::vector<std::vector<BoundBox>> results;
  results.resize(this->num_class);
  this->decode_infer(out, center_priors, threshold, results);

  std::vector<BoundBox> dets;
  for (int i = 0; i < (int)results.size(); i++) {
    this->nms(results[i], nms);

    for (auto box : results[i]) {
      dets.push_back(box);
    }
  }

  return dets;
}

void NanoDet::decode_infer(ncnn::Mat &feats,
                           std::vector<CenterPrior> &center_priors,
                           float threshold,
                           std::vector<std::vector<BoundBox>> &results) {
  const int num_points = center_priors.size();

  for (int idx = 0; idx < num_points; idx++) {
    const int ct_x = center_priors[idx].x;
    const int ct_y = center_priors[idx].y;
    const int stride = center_priors[idx].stride;

    const float *scores = feats.row(idx);
    float score = 0;
    int cur_label = 0;
    for (int label = 0; label < this->num_class; label++) {
      if (scores[label] > score) {
        score = scores[label];
        cur_label = label;
      }
    }
    if (score > threshold) {
      const float *bbox_pred = feats.row(idx) + this->num_class;
      results[cur_label].push_back(
          this->disPred2Bbox(bbox_pred, cur_label, score, ct_x, ct_y, stride));
    }
  }
}

BoundBox NanoDet::disPred2Bbox(const float *&dfl_det, int label, float score,
                               int x, int y, int stride) {
  float ct_x = x * stride;
  float ct_y = y * stride;
  std::vector<float> dis_pred;
  dis_pred.resize(4);
  for (int i = 0; i < 4; i++) {
    float dis = 0;
    float *dis_after_sm = new float[this->reg_max + 1];
    activation_function_softmax(dfl_det + i * (this->reg_max + 1), dis_after_sm,
                                this->reg_max + 1);
    for (int j = 0; j < this->reg_max + 1; j++) {
      dis += j * dis_after_sm[j];
    }
    dis *= stride;
    dis_pred[i] = dis;
    delete[] dis_after_sm;
  }
  float xmin = (std::max)(ct_x - dis_pred[0], .0f);
  float ymin = (std::max)(ct_y - dis_pred[1], .0f);
  float xmax = (std::min)(ct_x + dis_pred[2], (float)this->input_size[0]);
  float ymax = (std::min)(ct_y + dis_pred[3], (float)this->input_size[1]);

  return BoundBox{xmin, ymin, xmax, ymax, score, label};
}

void NanoDet::nms(std::vector<BoundBox> &boxes, float threshold) {
  std::sort(boxes.begin(), boxes.end(),
            [](BoundBox a, BoundBox b) { return a.score > b.score; });
  std::vector<float> vArea(boxes.size());
  for (int i = 0; i < int(boxes.size()); ++i) {
    vArea[i] = (boxes.at(i).x2 - boxes.at(i).x1 + 1) *
               (boxes.at(i).y2 - boxes.at(i).y1 + 1);
  }
  for (int i = 0; i < int(boxes.size()); ++i) {
    for (int j = i + 1; j < int(boxes.size());) {
      float xx1 = (std::max)(boxes[i].x1, boxes[j].x1);
      float yy1 = (std::max)(boxes[i].y1, boxes[j].y1);
      float xx2 = (std::min)(boxes[i].x2, boxes[j].x2);
      float yy2 = (std::min)(boxes[i].y2, boxes[j].y2);
      float w = (std::max)(float(0), xx2 - xx1 + 1);
      float h = (std::max)(float(0), yy2 - yy1 + 1);
      float inter = w * h;
      float ovr = inter / (vArea[i] + vArea[j] - inter);
      if (ovr >= threshold) {
        boxes.erase(boxes.begin() + j);
        vArea.erase(vArea.begin() + j);
      } else {
        j++;
      }
    }
  }
}

void NanoDet::draw_debug_bboxes(const cv::Mat &image, const cv::Mat &out,
                                const std::vector<BoundBox> &bboxes) {
  for (size_t i = 0; i < bboxes.size(); i++) {
    const BoundBox &bbox = bboxes[i];
    cv::Scalar color = cv::Scalar(NanoDet::color_list[bbox.label][0],
                                  NanoDet::color_list[bbox.label][1],
                                  NanoDet::color_list[bbox.label][2]);

    cv::rectangle(out,
                  cv::Rect(cv::Point((bbox.x1), (bbox.y1)),
                           cv::Point((bbox.x2), (bbox.y2))),
                  color);

    char text[256];
    sprintf(text, "%s %.1f%%", NanoDet::labels[bbox.label], bbox.score * 100);

    int baseLine = 0;
    cv::Size label_size =
        cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.4, 1, &baseLine);

    int x = bbox.x1;
    int y = (bbox.y1) - label_size.height - baseLine;
    if (y < 0)
      y = 0;
    if (x + label_size.width > image.cols)
      x = image.cols - label_size.width;

    cv::rectangle(
        out,
        cv::Rect(cv::Point(x, y),
                 cv::Size(label_size.width, label_size.height + baseLine)),
        color, -1);

    cv::putText(out, text, cv::Point(x, y + label_size.height),
                cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 255, 255));
  }
}
