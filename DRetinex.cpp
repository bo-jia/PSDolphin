//
//  DRetinex.cpp
//  PSDolphin
//
//  Created by jiabo on 24/11/2017.
//  Copyright © 2017 Ocean. All rights reserved.
//

#include "DRetinex.hpp"
#include "opencv2/opencv.hpp"

void restoreColor(const cv::Mat& src, cv::Mat& dst, float restorationFactor, float colorGain) {
    if (src.channels() < 3) {
        src.copyTo(dst);
        return;
    }
    
    std::vector<cv::Mat> srcVec, tmpVec;
    cv::split(src, srcVec);
    for (int i = 0; i < srcVec.size(); ++i) {
        tmpVec.push_back(cv::Mat());
    }
    
    cv::Mat srcSum;
    cv::add(srcVec[0], srcVec[1], srcSum);
    cv::add(srcVec[2], srcSum, srcSum);
    
    for (int i = 0; i < srcVec.size(); ++i) {
        cv::divide(srcVec[i], srcSum, tmpVec[i], restorationFactor);
        cv::add(tmpVec[i], 1, tmpVec[i]);
        cv::log(tmpVec[i], tmpVec[i]);
        cv::multiply(srcVec[i], tmpVec[i], tmpVec[i], colorGain);
    }
    
    cv::merge(tmpVec, dst);
}

void PSDRetinex(unsigned char* data, int w, int h, float sigma, float gain, float offset) {
    cv::Mat src(h, w, CV_8UC3, data), srcGauss;
    
    cv::Mat srcf, srcfLog, srcfGauss, srcfGaussLog;
    src.convertTo(srcf, CV_32FC(src.channels()));
    cv::log(srcf, srcfLog);
    
    double minval, maxval;
    cv::minMaxLoc(srcfLog, &minval, &maxval);
    
    sigma = MIN(300, sigma);
    int ksize = floor(sigma * 6) / 2;
    ksize = ksize * 2 + 1;
    cv::GaussianBlur(src, srcGauss, cv::Size(ksize, ksize), sigma);
    
    srcGauss.convertTo(srcfGauss, CV_32FC(srcGauss.channels()));
    cv::log(srcfGauss, srcfGaussLog);
    
    cv::subtract(srcfLog, srcfGaussLog, srcfLog);
    
    srcfLog.convertTo(src, CV_8UC(srcfLog.channels()), gain, offset);
}
