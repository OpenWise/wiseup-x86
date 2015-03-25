/*
 * Author: Yevgeniy Kiveisha <yevgeniy.kiveisha@intel.com>
 * Copyright (c) 2014 Intel Corporation.
 */

#include <iostream>
#include <unistd.h>
#include <stdio.h>

#include "clientHandler.h"
#include "commonMethods.h"
#include "rfComm.h"

using namespace std;

extern uint8_t local_address[5];

WiseClientHandler::WiseClientHandler (redisContext * redisCtx) {	
	pthread_cond_init  (&lock.cond,  NULL);
	pthread_mutex_init (&lock.mutex, NULL);
	m_redisCtx = redisCtx;
}

WiseClientHandler::~WiseClientHandler () {
	pthread_mutex_destroy (&lock.mutex);
    pthread_cond_destroy  (&lock.cond);
}

void
WiseClientHandler::setCurrentDataPacket (rfcomm_data* wisePacket) {
	m_currentWisePacket = wisePacket;
}

long long
WiseClientHandler::getSensorHubAddress () {
	long long sensorHubAddress = 0;
	memcpy (&sensorHubAddress, m_currentWisePacket->sender, 5);
	return sensorHubAddress;
}

long long
WiseClientHandler::getSensorHubAddress (long long sensorAddr) {
	return sensorAddr >> 8;
}

long long
WiseClientHandler::getSensorAddress (rfcomm_sensor_info* sensorInfo) {
	long long sensorAddress = 0;
	memcpy (&sensorAddress, m_currentWisePacket->sender, 5);
	sensorAddress = (sensorAddress << 8) | sensorInfo->sensor_address;
	return sensorAddress;
}

uint8_t
WiseClientHandler::getSensorId (long long sensorAddr) {
	return sensorAddr & 0xFF;
}

void
WiseClientHandler::printClentInfo () {
	pthread_mutex_lock (&lock.mutex);
	if (m_clients.empty()) {
    } else {
		// Print here what you need.
	}
	pthread_mutex_unlock (&lock.mutex);
}

void
WiseClientHandler::updateSensorInfo (rfcomm_data* wisePacket) {
	uint8_t* data_ptr  = wisePacket->data_frame.unframeneted.data;
	WiseClient* client = findClient (wisePacket->sender);
	
	if (client != NULL) {
		char buffer[128];
		redisReply* reply = NULL;
		setCurrentDataPacket (wisePacket);
		long long sensorHubAddress = getSensorHubAddress ();
		
		switch (wisePacket->sender_information.sender_type) {
			case SENDER_SENSOR_LOCAL_HUB: {		
				rfcomm_sensor_info* sensor_info = (rfcomm_sensor_info *)data_ptr;
				while (wisePacket->data_information.data_size) {
					data_ptr += SENSOR_INFO_DATA_SIZE;
					 
					sprintf (buffer, "PUBLISH SENSOR-INFO {\"id\":\"%lld\",\"hub\":\"%lld\",\"addr\":\"%d\",\"type\":\"%d\",\"value\":\"%d\"}", 
																											getSensorAddress (sensor_info),
																											getSensorHubAddress (),
                                                                                                            sensor_info->sensor_address,
																											sensor_info->sensor_type, 
																											(int)*data_ptr);
					reply = (redisReply *)redisCommand (m_redisCtx, buffer);
					freeReplyObject(reply);
					
					data_ptr += sensor_info->sensor_data_len;
					wisePacket->data_information.data_size = wisePacket->data_information.data_size - 
												(SENSOR_INFO_DATA_SIZE + sensor_info->sensor_data_len);
					sensor_info = (rfcomm_sensor_info *)data_ptr;
				}
			}
			break;
		}
	}
}

