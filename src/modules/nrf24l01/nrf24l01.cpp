/*
 * Author: Yevgeniy Kiveisha <yevgeniy.kiveisha@intel.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "nrf24l01.h"

using namespace comm;

void
nrfRecieveHandler (void * arg) {
	WiseRFComm*		obj 				= (WiseRFComm*) arg;
	uint8_t         BROADCAST_ADDR[5]   = {0xFA, 0xFA, 0xFA, 0xFA, 0xFA};
	rfcomm_data *   packet              = (rfcomm_data *)obj->m_network->m_rxBuffer;
    
    printf ("(nrf24l01) [nrfRecieveHandler] >>>>>> GOT RF <<<<<< \n");
    for (int i = 0; i < 32; i++) {
        printf ("%x ", ((uint8_t *)packet)[i]);
    }
    printf ("\n");

	if (packet->magic_number[0] == 0xAA && packet->magic_number[1] == 0xBB) {
		/* BROADCAST PACKET */
		if (!memcmp (BROADCAST_ADDR, packet->target, 5)) {
            printf ("(nrf24l01) [nrfRecieveHandler] >>>>>> BROADCAST <<<<<< \n");
			obj->m_BroadcastHandler (obj->m_network->m_rxBuffer);
			obj->m_broadcastPacketCounter++;
		}
		
		/* UNICAST PACKET */
		if (!memcmp (obj->m_sender, packet->target, 5)) {
            printf ("(nrf24l01) [nrfRecieveHandler] >>>>>> UNICAST <<<<<< \n");
			obj->m_DataHandler (obj->m_network->m_rxBuffer);
			obj->m_dataPacketCounter++;
		}
	}
}

WiseRFComm::WiseRFComm (NRF24L01 * network, funcPtrVoidVoid dataHandler, funcPtrVoidVoid broadcastHandler) {
	m_network = network;
	
	pthread_cond_init  (&m_lock.cond,  NULL);
    pthread_mutex_init (&m_lock.mutex, NULL);
	
	init ();
	m_network->dataRecievedHandler = nrfRecieveHandler;
	m_network->dataContext = this;
	m_DataHandler = dataHandler;
	m_BroadcastHandler = broadcastHandler;
	
	ptrRX = m_network->m_rxBuffer;
	ptrTX = m_network->m_txBuffer;
	
	m_broadcastPacketCounter 	= 0;
	m_dataPacketCounter 		= 0;
}

WiseRFComm::~WiseRFComm () {
	pthread_mutex_destroy (&m_lock.mutex);
    pthread_cond_destroy  (&m_lock.cond);
}

void
WiseRFComm::sendPacket (uint8_t * target) {
	rfcomm_data * packet = (rfcomm_data *)m_network->m_txBuffer;
	
	pthread_mutex_lock (&m_lock.mutex);
	m_network->configure ();
	memcpy (packet->target, target, 5);
	setTarget (target);
	m_network->send ();
	m_network->configure ();
	pthread_mutex_unlock (&m_lock.mutex);
}

void
WiseRFComm::clearBufferTX () {
	memset (m_network->m_txBuffer, 0x0, 32);
}

void
WiseRFComm::clearBufferRX () {
	memset (m_network->m_rxBuffer, 0x0, 32);
}

void
WiseRFComm::setChannel (uint8_t channel) {
	pthread_mutex_lock (&m_lock.mutex);
	m_network->setChannel (channel);
	pthread_mutex_unlock (&m_lock.mutex);
}

void
WiseRFComm::setSender (uint8_t * sender) {
	memcpy (m_sender, sender, 5);
	// m_network->setSourceAddress (m_sender);
}

void
WiseRFComm::setTarget (uint8_t * target) {
	memcpy (m_target, target, 5);
}

void
WiseRFComm::listenForIncoming () {
	pthread_mutex_lock (&m_lock.mutex);
	m_network->pollListener ();
	pthread_mutex_unlock (&m_lock.mutex);
	usleep (10000);
}

void
WiseRFComm::init () {
    uint8_t BROADCAST_ADDR[5] = {0xFA, 0xFA, 0xFA, 0xFA, 0xFA};
    pthread_mutex_lock (&m_lock.mutex);
    m_network->setSourceAddress         ((uint8_t *) BROADCAST_ADDR);
    m_network->setDestinationAddress    ((uint8_t *) BROADCAST_ADDR);
	
    m_network->setPayload (32);
    m_network->configure ();
    m_network->setSpeedRate (NRF_250KBPS);
    m_network->setChannel (99);
    pthread_mutex_unlock (&m_lock.mutex);
}

