//
//  DEffect.cpp
//  PSDolphin
//
//  Created by jiabo on 28/11/2017.
//  Copyright © 2017 Ocean. All rights reserved.
//

#include "DEffect.hpp"
#include "opencv2/opencv.hpp"

void PSDGlowFilter(unsigned char* data, int w, int h, int glowRadius, float glowStrength) {
    cv::Mat src(h, w, CV_8UC3, data), gauss, gray, m;
    
    glowRadius = ((glowRadius >> 1) << 1) + 1;
    cv::cvtColor(src, gray, CV_RGB2GRAY);
    cv::GaussianBlur(src, gauss, cv::Size(glowRadius, glowRadius), glowRadius/2.);
    
    cv::multiply(gray, glowStrength, gray);
    cv::cvtColor(gray, m, CV_GRAY2RGB);
    
    cv::multiply(gauss, m, m, 1./255);
    cv::add(m, src, src);
}
