#ifndef WRAPPER_RNAM_CPP
#define WRAPPER_RNAM_CPP

#include "WrapperRNAM.hpp"
#include "QUtils.hpp"
#include "RNAMWorker.hpp"

Q_INVOKABLE void WrapperRNAM::loadImage()
{
    using namespace cv;

    if (jobThread != nullptr)
    {
        jobThread->terminate();
        jobThread->wait();
        delete jobThread;
        jobThread = nullptr;
    }

    using std::string;

    if (sourceImagePath.isEmpty())
    {
        emit error(ERRORNO(ERR_READ_FILE));
        return;
    }

    string path = sourceImagePath.toLocal8Bit().data();

    source_img = imread(QUtils::urlStrip(path));
    if (source_img.empty())
    {
        emit error(ERRORNO(ERR_READ_FILE));
        return;
    }

    bool is_gray = (QUtils::ScanImgType(source_img) >= 1);

    if (is_gray)
    {
        Mat gray;
        cv::cvtColor(source_img, gray, cv::COLOR_BGR2GRAY);
        source_img = gray;
    }

    img.data.clear();

    height = source_img.rows;
    width = source_img.cols;

    emit heightChanged();
    emit widthChanged();
}

/**
 * @brief Encode a image in new thread
 *
 * @param sigma _sigma value for RNAM
 */
void WrapperRNAM::encode(int sigma, int method)
{
    if (source_img.empty())
    {
        emit error(ERRORNO_EX(RNAM, ERR_NO_SOURCE));
        return;
    }

    if (source_img.rows != source_img.cols)
    {
        emit error(ERRORNO_EX(RNAM, ERR_INVALID_IMAGE_SIZE));
        return;
    }

    if (jobThread != nullptr)
    {
        emit error(ERRORNO_EX(RNAM, ERR_BUSY));
        return;
    }

    emit encodeStarted();

    img.progress = 0;
    jobThread = new RNAMEncodeWorker(&img, source_img, sigma, method);
    connect(reinterpret_cast<RNAMEncodeWorker *>(jobThread), &RNAMEncodeWorker::finished, this,
            &WrapperRNAM::onEncodeFinished); // connect befor start to avoid "Unknown Signal" exception
    reinterpret_cast<RNAMEncodeWorker *>(jobThread)->start();
}

/**
 * @brief Decode a image in new thread
 *
 */
void WrapperRNAM::decode()
{
    if (jobThread != nullptr)
    {
        emit error(ERRORNO_EX(RNAM, ERR_BUSY));
        return;
    }

    if (source_img.empty() and img.data.empty())
    {
        emit error(ERRORNO_EX(RNAM, ERR_NO_SOURCE));
        return;
    }

    if (img.data.empty())
    {
        emit error(ERRORNO_EX(RNAM, ERR_FORGET_ENCODE));
        return;
    }

    jobThread = new RNAMDecodeWorker(&img, decoded_result);
    connect(reinterpret_cast<RNAMDecodeWorker *>(jobThread), &RNAMDecodeWorker::finished, this,
            &WrapperRNAM::onDecodeFinished);

    jobThread->start();
}

Q_INVOKABLE void WrapperRNAM::displayDiff(QImageMemory *qimg)
{
    if (source_img.empty())
    {
        emit error(ERRORNO_EX(RNAM, ERR_NO_SOURCE));
        return;
    }

    if (decoded_result.empty())
    {
        emit error(ERRORNO_EX(RNAM, ERR_FORGET_DECODE));
        return;
    }

    Mat diff;

    QUtils::ImgDiffVisualizer(source_img, decoded_result, diff);

    qimg->setDataMat(diff);
    qimg->update();
}

Q_INVOKABLE void WrapperRNAM::displaySegment(QImageMemory *qimg, int channel)
{
    if (img.data.empty())
    {
        emit error(ERRORNO_EX(RNAM, ERR_FORGET_ENCODE));
        return;
    }

    Mat seg;

    if (img.metadata.imgtype == 1)
        channel = 0;

    img.getSegments(channel, seg, _visualColor1, _visualColor2);

    qimg->setDataMat(seg);
    qimg->update();
}