NRF24L01::NRF24L01 (uint8_t cs, uint8_t ce) {
    m_cs = cs;
    m_ce = ce;

    init (cs, ce);
}

NRF24L01::~NRF24L01 () {
    mraa_result_t error = MRAA_SUCCESS;
	
    error = mraa_spi_stop(m_spi);
    if (error != MRAA_SUCCESS) {
        fprintf (stderr, "Error #%d", error);
    }
    
    /*
    error = mraa_gpio_close (m_csnPinCtx);
    if (error != MRAA_SUCCESS) {
        fprintf (stderr, "Error #%d", error);
    }
    */

    error = mraa_gpio_close (m_cePinCtx);
    if (error != MRAA_SUCCESS) {
        fprintf (stderr, "Error #%d", error);
    }
}

void
NRF24L01::init (uint8_t chipSelect, uint8_t chipEnable) {    
    mraa_result_t error = MRAA_SUCCESS;

    m_csnPin    = chipSelect;
    m_cePin     = chipEnable;
    m_channel   = 99;
	
    m_spi = mraa_spi_init (1);
    if (m_spi == NULL) {
        printf("Initialization of spi failed, check syslog for details, exit...\n");
        exit(1);
    }

    mraa_spi_mode (m_spi, MRAA_SPI_MODE0);
    mraa_spi_bit_per_word (m_spi, 8);
    mraa_spi_frequency (m_spi, 8000000);

    printf("SPI initialised successfully\n");

    m_csnPinCtx = mraa_gpio_init (m_csnPin);
    if (m_csnPinCtx == NULL) {
        fprintf (stderr, "Are you sure that pin%d you requested is valid on your platform?", m_csnPin);
        exit (1);
    }

    m_cePinCtx = mraa_gpio_init (m_cePin);
    if (m_cePinCtx == NULL) {
        fprintf (stderr, "Are you sure that pin%d you requested is valid on your platform?", m_cePin);
        exit (1);
    }

    error = mraa_gpio_dir (m_csnPinCtx, MRAA_GPIO_OUT);
    if (error != MRAA_SUCCESS) {
        fprintf (stderr, "Error #%d", error);
    }

    error = mraa_gpio_dir (m_cePinCtx, MRAA_GPIO_OUT);
    if (error != MRAA_SUCCESS) {
        fprintf (stderr, "Error #%d", error);
    }

    ceLow();
    csOff ();
    
    printf("NRF initialised successfully\n");
}

void
NRF24L01::configure () {
    /* Set RF channel */
    setRegister (RF_CH, m_channel);

    /* Set length of incoming payload */
    setRegister (RX_PW_P0, m_payload);

    /* Set length of incoming payload for broadcast */
    setRegister (RX_PW_P1, m_payload);
    
    /* Start receiver */
    rxPowerUp ();
    rxFlushBuffer ();
}

void
NRF24L01::send (uint8_t * value) {
    uint8_t status;
    status = getStatus();

    while (m_ptx) {
        status = getStatus();

        if((status & ((1 << TX_DS)  | (1 << MAX_RT)))){
            m_ptx = 0;
            break;
        }
    } // Wait until last paket is send
	
    // TODO - delete it for production
	printf ("#(nrf24l01) OUT ( ");
	for (int i = 0; i < 32; i++) {
		printf ("%x ", value[i]);
	} printf (")\n");
    // -------------------------------

    ceLow ();
    
    txPowerUp (); // Set to transmitter mode , Power up
    txFlushBuffer ();

    csOn ();
    mraa_spi_write (m_spi, W_TX_PAYLOAD); // Write cmd to write payload
    writeBytes (value, NULL, m_payload); // Write payload
    csOff ();

    ceHigh(); // Start transmission
    
    while (dataSending ()) {  }
	usleep (100000);

    /*ceLow ();
    rxPowerUp ();
    rxFlushBuffer ();
    ceHigh();*/
}

void
NRF24L01::send () {
    send (m_txBuffer);
}

void
NRF24L01::setSourceAddress (uint8_t * addr) {
    ceLow ();
    writeRegister (RX_ADDR_P0, addr, ADDR_LEN);
    ceHigh ();
}

