/*
 * Author: Yevgeniy Kiveisha <yevgeniy.kiveisha@intel.com>
 * Copyright (c) 2014 Intel Corporation.
 */
 
#pragma once
 
#include <unistd.h>
#include <iostream>
#include <sys/time.h>
#include <stdio.h>

class WiseTimer {
    public:
		WiseTimer (uint64_t interval);
		~WiseTimer ();
		
        void setTimer (uint64_t interval) {
            m_interval  = interval;
            m_timestamp = getTimestamp ();
        }
 
        bool checkTimer () {
            if (getTimestamp () - m_timestamp > m_interval) {
                m_timestamp = getTimestamp ();
                return true;
            } else {
                return false;
            }
        }

    private:
        uint64_t getTimestamp () {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            return (uint64_t)(1000000 * tv.tv_sec + tv.tv_usec);
        }

        uint64_t m_interval;
        uint64_t m_timestamp;
};

WiseTimer::WiseTimer (uint64_t interval) {
	m_interval  = interval;
	m_timestamp = getTimestamp ();
}

WiseTimer::~WiseTimer () {

}
