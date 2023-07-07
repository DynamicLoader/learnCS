/**
 * @file QUtils.hpp
 * @author LuYi, Lin HongJie
 * @brief Utils provided to QML
 * @version 0.1
 * @date 2023-05-24
 *
 * @copyright Copyright (c) 2023
 *
 * Reference:
 *  https://blog.csdn.net/baidu_36639782/article/details/124385846
 *
 */

#ifndef __QUTILS_HPP__
#define __QUTILS_HPP__

#include <iostream>

#include <QObject>
#include <QQmlEngine>
#include <QUrl>
#include <QFile>
#include <QList>
#include <Qdir>

#include <opencv2/core.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "ErrorNo.h"

/**
 * @brief QUtils_impl namespace. Do NOT using this namespace directly!
 */
namespace QUtils_impl
{
using namespace cv;

// For ImgDiffVisualizer
constexpr int diffTheshold = 10;

static double _sigmoid(double val, double offset)
{
    return 1.0 / (1.0 + pow(2.7182818284590452353602874713527, -val + offset));
}

static void _diff_1chan(const Mat &a, const Mat &b, Mat &output)
{
    if (a.channels() > 1 or b.channels() > 1)
        return;

    if (a.cols != b.cols or a.rows != b.rows)
        return;

    output = Mat::zeros(a.size(), CV_8UC1);
    for (int i = 0; i < a.rows; i++)
        for (int j = 0; j < a.cols; j++)
        {
            int diff = abs(a.at<uchar>(i, j) - b.at<uchar>(i, j));
            output.at<uchar>(i, j) = _sigmoid(diff, diffTheshold) * 255;
        }
}

enum ImageType
{
    Color = 0,
    GrayScale = 1,
    Binary = 2
};


/**
 * @brief Utils provided to QML with singleton
 * 
 */
class QUtils : public QObject
{
    Q_OBJECT
  public:
    explicit QUtils(QObject *parent = nullptr) : QObject(parent)
    {
    }

    static QObject *qmlCreateInstance(QQmlEngine *engine, QJSEngine *scriptEngine)
    {
        (void)engine;
        (void)scriptEngine;
        return new QUtils;
    }

    // ============ QML Utils =============

    /**
     * @brief Get the Source Mat from internal
     *
     * @param url Path to resource
     */
    Q_INVOKABLE cv::Mat getSourceMatFromQRC(const QString &url, bool grey = false)
    {
        QFile file(":/embed" + url);
        cv::Mat m;
        if (file.open(QIODevice::ReadOnly))
        {
            qint64 sz = file.size();
            std::vector<uchar> buf(sz);
            file.read((char *)buf.data(), sz);
            if (buf.empty())
                return m;
            m = cv::imdecode(buf, grey ? cv::IMREAD_GRAYSCALE : cv::IMREAD_COLOR);
            file.close();
        }
        // std::cout << "Get qrc res" << std::endl;
        return m;
    }

    /**
     * @brief List integrated resource with given path
     *
     * @param url Path to list
     */
    Q_INVOKABLE QStringList listInternalResource(const QString &url)
    {
        QDir dir(":/embed" + url);
        return dir.entryList(QDir::Files);
    }

    Q_INVOKABLE QString url2LocalPath(const QUrl &url)
    {
        return url.toLocalFile();
    }

    Q_INVOKABLE QUrl localPath2Url(const QString &path)
    {
        return QUrl::fromLocalFile(path);
    }

    Q_INVOKABLE cv::Mat binarizeImg(const QString &path, double thresh)
    {
        cv::Mat ret;
        // try{
        cv::Mat img = cv::imread(path.toLocal8Bit().data(), cv::IMREAD_GRAYSCALE);
        // cv::imshow("lll",img);
        // cv::waitKey();
        // std::cout << path.toLocal8Bit().data() << std::endl;
        cv::threshold(img, ret, thresh * 255, 255, cv::THRESH_BINARY);
        return ret;
        // }catch(cv::Exception ex){
        // std::cerr << "WrapperTNAM::saveDecoded throw an exception:" << ex.what() << std::endl;
        // return ret;
        // }
    }