void
NRF24L01::setDestinationAddress (uint8_t * addr) {
    writeRegister (TX_ADDR, addr, ADDR_LEN);
}

void
NRF24L01::setBroadcastAddress (uint8_t * addr) {
    writeRegister (RX_ADDR_P1, addr, ADDR_LEN);
}

void
NRF24L01::setPayload (uint8_t payload) {
    m_payload = payload;
}

bool
NRF24L01::dataReady () {
    /* See note in getData() function - just checking RX_DR isn't good enough */
    uint8_t status = getStatus();
    /* We can short circuit on RX_DR, but if it's not set, we still need
     * to check the FIFO for any pending packets */
    if ( status & (1 << RX_DR) ) {
        return 1;
    }
    
    return !rxFifoEmpty();
}

bool
NRF24L01::dataSending () {
    uint8_t status;
    if(m_ptx)   { // Sending mode.
        status = getStatus();
        /* if sending successful (TX_DS) or max retries exceded (MAX_RT). */
        if((status & ((1 << TX_DS)  | (1 << MAX_RT)))){
            rxPowerUp ();
            return false; 
        }
        return true;
    }
    return false;
}

void
NRF24L01::getData (uint8_t * data)  {
    csOn ();
    /* Send cmd to read rx payload */
    mraa_spi_write (m_spi, R_RX_PAYLOAD);
    /* Read payload */
    writeBytes (data, data, m_payload);
    csOff ();
    /* NVI: per product spec, p 67, note c:
     * "The RX_DR IRQ is asserted by a new packet arrival event. The procedure
     * for handling this interrupt should be: 1) read payload through SPI,
     * 2) clear RX_DR IRQ, 3) read FIFO_STATUS to check if there are more 
     * payloads available in RX FIFO, 4) if there are more data in RX FIFO,
     * repeat from step 1)."
     * So if we're going to clear RX_DR here, we need to check the RX FIFO
     * in the dataReady() function */
    /* Reset status register */
    setRegister (STATUS, (1<<RX_DR));
}

uint8_t
NRF24L01::getStatus() {
    return getRegister (STATUS);
}

bool
NRF24L01::rxFifoEmpty () {
    uint8_t fifoStatus = getRegister (FIFO_STATUS);
	return (fifoStatus & (1 << RX_EMPTY));
}

void
NRF24L01::rxPowerUp () {
    m_ptx = 0;
    ceLow ();
    setRegister (CONFIG, _CONFIG | ( (1 << PWR_UP) | (1 << PRIM_RX) ));
    ceHigh ();
    setRegister (STATUS, (1 << TX_DS) | (1 << MAX_RT)); 
}

void
NRF24L01::rxFlushBuffer () {
    sendCommand (FLUSH_RX);
}

void
NRF24L01::txPowerUp () {
    m_ptx = 1;
    setRegister (CONFIG, _CONFIG | ( (1 << PWR_UP) | (0 << PRIM_RX) ));
}

void
NRF24L01::powerDown(){
    ceLow ();
    setRegister (CONFIG, _CONFIG);
}

void
NRF24L01::setChannel (uint8_t channel) {
    m_channel = channel;
    setRegister (RF_CH, channel);
}

void
NRF24L01::setPower (power_t power) {
    uint8_t setupRegisterData = 0;

    switch (power) {
        case NRF_0DBM:
            m_power = 3;
        break;
        case NRF_6DBM:
            m_power = 2;
        break;
        case NRF_12DBM:
            m_power = 1;
        break;
        case NRF_18DBM:
            m_power = 0;
        break;
    }

    setupRegisterData = getRegister (RF_SETUP); // Read current value.
    setupRegisterData &= 0xFC; // Erase the old value;
    setupRegisterData |= (m_power & 0x3);
    setRegister (RF_SETUP, setupRegisterData); // Write the new value.
}

uint8_t
NRF24L01::setSpeedRate (speed_rate_t rate) {
    uint8_t setupRegisterData = 0;

    setupRegisterData = getRegister (RF_SETUP); // Read current value.
    setupRegisterData &= ~((1 << RF_DR_LOW) | (1 << RF_DR_HIGH));

    switch (rate) {
        case NRF_250KBPS:
            setupRegisterData |= (1 << RF_DR_LOW) ;
        break;
        case NRF_1MBPS:
        break;
        case NRF_2MBPS:
            setupRegisterData |= (1 << RF_DR_HIGH);
        break;
    }

    setRegister (RF_SETUP, setupRegisterData); // Write the new value.

    if (setupRegisterData == getRegister (RF_SETUP)) {
        return 0x0;
    }

    return 0x1;
}

