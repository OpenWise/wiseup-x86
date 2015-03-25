/*
 * Author: Yevgeniy Kiveisha <lodmilak@gmail.com>
 * Copyright (c) 2014 OpenWise.
 */
 
#include <iostream>
#include <ctime>
#include <unistd.h>
#include <stdlib.h>

#include "nrfTaskMng.h"

using namespace std;

nrfActionTaskMng::nrfActionTaskMng (uint64_t interval) {
	m_interval = interval;

	pthread_cond_init  (&m_lock.cond, NULL);
	pthread_mutex_init (&m_lock.mutex, NULL);
}

nrfActionTaskMng::~nrfActionTaskMng () {
	pthread_mutex_destroy (&m_lock.mutex);
    pthread_cond_destroy  (&m_lock.cond);
}

void * 
nrfActionTaskExecuterWorker (void * args) {
	nrfActionTaskMng* 	obj 		 = (nrfActionTaskMng*)args;
	bool				isDelete	 = false;
	std::vector<nrf_action_task>::iterator indexToDelete;
	while (obj->m_isWorking) {
		pthread_mutex_lock (&obj->m_lock.mutex);
		if (obj->m_tasks.size() > 0) {
			for (std::vector<nrf_action_task>::iterator item = obj->m_tasks.begin(); item != obj->m_tasks.end(); ++item) {
				if (CommonMethods::getTimestampMillis() - item->timestamp > obj->m_interval) {
                    // TODO resend packet
                    nrf24l01_msg_t msg;
                    memcpy (&msg.packet, &item->packet, 32);
                    msg.features.with_ack = NO;
                    item->timestamp = CommonMethods::getTimestampMillis();
                    
                    // Send to OUTGOING queue
                    WiseIPC *ipcPacketsOut = new WiseIPC ("/tmp/wiseup/nrf_outgoing_queue");
                    if (ipcPacketsOut->setClient () == SUCCESS) {
                        ipcPacketsOut->setBuffer((uint8_t *)&msg);
                        if (ipcPacketsOut->sendMsg(sizeof (nrf24l01_msg_t)) == false) { 
                        } else {
                            printf ("(WiseClientHandler) [sendSensorCommand] RESEND for  [%d %d %d %d %d]\n", 
                                                                msg.packet.target[0], msg.packet.target[1], 
                                                                msg.packet.target[2], msg.packet.target[3], 
                                                                msg.packet.target[4]);
                        }
                    }

                    delete ipcPacketsOut;
				}
			}
		}
		
		pthread_mutex_unlock (&obj->m_lock.mutex);
		usleep (obj->m_interval);
	}
}

 bool
nrfActionTaskMng::start () {
	m_isWorking = true;

    int error 	= pthread_create(&m_workerTaskExecuter, NULL, nrfActionTaskExecuterWorker, this);
    if (error) {
        return false;
    }

    return true;
}

void
nrfActionTaskMng::stop () {
	m_isWorking = false;
	pthread_cancel (m_workerTaskExecuter);
}

void
nrfActionTaskMng::apiAddTask (sensor_info_t sensor, rfcomm_data* packet) {
	nrf_action_task task;
	task.sensor = sensor;
	task.timestamp = CommonMethods::getTimestampMillis();
	memcpy (&task.packet, packet, sizeof (rfcomm_data));

	printf ("(nrfActionTaskMng) [apiAddTask] ADDING TO RESEND (%lld)\n", sensor.sensorAddress);

	pthread_mutex_lock (&m_lock.mutex);
	m_tasks.push_back (task);
	pthread_mutex_unlock (&m_lock.mutex);
}

void
nrfActionTaskMng::apiRemoveTask (long long sensorAddress) {
	pthread_mutex_lock (&m_lock.mutex);
    printf ("(nrfActionTaskMng) [apiRemoveTask] REMOVE %lld\n", sensorAddress);
	for (std::vector<nrf_action_task>::iterator item = m_tasks.begin(); item != m_tasks.end(); ++item) {
		if (item->sensor.sensorAddress == sensorAddress) {
            printf ("(nrfActionTaskMng) [apiRemoveTask] REMOVE\n");
			m_tasks.erase (item);
			pthread_mutex_unlock (&m_lock.mutex);
			return;
		}
	}
	pthread_mutex_unlock (&m_lock.mutex);
}
