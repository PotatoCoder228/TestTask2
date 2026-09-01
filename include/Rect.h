#pragma once

#include <stdint.h>


struct Point
{
    int64_t x_;
    int64_t y_;
};

struct Rect
{
public:
    Rect(Point tl, Point br)
        : tl_(tl),
          br_(br),
          bl_{tl.x_, br.y_},
          tr_{br.x_, tl.y_}
    {
    }

    Point tl_, br_, bl_, tr_;
};