Q_INVOKABLE void WrapperRNAM::writeZif()
{
    if (img.data.empty())
    {
        emit error(ERRORNO_EX(RNAM, ERR_FORGET_ENCODE));
        return;
    }
    img.generateStat();

    if (zifOutPath.isEmpty())
    {
        emit error(ERRORNO(ERR_WRITE_FILE));
        return;
    }

    if (jobThread != nullptr)
    {
        emit error(ERRORNO_EX(RNAM, ERR_BUSY));
        return;
    }

    ZIF::Coder::OutputImplCB_t writer = [this](std::ofstream &out, uint32_t &len) {
        struct rawRec
        {
            int pos[4];
            uchar color[4];
        };

        len = 0;
        using namespace std;

        img.generateStat();

        // Write metadata
        out.write((char *)(&img.metadata), sizeof(RNAMImage::Metadata));
        len += sizeof(RNAMImage::Metadata);

        // Write image data
        for (int i = 0; i < img.data.size(); i++)
        {
            out.write((char *)(&img.data[i].stat), sizeof(RNAMImage::Stat));
            len += sizeof(RNAMImage::Stat);

            for (int j = 0; j < img.data[i].stat.blockCount; j++)
            {
                rawRec rec;
                rec.pos[0] = img.data[i].colorList[j].getLt().x;
                rec.pos[1] = img.data[i].colorList[j].getLt().y;
                rec.pos[2] = img.data[i].colorList[j].getRb().x;
                rec.pos[3] = img.data[i].colorList[j].getRb().y;

                Scalar tmp = img.data[i].colorList[j].getGrey();
                for (int i = 0; i < 4; i++)
                    rec.color[i] = tmp[i];

                out.write((char *)(&rec), sizeof(rawRec));
            }

            len += sizeof(rawRec) * img.data[i].colorList.size();
            cout << "Wrote " << img.data[i].colorList.size() << " blocks at channel #" << i << endl;

            auto tmp = img.data[i].coordinateList.size();
            out.write((char *)(&tmp), sizeof(tmp));
            for (int j = 0; j < img.data[i].coordinateList.size(); j++)
                out.write((char *)(&img.data[i].coordinateList.data()[j]), sizeof(char));
            len += sizeof(tmp) + sizeof(char) * img.data[i].coordinateList.size();
        }

        return 0;
    };

    ZIF::Coder coder(ZIF_CODER_NO_INPUT, writer, RNAM_ENCODE_ZIF);
    auto ret = coder.writeFile(QUtils::urlStrip(zifOutPath.toLocal8Bit().data()));

    if (ret)
        emit error(ret);
    return;
}

Q_INVOKABLE void WrapperRNAM::readZif()
{
    if (zifInputPath.isEmpty())
    {
        emit error(ERRORNO(ERR_READ_FILE));
        return;
    }

    if (jobThread != nullptr)
    {
        emit error(ERRORNO_EX(RNAM, ERR_BUSY));
        return;
    }

    ZIF::Coder::InputImplCB_t reader = [this](std::ifstream &in, uint32_t len) {
        struct rawRec
        {
            int pos[4];
            uchar color[4];
        };
        // Clear first
        img.data.clear();

        // Read metadata
        in.read((char *)(&img.metadata), sizeof(RNAMImage::Metadata));

        // Read image data
        for (int i = 0; i < img.metadata.imgtype; i++)
        {
            RNAMImage::Channel chan;
            in.read((char *)(&chan.stat), sizeof(RNAMImage::Stat));

            for (int j = 0; j < chan.stat.blockCount; j++)
            {
                rawRec rec;
                Color c;

                in.read((char *)(&rec), sizeof(rawRec));

                c.setLt(Point(rec.pos[0], rec.pos[1]));
                c.setRb(Point(rec.pos[2], rec.pos[3]));

                Scalar tmp;
                for (int i = 0; i < 4; i++)
                    tmp[i] = rec.color[i];

                c.setGrey(tmp);

                chan.colorList.push_back(c);
            }

            size_t tmp;
            in.read((char *)(&tmp), sizeof(tmp));

            for (int j = 0; j < tmp; j++)
            {
                char tmp2;
                in.read((char *)(&tmp2), sizeof(char));
                chan.coordinateList.push_back(tmp2);
            }
            img.data.push_back(chan);
        }

        return 0;
    };

    ZIF::Coder coder(reader, ZIF_CODER_NO_OUTPUT, RNAM_ENCODE_ZIF);
    int ret = coder.readFile(QUtils::urlStrip(zifInputPath.toLocal8Bit().data()));

    height = img.metadata.M;
    width = img.metadata.N;
    emit heightChanged();
    emit widthChanged();

    if (ret)
        emit error(ret);
    return;
}