mraa_result_t
NRF24L01::ceHigh () {
    return mraa_gpio_write (m_cePinCtx, HIGH);
}

mraa_result_t
NRF24L01::ceLow () {
    return mraa_gpio_write (m_cePinCtx, LOW);
}

mraa_result_t
NRF24L01::csOn () {
    return mraa_gpio_write (m_csnPinCtx, LOW);
}

mraa_result_t
NRF24L01::csOff () {
    return mraa_gpio_write (m_csnPinCtx, HIGH);
}

void
NRF24L01::pollListener() {
    if (dataReady()) {
        getData (m_rxBuffer);
        dataRecievedHandler (dataContext); /* let know that data arrived */
    }
}

void
NRF24L01::txFlushBuffer () {
	sendCommand (FLUSH_TX);
}

void
NRF24L01::setBeaconingMode () {
    setRegister (CONFIG,     0x12);	// on, no crc, int on RX/TX done
	setRegister (EN_AA,      0x00);	// no auto-acknowledge
	setRegister (EN_RXADDR,  0x00);	// no RX
	setRegister (SETUP_AW,   0x02);	// 5-byte address
	setRegister (SETUP_RETR, 0x00);	// no auto-retransmit
	setRegister (RF_SETUP,   0x06);	// 1MBps at 0dBm
	setRegister (STATUS,     0x3E);	// clear various flags
	setRegister (DYNPD,      0x00);	// no dynamic payloads
	setRegister (FEATURE,    0x00);	// no features
	setRegister (RX_PW_P0,   32);	// always RX 32 bytes
	setRegister (EN_RXADDR,  0x01);	// RX on pipe 0
    
    uint8_t addr[4] = { swapbits(0x8E), swapbits(0x89), swapbits(0xBE), swapbits(0xD6)};
    writeRegister (TX_ADDR,     addr, 4);
    writeRegister (RX_ADDR_P0,  addr, 4);
    
    uint8_t index = 0;
    m_bleBuffer[index++] = 0x42;        // PDU type, given address is random
    m_bleBuffer[index++] = 0x1B;        // 6+3+2+16 = 27 bytes of payload
    
    m_bleBuffer[index++] = BLE_MAC_0;
    m_bleBuffer[index++] = BLE_MAC_1;
    m_bleBuffer[index++] = BLE_MAC_2;
    m_bleBuffer[index++] = BLE_MAC_3;
    m_bleBuffer[index++] = BLE_MAC_4;
    m_bleBuffer[index++] = BLE_MAC_5;
    
    m_bleBuffer[index++] = 2;           // flags (LE-only, limited discovery mode)
    m_bleBuffer[index++] = 0x01;
    m_bleBuffer[index++] = 0x05;
    
    m_bleBuffer[index++] = 17;
    m_bleBuffer[index++] = 0x08;
}

void
NRF24L01::sendBeaconingMsg (uint8_t * msg) {
    const uint8_t   chRf[] = {2, 26,80};
    const uint8_t   chLe[] = {37,38,39};
    uint8_t         index = BLE_PAYLOAD_OFFSET + 16;
    
    memcpy (&m_bleBuffer[BLE_PAYLOAD_OFFSET], msg, 16);
    m_bleBuffer[index++] = 0x55;
    m_bleBuffer[index++] = 0x55;
    m_bleBuffer[index++] = 0x55;
        
    uint8_t channel = 0;
    while (++channel != sizeof(chRf)) {
        setRegister (RF_CH,     chRf[channel]);
		setRegister (STATUS,    0x6E);	        //clear flags
        
        blePacketEncode (m_bleBuffer, index, chLe[channel]);
        
        sendCommand (FLUSH_TX); // Clear RX Fifo
		sendCommand (FLUSH_RX); // Clear TX Fifo
        
        csOn ();
        mraa_spi_write (m_spi, W_TX_PAYLOAD);        // Write cmd to write payload
        writeBytes (m_bleBuffer, NULL, 32);     // Write payload
        csOff ();
        
        setRegister (CONFIG, 0x12);	            // tx on
        ceHigh ();                              // Start transmission
        usleep (10000);
        ceLow ();
    }
}

