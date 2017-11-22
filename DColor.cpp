//
//  DColor.cpp
//  PSDolphin
//
//  Created by Felix on 15/11/2017.
//  Copyright © 2017 Ocean. All rights reserved.
//

#include "DColor.hpp"
#include "opencv2/opencv.hpp"


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
        
        srcVec[i].convertTo(tmp, CV_32FC1);
        
        cv::subtract(tmp, minVal, tmp);
        cv::multiply(tmp, 255.0 / (maxVal-minVal), tmp);

        tmp.setTo(0, srcVec[i] < minVal);
        tmp.setTo(255, srcVec[i] >= maxVal);
        
        tmp.convertTo(srcVec[i], CV_8UC1);
    }
    
    cv::merge(srcVec, src);
}