    Q_INVOKABLE QString getErrorMsg(int err)
    {
        switch (err)
        {
        case ERRORNO(ERR_OPEN_FILE):
            return "Can't open file";
        case ERRORNO(ERR_READ_FILE):
            return "Can't read file";
        case ERRORNO(ERR_WRITE_FILE):
            return "Can't write file";
        case ERRORNO(ERR_SEEK_FILE):
            return "Can't seek file";
        case ERRORNO(ERR_EARLY_EOF):
            return "Unexpected end of file";

        case ModuleRNAM + ERR_INVALID_IMAGE_SIZE:
            return "Input image is not square. Do you forget to run preprocess?";
        case ModuleRNAM + ERR_BUSY:
            return "A job is already running.";
        case ModuleRNAM + ERR_NO_SOURCE:
            return "No source image. Load one first.";
        case ModuleRNAM + ERR_FORGET_ENCODE:
            return "RNAM image is empty. Do you forget to press Encode?";
        case ModuleRNAM + ERR_FORGET_DECODE:
            return "No decode result found. Do you forget to press Decode?";
        case ModuleRNAM + ERR_WRONG_CHAN_INDEX:
            return "Invalid channel index. Check image type.";

        case ERRORNO_EX(TNAM, ERR_EMPTY_INPUT):
            return "Empty input provided. Does the file exist and we can read it property?";
        case ERRORNO_EX(TNAM, ERR_BUSY):
            return "The task is already running! You can create a new task to perform the operation.";

        case ERRORNO_EX(ZIF,ERR_UNSUPPORTED_VERSION):
            return "The ZIF file has an unsupported version.";
        case ERRORNO_EX(ZIF,ERR_INVALID_FORMAT):
            return "The format of ZIF file is not correct. It may be encoded by other codec, or the file is corrupted.";

        default:
            return "Unexpected Error! Errno: " + QString::number(err);
        };
    }

    // ===================== Internal Utils =========================

    /**
     * @brief Generate a image showing difference between encoding result and original image.\
     *        If image is in grayscale, the difference will be shown in grayscale.\
     *        For RGB image, the difference of each channel will be interleaved.
     *
     * @param a
     * @param b
     * @param output
     */
    static void ImgDiffVisualizer(const cv::Mat &a, const cv::Mat &b, cv::Mat &output)
    {
        if (a.channels() != b.channels())
            return;

        if (a.cols != b.cols or a.rows != b.rows)
            return;

        if (a.channels() == 1)
        {
            _diff_1chan(a, b, output);
            return;
        }

        output = Mat::zeros(a.size(), CV_8UC3);
        for (int i = 0; i < a.rows; i++)
            for (int j = 0; j < a.cols; j++)
                for (int k = 0; k < a.channels(); k++)
                {
                    int diff = abs(a.at<Vec3b>(i, j)[k] - b.at<Vec3b>(i, j)[k]);
                    output.at<Vec3b>(i, j)[k] = _sigmoid(diff, diffTheshold) * 255;
                }
    }

    /**
     * @brief Return the color property of a image[Color / GrayScale / Binary]
     *
     * @param img
     * @return Enum[0: Color, 1: GrayScale, 2: Binary]
     */
    static int ScanImgType(const Mat &img)
    {
        bool is_gray = true;
        bool is_binary = true;

        for (int i = 0; i < img.rows; i++)
            for (int j = 0; j < img.cols; j++)
            {
                if (img.at<Vec3b>(i, j)[0] != img.at<Vec3b>(i, j)[1]
                    or img.at<Vec3b>(i, j)[1] != img.at<Vec3b>(i, j)[2]
                    or img.at<Vec3b>(i, j)[0] != img.at<Vec3b>(i, j)[2])
                    return Color;

                if (is_gray)
                    for (int c = 0; c < 3; c++)
                        if (img.at<Vec3b>(i, j)[c] != 0 and img.at<Vec3b>(i, j)[c] != 255)
                            is_binary = false;

                if (!is_gray and !is_binary)
                    return Color;
            }

        return (is_binary + is_gray);
    }

    /**
     * @brief Remove the prefix "file:///" from a path string
     *
     * @param path std::string containing a path
     */
    static std::string urlStrip(std::string path)
    {
        const std::string refPrefix = "file:///";
        for (char i : refPrefix)
        {
            if (*path.begin() != i)
            {
                return ("");
            }
            else
                path.erase(path.begin());
        }

        return path;
    }

    /**
     * @brief 检查一个文件是否具有合法的后缀
     *
     * @param path 文件名
     * @return 属于常见位图后缀返回1，是ZIF时返回-1，其余返回0.
     */
    Q_INVOKABLE static int imageSuffixChecker(const QString &path)
    {
        using std::string;
        string s = path.toLocal8Bit().toStdString();
        string suffix;

        bool foundDot = false;

        long long p = s.length() - 1;
        while (p > -1)
        {
            if (s[p] == '.')
            {
                foundDot = true;
                suffix = s.substr(p + 1, s.length() - p - 1);
                break;
            }
            p--;
        }

        if (!foundDot)
            return 0;

        suffix = toLowerCase(suffix);
        if (suffix == "zif")
            return -1;

        const string validSuffix[] = {"jpg", "jpeg", "png", "bmp", "tiff", "tif"};
        for (const auto &i : validSuffix)
            if (suffix == i)
                return 1;
        return 0;
    }

  private:
};

} // namespace QUtils_impl

// Export
using QUtils = QUtils_impl::QUtils;

#endif
