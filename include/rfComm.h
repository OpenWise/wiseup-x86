/*
 * Author: Yevgeniy Kiveisha <yevgeniy.kiveisha@intel.com>
 * Copyright (c) 2014 Intel Corporation.
 */

#pragma once

#include <stdint.h>

#define DATA_PACKAGE_SIZE   	    15
#define RFCOMM_VERSION 			    0x1

/* Data type */
#define SENSOR_INFO_DATA_NO_AUTH_TYPE   0x0
#define SENSOR_INFO_DATA_TYPE	        0x1
#define SENSOR_CMD_DATA_TYPE	        0x2
#define DEVICE_PROT_DATA_TYPE	        0x3

/* Sensor command type */
#define SENSOR_CMD_RELAY		    0x1
#define SENSOR_CMD_RELAY_RGB	    0x2

/* Device protocol commands */
#define DEVICE_PROT_CONNECT_REQ     0x1
#define DEVICE_PROT_CONNECT_ADDR    0x2
#define DEVICE_PROT_CONNECT_CHK     0x99

/* Sizes */
#define SENSOR_INFO_DATA_SIZE	    0x5
#define DEVICE_PROT_CONN_DATA_SIZE	0x1
#define SENSOR_CMD_DATA_TYPE_SIZE	0x6

/* Sensors type */
#define TEMPERATURE_SENSOR_TYPE     0x1
#define LUMINANCE_SENSOR_TYPE 	    0x2
#define PIR_SENSOR_TYPE 		    0x3
#define RELAY_SENSOR_TYPE		    0x4
#define RGB_LED_SENSOR_TYPE		    0x5

/* Sender types */
#define SENDER_SENSOR_LOCAL_HUB    	0x1
#define SENDER_SENSOR_WIRELESS_HUB  0x2

struct device_version {
	uint8_t major	:4;
	uint8_t minor	:4;
};

struct rfcomm_sensor_info {
	uint8_t 	sensor_address;
	uint8_t		sensor_type;
	uint16_t	sensor_update_interval;
	uint8_t		sensor_data_len;
};

struct rfcomm_individual_sensor_info {
	uint8_t 	sensor_address[6];
	uint8_t		sensor_type;
	uint16_t	sensor_update_interval;
	uint8_t		sensor_data_len;
};

struct rfcomm_sensor_command {
	uint8_t 	sensor_address;
	uint8_t		command_type;
    uint8_t     command_data[4];
};

struct rfcomm_device_prot {
	uint8_t 	device_cmd;
    uint8_t     device_data[8];
};

struct rfcomm_data {
	struct control { 
		uint8_t is_fragmeneted	: 1;
		uint8_t version			: 3;
		uint8_t is_broadcast	: 1;
		uint8_t is_ack			: 1;
		uint8_t reserved		: 2;
	};
	
	struct data_info {
		uint8_t data_type		: 3;
		uint8_t data_size		: 5;
	};
	
	struct sender_info {
		uint8_t sender_type	    : 5;
		uint8_t reserved		: 3;
	};
	
	union data_package {
		struct _unframeneted {
			uint8_t 	data[DATA_PACKAGE_SIZE];
		} unframeneted;
		
		struct _fragmented {
			uint8_t		frame_number;
			uint8_t		data[DATA_PACKAGE_SIZE - 1];
		} fragmented;
	};
	
	uint8_t			magic_number[2];		// 2  byte
	control 		control_flags;			// 1  byte
	uint8_t			sender[5];				// 5  byte
    uint8_t			target[5];				// 5  byte
	data_info		data_information;		// 1  byte
	sender_info		sender_information;		// 1  byte
	data_package 	data_frame;				// 15 byte
	uint16_t		packet_id;				// 2  byte
};
