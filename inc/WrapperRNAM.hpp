/**
 * @file WrapperRNAM.hpp
 * @author A1exlin
 * @brief Wrapper for RNAMImage, exposing interfaces to QML.
 * @version 0.1
 * @date 2023-05-24
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef WRAPPERRNAM_H
#define WRAPPERRNAM_H

#include "ZIF.hpp"
#include <functional>
#include <qthread.h>
#pragma once

#include "ErrorNo.h"

#include "QmlExport.hpp"
#include "QImageMemory.hpp"
#include "RNAMWorker.hpp"
#include "RNAMimage.hpp"
#include "RNAM/CommonUtil.hpp"

#include "QUtils.hpp"
#include <QObject>
#include <QDebug>
#include <qobject.h>
#include <qtmetamacros.h>
#define WrapperRNAMVersion "1.0"

using cv::Mat;

/**
 * @brief 一个用于将RNAMImage暴露给QML的类. 封装了RNAMImage的相应接口, 以及存储完成演示所需的数据，包括源图像等。并且提供了信号和槽.
 *
 */
class WrapperRNAM : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString sourceImagePath READ getSourceImagePath WRITE setSourceImagePath NOTIFY sourceImagePathChanged)
    Q_PROPERTY(QString resultSavePath READ getResultSavePath WRITE setResultSavePath NOTIFY resultSavePathChanged)
    Q_PROPERTY(QString zifOutPath READ getZifOutPath WRITE setZifOutPath NOTIFY zifOutPathChanged)
    Q_PROPERTY(QString zifInputPath READ getZifInputPath WRITE setZifInputPath NOTIFY zifInputPathChanged)

    Q_PROPERTY(int height READ getHeight NOTIFY heightChanged)
    Q_PROPERTY(int width READ getWidth NOTIFY widthChanged)

    QProperty_Helper(QColor, visualColor1);
    QProperty_Helper(QColor, visualColor2);

    int height;
    int width;

  private:
    RNAMImage img;
    Mat decoded_result;
    Mat source_img;

    QThread *jobThread;

    QString sourceImagePath;
    QString resultSavePath;

    QString zifInputPath;
    QString zifOutPath;

    static constexpr int defaultSize = 200;
    // StatWrapper stat;

  signals:
    void encodeStarted();
    void encodeFinished(int ret);

    void decodeStarted();
    void decodeFinished(int ret);

    void sourceImagePathChanged();
    void resultSavePathChanged();

    void zifInputPathChanged();
    void zifOutPathChanged();

    void error(int errorno);

    void heightChanged();
    void widthChanged();

    void statChanged();

  public slots:
    void onEncodeFinished()
    {
        delete jobThread;
        jobThread = nullptr;

        img.generateStat(source_img);

        emit encodeFinished(0);
    }
    void onDecodeFinished()
    {
        delete jobThread;
        jobThread = nullptr;

        emit decodeFinished(0);
    }

  public:
    /******* Basic QObj properties ********/

    explicit WrapperRNAM(QObject *parent = nullptr) : QObject(parent)
    {
        jobThread = nullptr;

        height = 1;
        width = 1;

        // Hatsune Miku color scheme
        _visualColor1 = 0x47c8c0;
        _visualColor2 = 0xff629d;
    }

    ~WrapperRNAM()
    {
        if (jobThread != nullptr)
        {
            jobThread->terminate();
            jobThread->wait();
            delete jobThread;
        }
    }

    Q_INVOKABLE QString getName()
    {
        return QString("WrapperRNAM " WrapperRNAMVersion);
    }

    Q_INVOKABLE QString getDescription()
    {
        return "Rectangular NAM is a novel image encoding suggested by YP Zheng et al.";
    }

    /**** Getters and setters ****/

    Q_INVOKABLE int getHeight()
    {
        if (source_img.empty())
            return defaultSize;
        return height;
    }

    Q_INVOKABLE int getWidth()
    {
        if (source_img.empty())
            return defaultSize;
        return width;
    }

    QString getSourceImagePath()
    {
        return this->sourceImagePath;
    }

    /**
     * @brief 设置要进行RNAM编码的输入位图路径。
     *
     * @param path QString 一个QUrl字符串，表示图片位置。
     * @return void
     */
    Q_INVOKABLE void setSourceImagePath(const QString &path)
    {
        this->sourceImagePath = path;
        emit sourceImagePathChanged();
    }

    Q_INVOKABLE QString getResultSavePath()
    {
        return resultSavePath;
    }

    /**
     * @brief 设置解码结果的输出路径。
     *
     * @param path QString 一个QUrl字符串，表示图片位置。
     */
    Q_INVOKABLE void setResultSavePath(const QString &path)
    {
        resultSavePath = path;
    }

    Q_INVOKABLE QString getZifInputPath()
    {
        return zifInputPath;
    }

    /**
     * @brief 设置RNAM编码结果的输入路径。
     *
     * @param path QString 一个QUrl字符串，表示图片位置。
     */
    Q_INVOKABLE void setZifInputPath(const QString &path)
    {
        zifInputPath = path;
        zifInputPathChanged();
    }

    /**
     * @brief 获取RNAM编码结果的输出路径。
     *
     * @return QString
     */
    Q_INVOKABLE QString getZifOutPath()
    {
        return this->zifOutPath;
    }

    /**
     * @brief 设置RNAM编码结果的输出路径。
     *
     * @param path QString 一个QUrl字符串，表示图片位置。
     * @return void
     */
    Q_INVOKABLE void setZifOutPath(const QString &path)
    {
        this->zifOutPath = path;
    }

    /**** Main modules ****/

    /**
     * @brief 将输入图片加载到内存中。
     *
     * @return void
     */
    Q_INVOKABLE void loadImage();

    /**
     * @brief 启动编码。需要先载入图片。
     *
     * @return void
     */
    Q_INVOKABLE void encode(int sigma, int method);

    /**
     * @brief 启动解码。需要先编码。
     *
     * @return void
     */
    Q_INVOKABLE void decode();

    /**
     * @brief 将输入的原图像显示到指定的QImageMemory对象
     *
     * @param img QImageMemory对象
     * @return void
     */
    Q_INVOKABLE void displaySource(QImageMemory *img)
    {
        img->setDataMat(source_img);
        img->update();
    }

    /**
     * @brief 将编码图像的解码结果显示到指定的QImageMemory对象
     *
     * @param img 指定的QImageMemory对象
     */
    Q_INVOKABLE void displayResult(QImageMemory *img)
    {
        img->setDataMat(decoded_result);
        img->update();
    }

    /**
     * @brief 保存解码后的位图
     *
     *
     */
    Q_INVOKABLE void saveDecodeResult();

    /**
     * @brief 显示解码结果和原图像的差异。
     *
     * @param img 指定的QImageMemory对象
     */
    Q_INVOKABLE void displayDiff(QImageMemory *img);

    /**
     * @brief 显示指定通道的分块信息。
     *
     * @param img 指定的QImageMemory对象
     * @param channel 要显示的通道。[0,1,2] for RGB. 如果是黑白图像，此参数的任何输入都只会返回唯一的通道。
     */
    Q_INVOKABLE void displaySegment(QImageMemory *img, int channel);

    /**
     * @brief 获得编码进度的百分数(整数)。[0-100]
     *
     * @return _int64 编码进度
     */
    Q_INVOKABLE int getProgress()
    {
        if (jobThread != nullptr)
            return (double)img.progress / img.metadata.imgtype / img.metadata.M * 100 / img.metadata.N;
        else
            return 0;
    }

    /**
     * @brief 终止正在进行的编解码工作
     */
    Q_INVOKABLE void terminate()
    {
        if (jobThread != nullptr)
        {
            jobThread->terminate();
            jobThread->wait();

            delete jobThread;
            jobThread = nullptr;
        }
    }

    /**
     * @brief 将编码结果写入符合标准的ZIF文件。
     *
     * @return 编码是否成功。
     */
    Q_INVOKABLE void writeZif();

    /**
     * @brief 从符合标准的ZIF文件中读取编码结果。
     *
     * @return 读取状态。
     */
    Q_INVOKABLE void readZif();

    /**
     * @brief 获得指定通道的相关统计信息。
     * @param channel 选择要读取统计信息的通道。{0,1,2}
     * @param target 输出目标。
     */
    Q_INVOKABLE void loadStat(int channel, RNAM_Stat_t *target);

    /**
     * @brief 将已经读入的原图像转换为灰度图像。
     */
    Q_INVOKABLE void convertSourceToGray();

    /**
     * @brief 将指定的Mat对象读入到source_img.
     * @param mat 输入Mat对象
     */
    Q_INVOKABLE void loadSourceFromMat(const cv::Mat &mat)
    {
        source_img = mat;
        height = source_img.rows;
        width = source_img.cols;

        emit heightChanged();
        emit widthChanged();

        sourceImagePath = "";
        emit sourceImagePathChanged();
    }

    /**
     * @brief 为非正方形的图像添加黑边
     */
    Q_INVOKABLE void addPadding();

    /**
     * @brief 将非正方形输入直接裁剪为正方形。
     */
    Q_INVOKABLE void trimSource();
};

#endif // WRAPPERRNAM_H
