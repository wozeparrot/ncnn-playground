#ifndef TYPES_H
#define TYPES_H

struct CenterPrior {
  int x;
  int y;
  int stride;
};

typedef struct BoundBox {
  float x1;
  float y1;
  float x2;
  float y2;
  float score;
  int label;
} BoundBox;

#endif
