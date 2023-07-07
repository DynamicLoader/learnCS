/**
 * @file QImageMemory.hpp
 * @author LuYi (you@domain.com)
 * @brief QML Element to display opencv::Mat directly
 * @version 0.1
 * @date 2023-05-16
 *
 * @copyright Copyright (c) 2023
 *
 * Reference:
 *  https://blog.csdn.net/kenfan1647/article/details/122018805
 */

#ifndef QIMAGEMEMORY_HPP
#define QIMAGEMEMORY_HPP

#include <iostream>

#include <QQuickPaintedItem>
#include <QPixmap>
#include <QPainter>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

/**
 * @brief Directly show image from memory (i.e. cv::Mat)
 * 
 */
class QImageMemory : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(uint16_t ImgHeight READ getImgHeight NOTIFY ImgHeightChanged)
    Q_PROPERTY(uint16_t ImgWidth READ getImgWidth NOTIFY ImgWidthChanged)
    Q_PROPERTY(cv::Mat DataMat READ getDataMat WRITE setDataMat NOTIFY DataMatChanged)

  public:
    QImageMemory(QQuickItem *parent = 0) : QQuickPaintedItem(parent)
    {
    }

    Q_INVOKABLE QString getName()
    {
        return "QImageMemory 1.0";
    }

    Q_INVOKABLE void setEmpty(){
        this->setDataMat(cv::Mat());
    }

    cv::Mat getDataMat(){
        return this->_mat;
    }

    void setDataMat(const cv::Mat &mat)
    {
//        if (mat.empty())
//            return;

        _mat = mat;
        emit DataMatChanged();

        emit ImgHeightChanged();
        emit ImgWidthChanged();
    }

    void paint(QPainter *painter) override
    {
        painter->setRenderHints(QPainter::Antialiasing, true);
        auto rect = boundingRect().toRect();
        painter->drawPixmap(rect, matToPixmap(rect.size(), _mat));
    }

    // Property impl
    uint16_t getImgHeight()
    {
        return this->_mat.rows;
    }

    uint16_t getImgWidth()
    {
        return this->_mat.cols;
    }

  signals:
    void ImgHeightChanged();
    void ImgWidthChanged();
    void DataMatChanged();

  private:
    cv::Mat _mat;

    static QPixmap matToPixmap(const QSize &size, const cv::Mat &mat)
    {
        if (mat.empty())
            return QPixmap();

        auto img = QImage((const unsigned char *)(mat.data), mat.cols, mat.rows, mat.cols * mat.channels(),
                          (mat.channels() == 1 ? QImage::Format_Grayscale8 : QImage::Format_BGR888));
        return QPixmap::fromImage(img.scaled(size));
    }
};

#endif
