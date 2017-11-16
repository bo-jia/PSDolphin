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

#endif /* DColor_hpp */