wise_status_t
WiseClientHandler::registrationCheck (rfcomm_data* wisePacket) {
    WiseClient* device = findClient (wisePacket->sender);
	
    if (wisePacket->data_information.data_type == DEVICE_PROT_DATA_TYPE) {
        rfcomm_device_prot* prot = (rfcomm_device_prot*)wisePacket->data_frame.unframeneted.data;
        
        if (prot->device_cmd == DEVICE_PROT_CONNECT_REQ) {
            if (device != NULL) {
                device->timestamp = (uint64_t)time(NULL); // Update time-stamp
                
                if (device->status == DISCOVERY) {
					printf ("(WiseClientHandler) [registrationCheck] DISCOVERY [%d %d %d %d %d]\n",
														wisePacket->sender[0], wisePacket->sender[1], 
														wisePacket->sender[2], wisePacket->sender[3], 
														wisePacket->sender[4]);
                    return DISCOVERY;
                } else if (device->status == CONNECTED) {
					printf ("(WiseClientHandler) [registrationCheck] CONECTED [%d %d %d %d %d]\n",
														wisePacket->sender[0], wisePacket->sender[1], 
														wisePacket->sender[2], wisePacket->sender[3], 
														wisePacket->sender[4]);
                    return CONNECTED;
                }
            } else {
                /* 
                 * New device, let us add it to the list of devices and
                 * send our address back to the device. When the device will recieve 
                 * our address it will stop broadcasting and know this gateway.
                 */
                printf ("(WiseClientHandler) [registrationCheck] NEW DEVICE [%d %d %d %d %d]\n",
														wisePacket->sender[0], wisePacket->sender[1], 
														wisePacket->sender[2], wisePacket->sender[3], 
														wisePacket->sender[4]);
                return DISCOVERY;
            }
        }
    } else if (wisePacket->data_information.data_type == SENSOR_INFO_DATA_NO_AUTH_TYPE) {
        if (device != NULL) {
            device->timestamp = (uint64_t)time(NULL);
            printf ("(WiseClientHandler) [registrationCheck] SENSOR DATA UNREGISTERED [%d %d %d %d %d]\n",
														wisePacket->sender[0], wisePacket->sender[1], 
														wisePacket->sender[2], wisePacket->sender[3], 
														wisePacket->sender[4]);
            return SENSOR_INFO_NO_AUTH_REGISTERED_DEVICE;
        }
        
        return SENSOR_INFO_NO_AUTH_UNREGISTERED_DEVICE;
    } else {
        if (device != NULL) {
            device->timestamp = (uint64_t)time(NULL);
			printf ("(WiseClientHandler) [registrationCheck] SENSOR DATA [%d %d %d %d %d]\n",
														wisePacket->sender[0], wisePacket->sender[1], 
														wisePacket->sender[2], wisePacket->sender[3], 
														wisePacket->sender[4]);
            return CONNECTED;
        }
    }
	
	printf ("(WiseClientHandler) [registrationCheck] UNKNOWN [%d %d %d %d %d]\n",
														wisePacket->sender[0], wisePacket->sender[1], 
														wisePacket->sender[2], wisePacket->sender[3], 
														wisePacket->sender[4]);
    
    return UNKNOWN;
}

void
WiseClientHandler::sendRegistration (rfcomm_data* wisePacketRX) {
    rfcomm_data wisePacketTX;
	memset (&wisePacketTX, 0, 32);

    /* Create pacakge */
    wisePacketTX.data_information.data_type      = DEVICE_PROT_DATA_TYPE;
    wisePacketTX.data_information.data_size      = DEVICE_PROT_CONN_DATA_SIZE;
    wisePacketTX.control_flags.is_fragmeneted    = 0;
    wisePacketTX.control_flags.version           = 1;
    wisePacketTX.control_flags.is_broadcast      = 0;
    wisePacketTX.control_flags.is_ack            = 0;
    wisePacketTX.magic_number[0]                 = 0xAA;
    wisePacketTX.magic_number[1]                 = 0xBB;
    memcpy (wisePacketTX.sender, local_address, 5);
    memcpy (wisePacketTX.target, wisePacketRX->sender, 5);
    rfcomm_device_prot* deviceProt              = (rfcomm_device_prot *)wisePacketTX.data_frame.unframeneted.data;
    deviceProt->device_cmd                      = DEVICE_PROT_CONNECT_ADDR;
    memcpy (deviceProt->device_data, local_address, 5);
	
	nrf24l01_msg_t msg;
	memcpy (&msg.packet, &wisePacketTX, 32);
    msg.features.with_ack   = NO;
    msg.sensorAddress       = 0x0;

    // Send to OUTGOING queue
    WiseIPC *ipcPacketsOut = new WiseIPC ("/tmp/wiseup/nrf_outgoing_queue");
    if (ipcPacketsOut->setClient () == SUCCESS) {
        ipcPacketsOut->setBuffer((uint8_t *)&msg);
        if (ipcPacketsOut->sendMsg(sizeof (nrf24l01_msg_t)) == false) { 
		} else {
        	printf ("(WiseClientHandler) [sendRegistration] Registration for  [%d %d %d %d %d]\n", 
                                                wisePacketTX.target[0], wisePacketTX.target[1], 
                                                wisePacketTX.target[2], wisePacketTX.target[3], 
                                                wisePacketTX.target[4]);
        }
    }

    delete ipcPacketsOut;
}

