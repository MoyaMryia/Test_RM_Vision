#pragma once

#define _ENEMY 2
#define _FRIENDS 0
//#define USING_YOLO
#define USING_BACKUP
//#define VIDEO


#if !defined(USING_YOLO) && !defined(USING_BACKUP)
    #error "ERROR: At least one of the USING_YOLO and USING_BACKUP should be declaried."
#endif 

#if !defined(_ENEMY) || ((_ENEMY != 2) && (_ENEMY != 0))
    #error "ERROR: _ENEMY not defined as expected."
#endif

#if !defined(_FRIENDS)
    #warning "WARNING: _FRIENDS not defined."
#endif

#include <../include/armor.hpp>
#include "../include/videoReader.hpp"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <string>
#include <numeric>
#include <memory>
#include <opencv2/opencv.hpp>    
#include <cmath>
#define pi 3.1415926535897

