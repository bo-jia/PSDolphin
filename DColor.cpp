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
