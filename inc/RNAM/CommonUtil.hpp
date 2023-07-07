#ifndef __COMMONUTILS_HPP__
#define __COMMONUTILS_HPP__

#include "RNAM/incHead.h"

#include <cstdlib>
#include <random>

enum
{
    HORIZONAL_PRIORITY = 1,
    DIAGONAL_PRIORITY = 2

    // Maybe more here
};

void mark(Mat &mark, Point lt, Point rb);

void Encode(Mat &R, int height, int width, std::vector<char> &Q);

void Decode(Mat &R, int height, int width, const std::vector<char> &Q);

void RNAMCEncoding(Mat Source, Mat &R, Mat &markMatrix, int M, int N, std::vector<Color> &p, std::vector<char> &q,
                   int sigma, int method, std::atomic<long long> *progress);

void RDHelper(Mat &R, Color p1);

void RNAMCDecoding(Mat &R, int M, int N, std::vector<Color> p, std::vector<char> q);

/**
 * @brief Calculate the PSNR of the image
 *
 * @param source
 * @param result
 * @param M height of image
 * @param n width of image
 * @return double PSNR value
 */
double PSNR(Mat source, Mat result, int M, int N);

/**
 * @brief Calculate the bpp of the image
 *
 * @param p zyp_format color list
 * @param M height of image
 * @param n width of image
 * @param q zyp_defined coordinateList
 * @return double bpp value of the image
 */
double BPP(std::vector<Color> p, int M, int N, std::vector<char> q);

#endif