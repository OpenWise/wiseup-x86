/*
 * Author: Yevgeniy Kiveisha <yevgeniy.kiveisha@intel.com>
 * Copyright (c) 2014 Intel Corporation.
 */

#pragma once

#include <iostream>

using namespace std;

class FFError {
    public:
        std::string Label;

        FFError( ) { 
            Label = (char *)"Generic Error"; 
        }
        FFError( char *message ) { 
            Label = message; 
        }
        ~FFError() { }
        inline const char* GetMessage (void){ 
            return Label.c_str(); 
        }
};
