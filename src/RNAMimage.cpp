#include <QColor>
#include "RNAMimage.hpp"

void RNAMImage::generateStat(const Mat &origin)
{
    generateStat();

    if (origin.channels() != metadata.imgtype)
        return;

    if (metadata.imgtype == 1)
    {
        Mat output;
        toBitmap(output);

        data[0].stat.psnr = PSNR(origin, output, metadata.M, metadata.N);
        return;
    }
    else if (metadata.imgtype == 3)
    {
        Mat output[3];
        for (int i = 0; i < 3; i++)
            chanToBitmap(output[i], i);

        Mat reference[3];
        split(origin, reference);

        for (int i = 0; i < 3; i++)
            data[i].stat.psnr = PSNR(reference[i], output[i], metadata.M, metadata.N);
    }
}

void RNAMImage::bitmapToChan(const Mat &source, RNAMImage::Channel &chan, int method, int sigma)
{
    if (source.empty())
        return;

    if (!_is_valid_method(method))
        return;

    // Preprocess

    Mat *graySource;
    if (source.channels() == 1)
        graySource = const_cast<Mat *>(&source);
    else
    {
        graySource = new Mat();
        cvtColor(source, *graySource, cv::COLOR_BGR2GRAY);
    }

    metadata.M = graySource->rows;
    metadata.N = graySource->cols;
    metadata.method = method;

    // Unknown usage, maybe for marking
    Mat R = Mat::zeros(metadata.M, metadata.N, CV_8UC1);
    Mat markMatrix = Mat::zeros(metadata.M, metadata.N, CV_8UC1);
    // Encode
    RNAMCEncoding(*graySource, R, markMatrix, metadata.M, metadata.N, chan.colorList, chan.coordinateList, sigma,
                  method, &progress);
    Encode(R, metadata.M, metadata.N, chan.coordinateList);
}

void RNAMImage::fromBitmap(const Mat &source, int method, int sigma)
{
    if (source.empty())
    {
        return;
    }

    if (!_is_valid_method(method))
    {
        return;
    }

    // init
    data.clear();

    Mat *channels;

    if (source.channels() == 1)
        channels = const_cast<Mat *>(&source);
    else
    {
        channels = new Mat[source.channels()];
        cv::split(source, channels);
    }

    // Preprocess
    metadata.M = channels[0].rows;
    metadata.N = channels[0].cols;
    metadata.method = method;
    metadata.imgtype = source.channels();

    for (int i = 0; i < source.channels(); i++)
    {
        Channel chan;
        bitmapToChan(channels[i], chan, method, sigma);
        data.push_back(chan);
    }

    if (source.channels() > 1)
        delete[] channels;
}

void RNAMImage::chanToBitmap(Mat &target, int chan)
{
    if (data[chan].colorList.empty() or data[chan].coordinateList.empty())
        return;

    // init
    target = Mat::zeros(metadata.M, metadata.N, CV_8UC1);
    // put
    Decode(target, metadata.M, metadata.N, data[chan].coordinateList);
    RNAMCDecoding(target, metadata.M, metadata.N, data[chan].colorList, data[chan].coordinateList);
}

void RNAMImage::toBitmap(Mat &target)
{
    if (data.empty())
    {
        return;
    }

    if (data.size() == 1)
    {
        chanToBitmap(target, 0);
        return;
    }

    vector<Mat> merging;
    for (int i = 0; i < data.size(); i++)
    {
        Mat chan;
        chanToBitmap(chan, i);
        merging.push_back(chan);
    }

    cv::merge(merging, target);
}

cv::Scalar qcolorToScalar(QColor c)
{
    std::cout << c.red() << " " << c.green() << " " << c.blue() << std::endl;
    return Scalar (c.blue(), c.green(), c.red());
}

void RNAMImage::getSegments(int chan, Mat &target, QColor color1, QColor color2)
{
    vector<Color> &p = data[chan].colorList;

    target = Mat::zeros(metadata.M, metadata.N, CV_8UC3);
    Scalar fillColor;

    Scalar beginColor = qcolorToScalar(color1);
    Scalar endColor = qcolorToScalar(color2);
    Scalar step = endColor - beginColor;

    for (int i = 0; i < p.size(); i++)
    {
        double grad = 0.5 * p[i].getLt().x / target.cols + 0.5 * p[i].getLt().y / target.rows;
        fillColor = beginColor + grad * step;
        fillColor = fillColor * 0.25 + Scalar(255, 255, 255) * 0.75;

        rectangle(target, p[i].getLt(), p[i].getRb(), fillColor, -1);
    }

    for (int i = 0; i < p.size(); i++)
    {
        double grad = 0.5 * p[i].getLt().x / target.cols + 0.5 * p[i].getLt().y / target.rows;

        fillColor = endColor - grad * step;
        if (i % 2)
            fillColor = fillColor * 0.5 + Scalar(127, 127, 127);
        rectangle(target, p[i].getLt(), p[i].getRb(), fillColor, 0.5, cv::LINE_AA);
    }
}
