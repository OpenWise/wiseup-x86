/*
 * Author: Yevgeniy Kiveisha <yevgeniy.kiveisha@intel.com>
 * Copyright (c) 2014, Le Foll Brendan
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
 
/**
 * Gpio Direction options
 */
typedef enum {
    GPIO_OUT    = 0, /**< Output. A Mode can also be set */
    GPIO_IN     = 1  /**< Input */
} gpio_dir_t;

/**
 * A structure representing a gpio pin.
 */
struct _gpio {
    /*@{*/
    int pin; /**< the pin number, as known to the os. */
    int value_fp; /**< the file pointer to the value of the gpio */
    int owner; /**< If this context originally exported the pin */
    /*@}*/
};

/**
 * Opaque pointer definition to the internal struct _gpio
 */
typedef struct _gpio* gpio_context;

/**
 * Initialise gpio_context, based on board number
 *
 *  @param pin Pin number read from the board, i.e IO3 is 3
 *  @returns gpio context or NULL
 */
gpio_context gpio_init(int pin);

/**
 * Set Gpio direction
 *
 * @param dev The Gpio context
 * @param dir The direction of the Gpio
 * @return Result of operation
 */
result_t gpio_dir(gpio_context dev, gpio_dir_t dir);

/**
 * Close the Gpio context
 * - Will free the memory for the context and unexport the Gpio
 *
 * @param dev The Gpio context
 * @return Result of operation
 */
result_t gpio_close(gpio_context dev);

/**
 * Read the Gpio value.
 *
 * @param dev The Gpio context
 * @return Result of operation
 */
int gpio_read(gpio_context dev);

/**
 * Write to the Gpio Value.
 *
 * @param dev The Gpio context
 * @param value Integer value to write
 * @return Result of operation
 */
result_t gpio_write(gpio_context dev, int value);

/**
 * Close the Gpio context
 * - Will free the memory for the context and unexport the Gpio
 *
 * @param dev The Gpio context
 * @return Result of operation
 */
result_t 
gpio_close(gpio_context dev);

#ifdef __cplusplus
}
#endif

