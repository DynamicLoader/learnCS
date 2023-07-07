#pragma once

#include "RNAM/CommonUtil.hpp"
#include <stdint.h>
#include <vector>
#include <atomic>
#include <fstream>

using cv::Mat;

using std::vector;

constexpr int DEFAULT_SIGMA = 10;

/**
 * @brief RNAM图像类。存储NAM分割的图像表示信息和相关的元数据,提供与位图相互转换的方法.
 *
 */
class RNAMImage
{
  public:
    friend class WrapperRNAM;
    struct Stat
    {
        // RNAM image stat
        double bpp;
        double cr;
        double psnr;
        int blockCount;
    };

    struct Channel
    {
        Stat stat;
        // RNAM image data
        vector<Color> colorList;
        vector<char> coordinateList;
    };

    struct Metadata
    {
        int M; // rows
        int N; // cols

        int imgtype; // grayscale or rgb
        int method;  // encode method
    };

    std::atomic<long long> progress;

    RNAMImage()
    {
        progress = 0;
    }

  private:
    vector<Channel> data;
    Metadata metadata;

  private:
    // Helper for GenStat
    void StatGen_1Chan(Channel &chan)
    {
        if (chan.colorList.empty() or chan.coordinateList.empty())
            return;

        chan.stat.bpp = BPP(chan.colorList, metadata.M, metadata.N, chan.coordinateList);
        chan.stat.cr = 8.0 / chan.stat.bpp;
        chan.stat.blockCount = chan.colorList.size();
    }

  public:
    /**
     * @brief Calculate stats of current RNAM image image. Except PSNR. (Orignal image required.)
     *
     */
    void generateStat()
    {
        for (int i = 0; i < data.size(); i++)
            StatGen_1Chan(data[i]);
    }

    /**
     * @brief Calculate stats of current RNAM image image. May take a long time.
     *
     * @param origin Reference image to calculate PSNR. Here, you should pass the original image.
     */
    void generateStat(const Mat &origin);

    /**
     * @brief Get the Stat object
     *
     * @return  A struct containing BPP, CR, PSNR and block count.
     */
    Stat GetStat(int chan) const
    {
        return data.at(chan).stat;
    }

  private:
    bool _is_valid_method(int method)
    {
        return method == HORIZONAL_PRIORITY or method == DIAGONAL_PRIORITY;
    }

  public:
    /**
     * @brief Converts a GRAYSCALE image to RNAM image Channel.
     *
     * @param source a Grayscale image. if source is not in grayscale, this function EXIT.
     * @param target target Channel.
     * @param method Encoding method. {HORIZONAL_PRIORITY | DIAGONAL_PRIORITY}
     * @param sigma
     */
    void bitmapToChan(const Mat &source, Channel &chan, int method = HORIZONAL_PRIORITY, int sigma = 5);

    /**
     * @brief Encode a bitmap into RNAM image image.
     *
     * @param source Source bitmap. RGB supported.
     * @param method Encoding method. {HORIZONAL_PRIORITY | DIAGONAL_PRIORITY}
     * @param sigma tolerance. default = 5. Pass small value for more detail.
     */
    void fromBitmap(const Mat &source, int method = HORIZONAL_PRIORITY, int sigma = DEFAULT_SIGMA);

    /**
     * @brief Decode one grayscale into a grayscale bitmap.
     *
     * @param chan Channel to decode. Accept [0,1,2] for BGR.
     * @param target Target bitmap. Must be grayscale.
     */
    void chanToBitmap(Mat &target, int chan);

    /**
     * @brief Decode RNAM image image into a bitmap. RGB supported.
     *
     * @param target Target Mat, Grayscale or RGB.
     */
    void toBitmap(Mat &target);

    /**
     * @brief Get how a grayscale image is represented by rectangles.
     *
     * @param chan Channel to compute segmentation. [0,1,2] for BGR.
     * @param target Target Mat, RGB.
     */
    void getSegments(int chan, Mat &target, QColor color1, QColor color2);
};