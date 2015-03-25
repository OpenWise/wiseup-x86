/*
 * Author: Yevgeniy Kiveisha <yevgeniy.kiveisha@intel.com>
 * Copyright (c) 2014, Ingleby Thomas C
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include <stdint.h>

/**
 * Opaque pointer definition to the internal struct _spi
 */
typedef struct _spi* spi_context;

/**
 * Initialise SPI_context, uses board mapping. Sets the muxes
 *
 * @param bus Bus to use, as listed in platform definition, normally 0
 * @return Spi context or NULL
 */
spi_context spi_init(int bus, int slave);

/**
 * Set the SPI device mode. see spidev 0-3.
 *
 * @param dev The Spi context
 * @param mode The SPI mode, See Linux spidev
 * @return Spi context or NULL
 */
result_t spi_mode(spi_context dev, unsigned short mode);

/** Set the SPI device operating clock frequency.
 *
 * @param dev the Spi context
 * @param hz the frequency in hz
 * @return spi_context The returned initialised SPI context
 */
result_t spi_frequency(spi_context dev, int hz);

/** Write Single Byte to the SPI device.
 *
 * @param dev The Spi context
 * @param data Data to send
 * @return Data received on the miso line
 */
uint8_t spi_write(spi_context dev, uint8_t data);

/** Write Buffer of bytes to the SPI device. The pointer return has to be
 * free'd by the caller.
 *
 * @param dev The Spi context
 * @param data to send
 * @param length elements within buffer, Max 4096
 * @return Data received on the miso line, same length as passed in
 */
uint8_t* spi_write_buf(spi_context dev, uint8_t* data, int length);

/**
 * Change the SPI lsb mode
 *
 * @param dev The Spi context
 * @param lsb Use least significant bit transmission. 0 for msbi
 * @return Result of operation
 */
result_t spi_lsbmode(spi_context dev, int lsb);

/**
 * Set bits per mode on transaction, defaults at 8
 *
 * @param dev The Spi context
 * @param bits bits per word
 * @return Result of operation
 */
result_t spi_bit_per_word(spi_context dev, unsigned int bits);

/**
 * De-inits an spi_context device
 *
 * @param dev The Spi context
 * @return Result of operation
 */
result_t spi_stop(spi_context dev);

#ifdef __cplusplus
}
#endif

