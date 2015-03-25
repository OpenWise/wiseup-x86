/*
 * Author: Yevgeniy Kiveisha <lodmilak@gmail.com>
 * Copyright (c) 2014 OpenWise.
 */

#pragma once

#include <iostream>
#include <pthread.h>
#include <vector>
#include <stdlib.h>

#include "rfComm.h"
#include "ipc.h"
#include "commonMethods.h"

typedef struct {
	sensor_info_t 			sensor;
	rfcomm_data 			packet;
	uint16_t				timestamp;	
} nrf_action_task;

class nrfActionTaskMng {
public:
	nrfActionTaskMng (uint64_t interval);
	~nrfActionTaskMng ();

	void apiAddTask (sensor_info_t sensor, rfcomm_data* packet);
	void apiRemoveTask (long long sensorAddress);

	bool start ();
	void stop ();

	bool				m_isWorking;
	sync_context_t		m_lock;
	uint64_t			m_interval;
	vector<nrf_action_task> m_tasks;

private:
	pthread_t       	m_workerTaskExecuter;
};
