/**
 * @file WrapperTNAM.hpp
 * @author LuYi
 * @brief Wrapper of TNAM algorithm for QML environment
 * @version 0.1
 * @date 2023-05-15
 *
 * Reference:
 *  https://blog.csdn.net/fuhanghang/article/details/125657372
 *
 */

#ifndef WRAPPERTNAM_H
#define WRAPPERTNAM_H

#include <cmath>

#include <QObject>
#include <QThread>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "ErrorNo.h"
#include "QmlExport.hpp"
#include "TNAM/TNAM.h"
#include "QImageMemory.hpp"
#include "ZIF.hpp"

#define WrapperTNAMVersion "1.0"

/**
 * @brief Namespace of implementations of WrapperTNAM
 */
namespace WrapperTNAM_impl
{

/**
 * @brief TNAM file sections for IO
 *
 */
struct meta_t
{
    uint16_t N = 0;
    uint16_t TriNum = 0;
    uint16_t PointNum = 0;
    uint16_t LineNum = 0;
    uint16_t actualH = 0;
    uint16_t actualW = 0;

    uint32_t _reserved[3] = {0};

    /* Followed by Q_tri and Q data*/
};

/**
 * @brief Data item type
 *
 */
struct dataitem_t
{
    uint16_t W : 11;
    uint16_t X_L5 : 5;
    uint16_t X_H6 : 6;
    uint16_t Y_L10 : 10;
    uint16_t Y_H1 : 1;
    uint16_t Z : 11;
    uint16_t flag : 1;
    // 45 bits, as 6 bytes
};

/**
 * @brief Typedef for TNAM coder data structure base
 *
 */
struct coder_base_t
{
    double n = 0;
    uint16_t line_num = 0;
    uint16_t num_of_total_triangular = 0;
    uint16_t point_num = 0;
    uint16_t actualH = 0;
    uint16_t actualW = 0;
};

/**
 * @brief Typedef for TNAM encoded data
 */
struct encoded_t : coder_base_t
{
    int16_t Q_tri[327675] = {0};
    int16_t Q[262144] = {0};
};

/**
 * @brief Typedef for TNAM decoded data
 *
 */
struct decoded_t : coder_base_t
{
    cv::Mat restructed_img;
};

class WrapperTNAM;

/**
 * @brief Encoder thread runner, overloading QThread::run and can be run in a single thread
 */
class EncoderThreadRunner : public QThread
{
    Q_OBJECT
  public:
    EncoderThreadRunner(WrapperTNAM *parent, cv::Mat &mat) : _parent(parent), _data(mat)
    {
    }

  signals:
    void finished(int ret);

  protected:
    void run() override;

  private:
    WrapperTNAM *_parent = nullptr;
    cv::Mat _data;
};

/**
 * @brief Decoder thread runner, overloading QThread::run and can be run in a single thread
 */
class DecoderThreadRunner : public QThread
{
    Q_OBJECT
  public:
    DecoderThreadRunner(WrapperTNAM *parent, encoded_t &enc) : _parent(parent), _data(enc)
    {
    }

  signals:
    void finished(int ret);

  protected:
    void run() override;

