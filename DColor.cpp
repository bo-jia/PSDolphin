//
//  DColor.cpp
//  PSDolphin
//
//  Created by Felix on 15/11/2017.
//  Copyright © 2017 Ocean. All rights reserved.
//

#include "DColor.hpp"
#include "opencv2/opencv.hpp"

// https://www.cnblogs.com/Imageshop/archive/2013/04/20/3032062.html
// https://www.csie.ntu.edu.tw/~fuh/personal/ANovelAutomaticWhiteBalanceMethodforDigital.pdf
void PSDAutomaticWhiteBalance(unsigned char* data, int w, int h) {
    
    cv::Mat src(h, w, CV_8UC3, data), scaledSrc;
    
    // Step 1: resize image to 256 for efficiency
    int blockSize  = 48;
    int scaledSize = 256;
    
    int new_w = w, new_h = h;
    if (w > blockSize || h > blockSize) {
        new_w = w * scaledSize / MAX(w, h);
        new_h = h * scaledSize / MAX(w, h);
    }
    new_w = (new_w >> 1) << 1;
    new_h = (new_h >> 1) << 1;
    cv::resize(src, scaledSrc, cv::Size(new_w, new_h));
    
    // convert color space from BGR to YCrCb
    cv::Mat YCrCb;
    cv::cvtColor(scaledSrc, YCrCb, CV_BGR2YCrCb);
    
    std::vector<cv::Mat> YCrCbVec;
    cv::split(YCrCb, YCrCbVec);
    
    /*
     * Step 2: split image into blocks, and find near white region
     *  A. compute the mean values CrM and CbM, and absolute differences CrD and CbD of each block,
     *  B. if CrD and CbD are too small, the region is discarded
     *  C. compute the final CrM, CbM, CrD, CbD by taking average
     *  D. get the near white region that satisfy the following relationships:
     *      |Cb(i,j) - (CbM + CbD * sign(CbM))|     < 1.5 * CbD
     *      |Cr(i,j) - (1.5*CrM + CrD * sign(CrM))| < 1.5 * CrD
     */
    cv::Mat Y = YCrCbVec[0], Cr = YCrCbVec[1], Cb = YCrCbVec[2];
    int dw = MIN(blockSize, scaledSrc.cols), dh = MIN(blockSize, scaledSrc.rows);
    int cn = scaledSrc.cols / dw, rn = scaledSrc.rows / dh;
    
    int count = 0;
    float CrM = 0, CrS = 0, CbM = 0, CbS = 0;
    for (int i = 0; i < rn; ++i) {
        for (int j = 0; j < cn; ++j) {
            cv::Rect rect(j*dw, i*dh, dw, dh);
            cv::Scalar rm, rs, bm, bs;
            cv::meanStdDev(Cr(rect), rm, rs);
            cv::meanStdDev(Cb(rect), bm, bs);
            if (rs[0] > 10 && bs[0] > 10) {
                CrM += rm[0];
                CrS += rs[0];
                CbM += bm[0];
                CbS += bs[0];
                count ++;
            }
        }
    }
    CrM = CrM / count - 128;
    CrS /= count;
    CbM = CbM / count - 128;
    CbS /= count;
    
    cv::Mat temp;
    float Cr0 = 1.5 * CrM + CrS * (CrM < 0 ? -1 : 1);
    cv::subtract(Cr, Cr0 + 128, temp, cv::Mat(), CV_16S);
    temp = cv::abs(temp);
    cv::Mat CrMask = temp < (1.5 * CrS);
    float Cb0 = 1.0 * CbM + CbS * (CbM < 0 ? -1 : 1);
    cv::subtract(Cb, Cb0 + 128, temp, cv::Mat(), CV_16S);
    temp = cv::abs(temp);
    cv::Mat CbMask = temp < (1.5 * CbS);
    cv::bitwise_and(CrMask, CbMask, CbMask);
    
    // Step 3: compute average BGR values of the top 10% of the candidate reference white points in near white region
    double Ymax;
    cv::minMaxLoc(Y, NULL, &Ymax);
    Y.setTo(0, 255 - CbMask);
    cv::Mat t0 = Y.reshape(1, Y.rows * Y.cols), t1;
    cv::sort(t0, t1, CV_SORT_EVERY_COLUMN | CV_SORT_DESCENDING);
    int thresholdValue = t1.at<uchar>(t0.rows * 0.1);
    CbMask.setTo(0, Y < thresholdValue);
    cv::Scalar rgbAvg = cv::mean(scaledSrc, CbMask);
    
    // Step 4: adjust the image
    cv::Scalar rgbGain;
    for (int i = 0; i < 3; ++i) {
        rgbGain[i] = Ymax / rgbAvg[i];
    }
    std::vector<cv::Mat> rgbVec;
    cv::split(src, rgbVec);
    for (int i = 0; i < 3; ++i) {
        cv::multiply(rgbVec[i], rgbGain[i], rgbVec[i]);
    }
    cv::merge(rgbVec, src);
    
}

/**
 Algorithm
 The formula:   y = [x - 127.5 * (1 - b)] * k + 127.5 * (1 + b);
                     x is the input pixel value
                     y is the output pixel value
                     B is brightness, value range is [-1,1]
                     k is used to adjust contrast
                         k = tan( (45*c) / 180 * PI );
                         c is contrast, value range is [-1,1]
 */

void PSDAdjustBrightnessAndContrast(unsigned char* data, int w, int h, float brightness, float contrast) {
    cv::Mat src(h, w, CV_8UC3, data), dst;
    float b = brightness;
    float k = tan((45+44*contrast)/180*CV_PI);
    
    cv::Mat lut(1, 256, CV_8UC1);
    for (int i = 0; i < 256; i++) {
        lut.at<uchar>(i) = MAX(0, MIN(255, (i - 127.5 * (1 - b)) * k + 127.5 * (1 + b)));
    }
    cv::LUT(src, lut, src);
}


extern void PSDAutoTone(unsigned char* data, int w, int h, float percentage) {
    cv::Mat src(h, w, CV_8UC3, data), tmp;
    
    cv::MatND hist;
    int histSize[] = {256};
    float range[] = {0,256};
    const float* ranges[3] = {range};
    int channels[1] = {0};
    
    std::vector<cv::Mat> srcVec;
    cv::split(src, srcVec);
    for (int i = 0; i < src.channels(); ++i) {
        
        // find min and max value
        cv::calcHist(&srcVec[i], 1, channels, cv::Mat(), hist, 1, histSize, ranges, true, false);
        float count = 0, total = srcVec[i].rows * srcVec[i].cols, maxVal = -1, minVal = -1;
        for (int j = 0; j < histSize[0]; ++j) {
            count += hist.at<float>(j);
            if (count > total * percentage && minVal < 0) {
                minVal = j;
            }
            if (count > total * (1-percentage) && maxVal < 0) {
                maxVal = j;
            }
        }
        
        // stretch region
        srcVec[i].convertTo(tmp, CV_32FC1);
        cv::subtract(tmp, minVal, tmp);
        cv::multiply(tmp, 255.0 / (maxVal-minVal), tmp);

        // clamp
        tmp.setTo(0, srcVec[i] < minVal);
        tmp.setTo(255, srcVec[i] >= maxVal);
        
        tmp.convertTo(srcVec[i], CV_8UC1);
    }
    
    cv::merge(srcVec, src);
}
