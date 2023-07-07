#include "RNAM/CommonUtil.hpp"
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <string>

#include "RNAMimage.hpp"

void printStat(RNAMImage::Stat &stat)
{
    using namespace std;

    cout << "bpp: " << stat.bpp << endl;
    cout << "cr: " << stat.cr << endl;
    cout << "psnr: " << stat.psnr << endl;
    cout << "blockCount: " << stat.blockCount << endl;
}

int main(int argc, char **argv)
{
    using namespace std;
    using namespace cv;

    int sigma;

    cout << "Program started with " << argc << " arguments" << endl;

    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " <input image>" << endl;
        return -1;
    }

    cout << "input _sigma: ";
    cin >> sigma;

    string input = argv[1];

    Mat RGBtest = imread(input);

    if (RGBtest.empty())
    {
        cout << "Could not open or find the image!\n" << endl;
        return -1;
    }
    cout << "Testing interconversion" << endl;

    RNAMImage test1;
    test1.fromBitmap(RGBtest, DIAGONAL_PRIORITY, sigma);
    Mat out1;
    test1.toBitmap(out1);

    imwrite("out1.png", out1);

    test1.generateStat();

    Mat seg[3];
    char letters[] = "BGR";
    for (int i = 0; i < 3; i++)
    {
        RNAMImage::Stat tmp = test1.GetStat(i);
        printStat(tmp);
        test1.getSegments(i, seg[i], 0, 0);
        imwrite(string("out1_segment_") + letters[i] + ".png", seg[i]);
    }

    return 0;
}