  private:
    WrapperTNAM *_parent = nullptr;
    encoded_t _data;
};

/**
 * @brief Wrapper of TNAM matlab generated code, to be exposed to QML environment
 * @note This class should be register with qml!
 *  @warning Images that are bigger than 2048 pixels on any edge will not be saved correctly.
 * This is due to the data structure of TNAM specified ZIF, which have only 11 bits for the position for pixels to
 * reduce space requirement. Actually, it's not supposed to encode such a big image since the encoder will be very slow.
 */
class WrapperTNAM : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl inputFile READ getInFile WRITE setInFile NOTIFY inFileChanged)
    Q_PROPERTY(QUrl outputFile READ getOutFile WRITE setOutFile NOTIFY outFileChanged)
    Q_PROPERTY(double binRate READ getBinRate WRITE setBinRate NOTIFY binRateChanged)

  protected:
    friend class WrapperTNAM_impl::EncoderThreadRunner;
    friend class WrapperTNAM_impl::DecoderThreadRunner;

  public:
    explicit WrapperTNAM(QObject *parent = nullptr) : QObject(parent)
    {
    }

    ~WrapperTNAM()
    {
        this->terminate();
        if (_encRunner)
            delete _encRunner;
        if (_decRunner)
            delete _decRunner;
    }

    /**
     * @brief Get the name of the module
     */
    Q_INVOKABLE QString getName()
    {
        return QString("WrapperTNAM " WrapperTNAMVersion);
    }

    /**
     * @brief Get the description of the module
     */
    Q_INVOKABLE QString getDescription()
    {
        return "Trianglur NAM is a new type of image division method suggested by Team ZYP.";
    }

    /**
     * @brief Start TNAM encoding
     *
     * @param loopMode If set to true, take this->_dec.restructed_img as input; otherwise take input from file with path
     * _inFp
     * @param sourceMat Pass in a cv::Mat that contains the source image. Can be ignored.
     */
    Q_INVOKABLE void startEncode(bool loopMode = false, const cv::Mat &sourceMat = cv::Mat())
    {
        if (_encRunner)
        {
            emit encodeFinished(ERRORNO_EX(TNAM,ERR_BUSY)); // Already running
            return;
        }

        cv::Mat mt;
        if (loopMode)
        {
            mt = this->_dec.restructed_img;
        }
        else
        {
            if (!sourceMat.empty())
            {
                mt = sourceMat;
            }
            else
            {
                mt = cv::imread(this->_inFp.toLocal8Bit().data(), cv::IMREAD_GRAYSCALE);
                if (!mt.data)
                {
                    emit encodeFinished(ERRORNO_EX(TNAM,ERR_EMPTY_INPUT)); // Cannot read image: no such file or unsupported format
                    return;
                }
            }
        }

        if(mt.empty()){
            emit encodeFinished(ERRORNO_EX(TNAM,ERR_EMPTY_INPUT));
            return;
        }

        // Get the binary format image to get competitable with TNAM
        cv::Mat dst;
        cv::threshold(mt, dst, this->_binRate * 255, 1, cv::THRESH_BINARY);
        
            
        _encRunner = new WrapperTNAM_impl::EncoderThreadRunner(this, dst);
        connect(_encRunner, WrapperTNAM_impl::EncoderThreadRunner::finished, this, WrapperTNAM::_onEncodeFinished);
        _encRunner->start();
    }

    /**
     * @brief Start TNAM decoding
     *
     * @param loopMode If set to true, take this->_enc as input; otherwise take input from file with path _inFp
     * @return Q_INVOKABLE
     */
    Q_INVOKABLE void startDecode(bool loopMode = false)
    {
        if (_decRunner)
        {
            emit decodeFinished(ERRORNO_EX(TNAM,ERR_BUSY));
            return;
        }

        WrapperTNAM_impl::encoded_t data;

        if (loopMode)
        {
            data = this->_enc;
        }
        else
        {
            ZIF::Coder coder(
                [&data](std::ifstream &is, uint32_t len) -> int {
                    meta_t meta;
                    is.read((char *)&meta, sizeof(meta_t));
                    if (!is.good())
                        return -1;
                    data.line_num = meta.LineNum;
                    data.num_of_total_triangular = meta.TriNum;
                    data.point_num = meta.PointNum;
                    data.n = meta.N;
                    data.actualH = meta.actualH;
                    data.actualW = meta.actualW;
                    for (uint16_t i = 0; i < meta.TriNum; ++i)
                    {
                        dataitem_t tmp = {0};
                        is.read((char *)&tmp, sizeof(dataitem_t));
                        if (!is.good())
                            return -1;
                        data.Q_tri[0 + i] = tmp.flag;
                        data.Q_tri[65535 * 1 + i] = tmp.W;
                        data.Q_tri[65535 * 2 + i] = (tmp.X_H6 << 5) | tmp.X_L5;
                        data.Q_tri[65535 * 3 + i] = (tmp.Y_H1 << 10) | tmp.Y_L10;
                        data.Q_tri[65535 * 4 + i] = tmp.Z;
                    }
                    for (uint16_t i = 0; i < meta.LineNum + meta.PointNum; ++i)
                    {
                        dataitem_t tmp = {0};
                        is.read((char *)&tmp, sizeof(dataitem_t));
                        if (!is.good())
                            return -1;
                        data.Q[65535 * 0 + i] = tmp.W;
                        data.Q[65535 * 1 + i] = (tmp.X_H6 << 5) | tmp.X_L5;
                        data.Q[65535 * 2 + i] = (tmp.Y_H1 << 10) | tmp.Y_L10;
                        data.Q[65535 * 3 + i] = tmp.Z;
                    }
                    return 0;
                },
                ZIF_CODER_NO_OUTPUT, TNAM_ENCODE_ZIF);

            auto ret = coder.readFile(this->_inFp.toLocal8Bit().data());
            if (ret != 0)
            {
                emit decodeFinished(ret);
                return;
            }
        }

        _decRunner = new WrapperTNAM_impl::DecoderThreadRunner(this, data);
        connect(_decRunner, WrapperTNAM_impl::DecoderThreadRunner::finished, this, WrapperTNAM::_onDecodeFinished);
        _decRunner->start();
    }

    /**
     * @brief Save encoded data to file specified in _outFp
     *
     * @return int 0 if success
     */
    Q_INVOKABLE int saveEncoded()
    {
        ZIF::Coder coder(
            ZIF_CODER_NO_INPUT,
            [this](std::ofstream &os, uint32_t &len) -> int {
                meta_t meta;
                meta.LineNum = this->_enc.line_num;
                meta.PointNum = this->_enc.point_num;
                meta.TriNum = this->_enc.num_of_total_triangular;
                meta.N = this->_enc.n;
                meta.actualH = this->_enc.actualH;
                meta.actualW = this->_enc.actualW;

                os.write((char *)&meta, sizeof(meta_t));
                if (!os.good())
                    return ERRORNO(ERR_WRITE_FILE);
                len += sizeof(meta_t);
                for (uint16_t i = 0; i < this->_enc.num_of_total_triangular; ++i)
                {
                    dataitem_t data = {0};
                    data.flag = this->_enc.Q_tri[65535 * 0 + i] & 0x1;
                    data.W = this->_enc.Q_tri[65535 * 1 + i];
                    data.X_L5 = (uint16_t)this->_enc.Q_tri[65535 * 2 + i] & 0x1F;
                    data.X_H6 = ((uint16_t)this->_enc.Q_tri[65535 * 2 + i] >> 5) & 0x3F;
                    data.Y_L10 = (uint16_t)this->_enc.Q_tri[65535 * 3 + i] & 0x3FF;
                    data.Y_H1 = ((uint16_t)this->_enc.Q_tri[65535 * 3 + i] >> 10) & 0x1;
                    data.Z = this->_enc.Q_tri[65535 * 4 + i];
                    os.write((char *)&data, sizeof(dataitem_t));
                    if (!os.good())
                        return ERRORNO(ERR_WRITE_FILE);
                }
                for (uint16_t i = 0; i < this->_enc.point_num + this->_enc.line_num; ++i)
                {
                    dataitem_t data = {0};
                    data.W = this->_enc.Q[65535 * 0 + i];
                    data.X_L5 = (uint16_t)this->_enc.Q[65535 * 1 + i] & 0x1F;
                    data.X_H6 = ((uint16_t)this->_enc.Q[65535 * 1 + i] >> 5) & 0x3F;
                    data.Y_L10 = (uint16_t)this->_enc.Q[65535 * 2 + i] & 0x3FF;
                    data.Y_H1 = ((uint16_t)this->_enc.Q[65535 * 2 + i] >> 10) & 0x1;
                    data.Z = this->_enc.Q[65535 * 3 + i];
                    os.write((char *)&data, sizeof(dataitem_t));
                    if (!os.good())
                        return ERRORNO(ERR_WRITE_FILE);
                }
                len += (this->_enc.num_of_total_triangular + this->_enc.line_num + this->_enc.point_num) *
                       sizeof(dataitem_t);
                return 0;
            },
            TNAM_ENCODE_ZIF);
        auto ret = coder.writeFile(this->_outFp.toLocal8Bit().data());
        return ret;
    }

    /**
     * @brief Save decoded image
     *
     * @return int Error code
     */
    Q_INVOKABLE int saveDecoded()
    {
        if (this->_dec.restructed_img.empty())
            return -1;
        try
        {
            return (cv::imwrite(this->_outFp.toLocal8Bit().data(), this->_dec.restructed_img) == true? 0 : -1);
        }
        catch (cv::Exception ex)
        {
            std::cerr << "WrapperTNAM::saveDecoded throw an exception:" << ex.what() << std::endl;
            return -1;
        }
    }

    /**
     * @brief Display restructed image to QImageMemory object
     *
     * @param img QImageMemory object to display on
     */
    Q_INVOKABLE void displayTo(QImageMemory *img)
    {
        img->setDataMat(_dec.restructed_img);
        img->update();
    }

    /**
     * @brief Get encode progress
     *
     * @return double Value of progress ranging from 0 to 1
     */
    Q_INVOKABLE double getEncodeProgress()
    {
        if (this->_encRunner == nullptr || this->_enc.n <= 0)
            return 0.0;
        return this->_algo.getEncodeProgress() / this->_enc.n;
    }

    /**
     * @brief Get decode progress
     * 
     * @return double Value of progress ranging from 0 to 1
     */
    Q_INVOKABLE double getDecodeProgress()
    {
        if (this->_decRunner == nullptr || this->_dec.n <= 0)
            return 0.0;
        return this->_algo.getDecodeProgress() / this->_dec.num_of_total_triangular;
    }

    /**
     * @brief Get encode stat
     *
     * @param ret Pointer to TNAM_Stat object
     */
    Q_INVOKABLE void getEncodeStat(QmlExports::TNAM_Stat_t *ret)
    {
        ret->setLineNum(this->_enc.line_num);
        ret->setPointNum(this->_enc.point_num);
        ret->setTriNum(this->_enc.num_of_total_triangular);
    }

    /**
     * @brief Get decode stat
     *
     * @param ret Pointer to TNAM_Stat object
     */
    Q_INVOKABLE void getDecodeStat(QmlExports::TNAM_Stat_t *ret)
    {
        ret->setLineNum(this->_dec.line_num);
        ret->setPointNum(this->_dec.point_num);
        ret->setTriNum(this->_dec.num_of_total_triangular);
    }


    /**
     * @brief Terminate any job of TNAM coder
     * 
     * @return int 0 if success
     */
    Q_INVOKABLE int terminate(){
        if(this->_encRunner && this->_encRunner->isRunning()){
            this->_encRunner->terminate();
            this->_encRunner->wait();
            if(!this->_encRunner->isFinished())
                return -1;
        }
        if(this->_decRunner && this->_decRunner->isRunning()){
            this->_decRunner->terminate();
            this->_decRunner->wait();
            if(!this->_decRunner->isFinished())
                return -1;
        }
        return 0;

    }

    /**
     * @brief Reset the coder's encode and decode data
     */
    Q_INVOKABLE void reset(){
        this->_enc = encoded_t{};
        this->_dec = decoded_t{};
    }

    // =============== Properties implement ===========

    QUrl getInFile()
    {
        return QUrl::fromLocalFile(this->_inFp);
    }
    void setInFile(const QUrl &path)
    {
        this->_inFp = path.toLocalFile();
        emit inFileChanged();
    }

    QUrl getOutFile()
    {
        return QUrl::fromLocalFile(this->_outFp);
    }
    void setOutFile(const QUrl &path)
    {
        this->_outFp = path.toLocalFile();
        emit outFileChanged();
    }

    double getBinRate()
    {
        return this->_binRate;
    }

    void setBinRate(double b)
    {
        if (b > 1 || b < 0)
            return;
        this->_binRate = b;
        emit binRateChanged();
    }

  signals:
    // Declaration only
    void encodeFinished(int ret);
    void decodeFinished(int ret);
    void inFileChanged();
    void outFileChanged();
    void binRateChanged();
    void encodeProgressUpdated(); // Never send

  private slots:

    void _onEncodeFinished(int ret)
    {
        if (ret)
            _enc = WrapperTNAM_impl::encoded_t{};
        disconnect(_encRunner, WrapperTNAM_impl::EncoderThreadRunner::finished, this, WrapperTNAM::_onEncodeFinished);
        emit encodeFinished(ret);
        delete _encRunner;
        _encRunner = nullptr;
    }

    void _onDecodeFinished(int ret)
    {
        if (ret)
            _dec = WrapperTNAM_impl::decoded_t{};
        disconnect(_decRunner, WrapperTNAM_impl::DecoderThreadRunner::finished, this, WrapperTNAM::_onDecodeFinished);
        emit decodeFinished(ret);
        delete _decRunner;
        _decRunner = nullptr;
    }

  private:
    TNAM _algo;
    QString _inFp;
    QString _outFp;
    double _binRate = 0.50;
    bool _intsrc = false;

    WrapperTNAM_impl::encoded_t _enc;
    WrapperTNAM_impl::decoded_t _dec;

    WrapperTNAM_impl::EncoderThreadRunner *_encRunner = nullptr;
    WrapperTNAM_impl::DecoderThreadRunner *_decRunner = nullptr;
};

} // namespace WrapperTNAM_impl