void
WiseClientHandler::sendSensorCommand (long long sensorAddr, int cmd) {
	long long 	hostAddr = getSensorHubAddress (sensorAddr);
	uint8_t		sensorId = getSensorId (sensorAddr);
	
	printf ("!!!!!!!!!!!!!!!!! %lld, %d, %d\n", hostAddr, sensorId, cmd);
	
	uint8_t destination[5] = {0};
	memcpy (destination, &hostAddr, 5);
	
	rfcomm_data wisePacketTX;
	memset (&wisePacketTX, 0, 32);
	
	/* Create pacakge */
    wisePacketTX.data_information.data_type      = SENSOR_CMD_DATA_TYPE;
    wisePacketTX.data_information.data_size      = SENSOR_CMD_DATA_TYPE_SIZE;
    wisePacketTX.control_flags.is_fragmeneted    = 0;
    wisePacketTX.control_flags.version           = 1;
    wisePacketTX.control_flags.is_broadcast      = 0;
    wisePacketTX.control_flags.is_ack            = 0;
    wisePacketTX.magic_number[0]                 = 0xAA;
    wisePacketTX.magic_number[1]                 = 0xBB;
    memcpy (wisePacketTX.sender, local_address, 5);
    memcpy (wisePacketTX.target, destination, 5);
	
    rfcomm_sensor_command* sensorCmd = (rfcomm_sensor_command *)wisePacketTX.data_frame.unframeneted.data;
	sensorCmd->sensor_address        = sensorId;
	sensorCmd->command_type          = SENSOR_CMD_RELAY;
	sensorCmd->command_data[0]       = cmd;
	
	nrf24l01_msg_t msg;
	memcpy (&msg.packet, &wisePacketTX, 32);
    msg.features.with_ack   = YES;
    msg.sensorAddress       = sensorAddr;
	
	// Send to OUTGOING queue
    WiseIPC *ipcPacketsOut = new WiseIPC ("/tmp/wiseup/nrf_outgoing_queue");
    if (ipcPacketsOut->setClient () == SUCCESS) {
        ipcPacketsOut->setBuffer((uint8_t *)&msg);
        if (ipcPacketsOut->sendMsg(sizeof (nrf24l01_msg_t)) == false) { 
		} else {
        	printf ("(WiseClientHandler) [sendSensorCommand] Action for  [%d %d %d %d %d]\n", 
                                                wisePacketTX.target[0], wisePacketTX.target[1], 
                                                wisePacketTX.target[2], wisePacketTX.target[3], 
                                                wisePacketTX.target[4]);
        }
    }

    delete ipcPacketsOut;
}

void
WiseClientHandler::addNewClient (uint8_t* address) {
	WiseClient* existedClient = findClient (address);
	
	// Add only if client doesn't exist
	if (existedClient == NULL) {
		WiseClient client   = WiseClient(address);
		client.timestamp    = (uint64_t)time(NULL);
		client.status       = DISCOVERY;
		
		pthread_mutex_lock (&lock.mutex);
		m_clients.push_back (client);
		pthread_mutex_unlock (&lock.mutex);
	}
}

WiseClient* 
WiseClientHandler::findClient (uint8_t * address) {
    WiseClient client = WiseClient(address);
    
	pthread_mutex_lock (&lock.mutex);
    if (m_clients.empty()) {
    } else {
        /* Lets try to find this device in the list */
        for (std::vector<WiseClient>::iterator item = m_clients.begin(); item != m_clients.end(); ++item) {
            if (*item == client) {
				pthread_mutex_unlock (&lock.mutex);
                return &(*item);
            }
        }
    }
	
    pthread_mutex_unlock (&lock.mutex);
    return NULL;
}

WiseClient::WiseClient (uint8_t * addr) {
	memcpy (address, addr, 5);
}

WiseClient::~WiseClient () {
}

WiseCommandHandler::WiseCommandHandler () {
}

WiseCommandHandler::~WiseCommandHandler () {
}

void
WiseCommandHandler::commandHandler (rfcomm_data* wisePacket) {    
    if (wisePacket->data_information.data_type == DEVICE_PROT_DATA_TYPE) {
        rfcomm_device_prot* prot = (rfcomm_device_prot*)wisePacket->data_frame.unframeneted.data;
        
        switch (prot->device_cmd) {
            case 0:
                
            break;
        }
    }
}