/*
 * ---------------
 * PRIVATE SECTION
 * ---------------
 */

/*
 * Write bytes to the SPI device.
 */
void
NRF24L01::writeBytes (uint8_t * dataout, uint8_t * datain, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        if (datain != NULL) {
            datain[i] = mraa_spi_write (m_spi, dataout[i]);
        } else {
			// printf ("%x ", dataout[i]);
            mraa_spi_write (m_spi, dataout[i]);
        }
    }
	// printf ("\n");
}

void
NRF24L01::setRegister (uint8_t reg, uint8_t value) {
    csOn ();
    mraa_spi_write (m_spi, W_REGISTER | (REGISTER_MASK & reg));
    mraa_spi_write (m_spi, value);
    csOff ();
}

uint8_t
NRF24L01::getRegister (uint8_t reg) {
    uint8_t data = 0;

    csOn ();
    mraa_spi_write (m_spi, R_REGISTER | (REGISTER_MASK & reg));
    data = mraa_spi_write (m_spi, data);
    csOff ();

    return data;
}

void
NRF24L01::readRegister (uint8_t reg, uint8_t * value, uint8_t len) {
    csOn ();
    mraa_spi_write (m_spi, R_REGISTER | (REGISTER_MASK & reg));
    writeBytes (value, value, len);
    csOff ();
}

void
NRF24L01::writeRegister (uint8_t reg, uint8_t * value, uint8_t len) {
    csOn ();
    mraa_spi_write (m_spi, W_REGISTER | (REGISTER_MASK & reg));
    writeBytes (value, NULL, len);
    csOff ();
}

void
NRF24L01::sendCommand (uint8_t cmd) {
    csOn ();
    mraa_spi_write (m_spi, cmd);
    csOff ();
}

mraa_spi_context&
NRF24L01::getSPIHandler () {
	return m_spi;
}

void
NRF24L01::bleCrc (const uint8_t* data, uint8_t len, uint8_t* dst) {
	uint8_t v, t, d;
	while(len--) {
		d = *data++;
		for(v = 0; v < 8; v++, d >>= 1){
			t = dst[0] >> 7;
			dst[0] <<= 1;
			if(dst[1] & 0x80) dst[0] |= 1;
			dst[1] <<= 1;
			if(dst[2] & 0x80) dst[1] |= 1;
			dst[2] <<= 1;
			
			if(t != (d & 1)) {
				dst[2] ^= 0x5B;
				dst[1] ^= 0x06;
			}
		}	
	}
}

void
NRF24L01::bleWhiten (uint8_t* data, uint8_t len, uint8_t whitenCoeff) {
	uint8_t  m;
	while(len--) {
		for(m = 1; m; m <<= 1) {
			if(whitenCoeff & 0x80){
				whitenCoeff ^= 0x11;
				(*data) ^= m;
			}
			whitenCoeff <<= 1;
		}
		data++;
	}
}

void
NRF24L01::blePacketEncode(uint8_t* packet, uint8_t len, uint8_t chan) {
	//length is of packet, including crc. pre-populate crc in packet with initial crc value!
	uint8_t i, dataLen = len - 3;
	
	bleCrc(packet, dataLen, packet + dataLen);
	for(i = 0; i < 3; i++, dataLen++) {
        packet[dataLen] = swapbits(packet[dataLen]);
    }
    
	bleWhiten(packet, len, (swapbits(chan) | 2));
	for(i = 0; i < len; i++) {
        packet[i] = swapbits(packet[i]);
    }
}

uint8_t
NRF24L01::swapbits(uint8_t a) {
	uint8_t v = 0;
	
	if(a & 0x80) v |= 0x01;
	if(a & 0x40) v |= 0x02;
	if(a & 0x20) v |= 0x04;
	if(a & 0x10) v |= 0x08;
	if(a & 0x08) v |= 0x10;
	if(a & 0x04) v |= 0x20;
	if(a & 0x02) v |= 0x40;
	if(a & 0x01) v |= 0x80;

	return v;
}

void
NRF24L01::nrf_print_details () {
}

void
NRF24L01::nrf_print_byte (char s[], uint8_t reg, uint8_t len) {
}
