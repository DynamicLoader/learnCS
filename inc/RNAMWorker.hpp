#ifndef RNAMWORKER_H
#define RNAMWORKER_H

#include "RNAMimage.hpp"
#include "ZIF.hpp"
#include <QThread>
#include <qthread.h>
#include <qtmetamacros.h>

/**
 * @brief RNAM编码线程对象
 */
class RNAMEncodeWorker : public QThread
{
    Q_OBJECT

  signals:
    void finished();

  private:
    RNAMImage *img;
    const Mat &source_img;
    int _sigma;
    int _method;

  public:
    RNAMEncodeWorker(RNAMImage *img, const Mat &source_img, int sigma, int method)
        : img(img), source_img(source_img), _sigma(sigma), _method(method)
    {
    }

    void run() override
    {
        img->fromBitmap(source_img, _method, _sigma);
        emit finished();
    }
};

/**
 * @brief RNAM解码线程对象
 */
class RNAMDecodeWorker : public QThread
{
    Q_OBJECT

  signals:
    void finished();

  private:
    RNAMImage *img;
    Mat &decoded_result;

  public:
    RNAMDecodeWorker(RNAMImage *img, Mat &decoded_result) : img(img), decoded_result(decoded_result)
    {
    }

    void run()
    {
        img->toBitmap(decoded_result);
        emit finished();
    }
};

#endif