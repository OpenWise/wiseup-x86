/*
 * Author: Yevgeniy Kiveisha <yevgeniy.kiveisha@intel.com>
 * Copyright (c) 2014 Wiseup.
 */

#pragma once

#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include "ipc.h"
#include "rfComm.h"

#define YES 	1
#define NO 		0

typedef struct {
	pthread_cond_t  cond;
	pthread_mutex_t mutex;
} sync_context_t;

typedef struct {
    uint32_t txPacketCount;
    uint32_t rxPacketCount;
    uint8_t  cpuTemperature;
    uint8_t  cpuUsage;
} screen_context;

typedef struct {

} deamon_context;

typedef struct { 
    uint8_t isAvalibale : 1;
    uint8_t isEvent     : 1;
    uint8_t isValueCng  : 1;
    uint8_t reserved    : 5;
} sensor_control_t;

typedef struct { 
    uint16_t            sensorHWValue;
    uint16_t            sensorUIValue;
} sensor_value_t;

typedef struct { 
    long long           sensorAddress;
    /* Saved for later use.
    long long           hubAddress;
    uint8_t             sensorPort;
    uint8_t             sensorType;
    sensor_value_t      value;
    sensor_value_t      backup;
    uint64_t            lastUpdate;
    sensor_control_t    flags;
    uint16_t            updateInterval; */
} sensor_info_t;

typedef struct {
	uint32_t	with_ack 	: 1;
	uint32_t	reserved 	: 31;
} nrf24l01_msg_flag_t;

typedef struct {
    long long           sensorAddress;
    rfcomm_data     	packet;
    uint64_t    		timestamp;
	nrf24l01_msg_flag_t	features;		
} nrf24l01_msg_t;

class CommonMethods {
public:
    static uint64_t getTimestampMillis () {
        struct timeval tv;

        gettimeofday(&tv, NULL);

        return (uint64_t)(1000000 * tv.tv_sec + tv.tv_usec);
    }

    static void printBuffer (char* name, uint8_t* buff, int len) {
    	printf ("%s ", name);
	    for (int i = 0; i < len; i++) {
	        printf ("%x ", buff[i]);
	    }
	    printf ("\n");
	}
};