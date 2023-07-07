#ifndef _INCL_HEADER
#define _INCL_HEADER

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

using cv::Mat;
using cv::Point;
using cv::Scalar;

/**
 * @brief 表示一个长方形. 存储了左上角和右下角的坐标和颜色信息.
 */
class Color
{
    Point lt;
    Point rb;
    Scalar grey;

  public:
    Color()
    {
        grey[0] = grey[1] = grey[2] = grey[3] = -1;
    }
    void setLt(Point lt)
    {
        this->lt = lt;
    }
    void setRb(Point rb)
    {
        this->rb = rb;
    }
    Point getLt()
    {
        return lt;
    }
    Point getRb()
    {
        return rb;
    }
    void setGrey(Scalar grey)
    {
        this->grey = grey;
    }
    Scalar getGrey()
    {
        return grey;
    }
};

#endif