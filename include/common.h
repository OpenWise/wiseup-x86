/*
 * Author: Yevgeniy Kiveisha <yevgeniy.kiveisha@intel.com>
 * Copyright (c) 2014 Wiseup.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <pthread.h>

/**
 * return codes
 */
typedef enum {
    SUCCESS                              =  0, /**< Expected response */
    ERROR_FEATURE_NOT_IMPLEMENTED        =  1, /**< Feature TODO */
    ERROR_FEATURE_NOT_SUPPORTED          =  2, /**< Feature not supported by HW */
    ERROR_INVALID_VERBOSITY_LEVEL        =  3, /**< Verbosity level wrong */
    ERROR_INVALID_PARAMETER              =  4, /**< Parameter invalid */
    ERROR_INVALID_HANDLE                 =  5, /**< Handle invalid */
    ERROR_NO_RESOURCES                   =  6, /**< No resource of that type avail */
    ERROR_INVALID_RESOURCE               =  7, /**< Resource invalid */
    ERROR_INVALID_QUEUE_TYPE             =  8, /**< Queue type incorrect */
    ERROR_NO_DATA_AVAILABLE              =  9, /**< No data available */
    ERROR_INVALID_PLATFORM               = 10, /**< Platform not recognised */
    ERROR_PLATFORM_NOT_INITIALISED       = 11, /**< Board information not initialised */
    ERROR_PLATFORM_ALREADY_INITIALISED   = 12, /**< Board is already initialised */

    ERROR_UNSPECIFIED                    = 99 /**< Unknown Error */
} result_t;

#ifdef __cplusplus
}
#endif
