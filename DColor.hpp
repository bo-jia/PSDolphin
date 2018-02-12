//
//  DColor.hpp
//  PSDolphin
//
//  Created by Felix on 15/11/2017.
//  Copyright © 2017 Ocean. All rights reserved.
//

#ifndef DColor_hpp
#define DColor_hpp

#include <stdio.h>


/**
 Adjust the brightness and contrast of image

 @param data image RGB data
 @param w width of image
 @param h height of image
 @param brightness value range [-1, 1]
 @param contrast value range [-1, 1]
 */
extern void PSDAdjustBrightnessAndContrast(unsigned char* data, int w, int h, float brightness = 0, float contrast = 0);


/**
 Auto tone

 @param data image RGB data
 @param w width of image
 @param h height of image
 @param percentage default value is 0.01
 */
extern void PSDAutoTone(unsigned char* data, int w, int h, float percentage = 0.01);

extern void PSDAutomaticWhiteBalance(unsigned char* data, int w, int h);

#endif /* DColor_hpp */