// Exports
using WrapperTNAM = WrapperTNAM_impl::WrapperTNAM;

#endif // WRAPPERTNAM_H

#ifdef WITH_IMPL // Avoid multiple defination

namespace WrapperTNAM_impl
{

void EncoderThreadRunner::run()
{
    using namespace cv;

    if (!_parent)
    {
        emit finished(ERRORNO_EX(TNAM, ERR_NO_PARENT));
        return;
    }

    if (_data.rows == 0 || _data.cols == 0)
    {
        emit finished(ERRORNO_EX(TNAM, ERR_EMPTY_INPUT)); // Empty file
        return;
    }

    encoded_t &enc = _parent->_enc;
    enc.n = std::max(_data.rows, _data.cols);
    enc.actualH = _data.rows;
    enc.actualW = _data.cols;

    coder::array<boolean_T, 2U> in;
    in.set_size(enc.n, enc.n);

    for (int idx0 = 0; idx0 < enc.n; idx0++)
    {
        for (int idx1 = 0; idx1 < enc.n; idx1++)
        {
            in[idx1 * in.size(0) + idx0] =
                (idx0 >= _data.rows || idx1 >= _data.cols ? 1 : _data.at<bool>(idx0, idx1)); // Column first
        }
    }

    // Process
    std::cout << enc.n << std::endl;
    _parent->_algo.triangular_raster(enc.n, in, &enc.point_num, &enc.line_num, &enc.num_of_total_triangular, enc.Q_tri,
                                     enc.Q);

    emit finished(0); // No Error
}

void DecoderThreadRunner::run()
{
    if (!this->_parent)
    {
        emit finished(ERRORNO_EX(TNAM, ERR_NO_PARENT));
        return;
    }
    if (this->_data.n == 0)
    {
        emit finished(ERRORNO_EX(TNAM, ERR_EMPTY_INPUT));
        return;
    }

    // Prepare Output
    coder::array<boolean_T, 2> res;
    res.set_size(_data.n, _data.n);
    // Process
    _parent->_algo.triangular_decode(_data.n, _data.Q_tri, _data.Q, _data.num_of_total_triangular, _data.point_num,
                                     _data.line_num, res);
    decoded_t &dec = _parent->_dec;
    (coder_base_t &)dec = (const coder_base_t &)_data;

    cv::Mat resimg(dec.actualH, dec.actualW, CV_8UC1);
    for (int i = 0; i < res.size(0) && i < dec.actualH; ++i)
    {
        for (int j = 0; j < res.size(1) && j < dec.actualW; ++j)
        {
            resimg.at<boolean_T>(i, j) = res[j * res.size(0) + i];
        }
    }

    cv::threshold(resimg, _parent->_dec.restructed_img, 0.5, 255, cv::THRESH_BINARY);
    emit finished(0);
}

} // namespace WrapperTNAM_impl

#endif