Q_INVOKABLE void WrapperRNAM::loadStat(int channel, RNAM_Stat_t *target)
{
    if (img.data.empty())
    {
        emit error(ERRORNO_EX(RNAM, ERR_FORGET_ENCODE));
        return;
    }

    if (img.metadata.imgtype == 1)
        channel = 0;

    auto t = img.GetStat(channel);
    target->setPSNR(t.psnr);
    target->setBPP(t.bpp);
    target->setCR(t.cr);
    target->setBlockCount(t.blockCount);
}

Q_INVOKABLE void WrapperRNAM::convertSourceToGray()
{
    if (source_img.empty())
    {
        emit error(ERRORNO_EX(RNAM, ERR_NO_SOURCE));
        return;
    }

    if (source_img.channels() != 3)
    {
        return;
    }

    Mat temp;
    cv::cvtColor(source_img, temp, cv::COLOR_BGR2GRAY);
    source_img = temp;
}

Q_INVOKABLE void WrapperRNAM::addPadding()
{
    if (source_img.empty())
    {
        emit error(ERRORNO_EX(RNAM, ERR_NO_SOURCE));
        return;
    }
    if (source_img.rows == source_img.cols)
        return;

    int dimension = std::max(source_img.cols, source_img.rows);
    Mat padded = Mat::zeros(dimension, dimension, source_img.type());
    source_img.copyTo(padded(cv::Rect(0, 0, source_img.cols, source_img.rows)));
    source_img = padded;

    height = source_img.rows;
    width = source_img.cols;

    emit heightChanged();
    emit widthChanged();
}

Q_INVOKABLE void WrapperRNAM::trimSource()
{
    using namespace cv;

    if (source_img.empty())
    {
        emit error(ERRORNO_EX(RNAM, ERR_NO_SOURCE));
        return;
    }
    if (source_img.rows == source_img.cols)
        return;

    int dimension = std::min(source_img.cols, source_img.rows);
    Mat trimmed = Mat::zeros(dimension, dimension, source_img.type());
    source_img(cv::Rect(0, 0, dimension, dimension)).copyTo(trimmed);
    source_img = trimmed;

    height = source_img.rows;
    width = source_img.cols;

    emit heightChanged();
    emit widthChanged();
}

Q_INVOKABLE void WrapperRNAM::saveDecodeResult()
{
    if (decoded_result.empty())
    {
        emit error(ERRORNO_EX(RNAM, ERR_FORGET_DECODE));
        return;
    }
    if (resultSavePath.isEmpty())
    {
        emit error(ERRORNO(ERR_WRITE_FILE));
        return;
    }

    if (QUtils::imageSuffixChecker(resultSavePath) != 1)
    {
        emit error(ERRORNO(ERR_WRITE_FILE));
        return;
    }

    cv::imwrite(QUtils::urlStrip(resultSavePath.toLocal8Bit().data()), decoded_result);
    return;
}

#endif