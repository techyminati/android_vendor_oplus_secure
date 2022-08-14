/*
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 */

#ifndef FPC_NAV_TYPES_H
#define FPC_NAV_TYPES_H

#include <stdint.h>

/**
 * \file
 *
 * \defgroup Image related settings
 * \defgroup subarea related settings
 */

enum {
    FPC_NAV_REQUEST_POLL_DATA,             /**< request a boomerang to ta, and call poll_data again */
    FPC_NAV_REQUEST_WAIT_IRQ_HIGH,         /**< request to wait for irq high to continue            */
    FPC_NAV_REQUEST_WAIT_IRQ_LOW,          /**< request to wait for irq low to continue             */
};

typedef enum {
    FPC_NAV_MODE_MIN = 0,
    FPC_NAV_MODE_SWIPE_AFD,                /**< Use afd based navigation mode */
    FPC_NAV_MODE_SWIPE_DFD,                /**< Use dfd + image based navigation mode */
    FPC_NAV_MODE_TRACKPAD,                 /**< Use dfd + image based navigation to give raw dx, dy */
    FPC_NAV_MODE_HW,
    FPC_NAV_MODE_MAX,
} fpc_nav_mode_t;

typedef enum {
    FPC_NAV_EVENT_NONE = 0,
    FPC_NAV_EVENT_SINGLE_CLICK,            //!< click
    FPC_NAV_EVENT_HOLD_CLICK,              //!< hold click
    FPC_NAV_EVENT_SLIDE_UP,                //!< up swipe
    FPC_NAV_EVENT_SLIDE_DOWN,              //!< down swipe
    FPC_NAV_EVENT_SLIDE_LEFT,              //!< left swipe
    FPC_NAV_EVENT_SLIDE_RIGHT,             //!< right swipe
    FPC_NAV_EVENT_DOUBLE_CLICK,            //!< double click
    FPC_NAV_EVENT_ALGO_ERROR,              /**< for debug, error during evaluating image,
                                                saving debug buffer for analysis */
} fpc_nav_event_t;

typedef struct {
    // TODO: Should use time, for instance take a capture when init, and calc the threshold dynamicly
    uint32_t threshold_tap_n_images_max;         /**< Maximum image frames a tap could have.<br/>
                                                       ~= tap_time_threshold / image_capture_time <br/>
                                                      Unit: (number of pic)  */
    uint32_t threshold_hold_n_images_min;        /**< Minimum image frames a hold click should have.<br/>
                                                       ~= hold_time_threshold / image_capture_time <br/>
                                                      Unit: (number of pic)  */
    uint32_t double_click_time_interval;         /**< Determines the time interval to wait for a second
                                                      click after a single click has been detected.
                                                      If a second click is detected within this time the
                                                      motion will be considered a double click.
                                                      If time exceeds this value a single click will be
                                                      generated.<br/>Unit: (ms) */
    /**
     * \name Image related settings
     * \{
     */
    uint32_t threshold_tap_translation_max;      /**< Maximum amount of movement in pixels a tap could move.<br/>
                                                      Unit: (px)  */
    uint32_t threshold_swipe_translation_min;    /**< Minimum amount of movement needed to consider a swipe.<br/>
                                                      Unit: (px)  */
    /** \} */
    uint32_t mode;                               /**< which mode to use, value should be one from \ref fpc_nav_mode_t*/
} fpc_nav_config_t;

enum {
    FORCE_SENSOR_NOT_AVAILABLE = -1,
};

typedef struct {
    fpc_nav_event_t nav_event;
    int32_t force;
    int32_t finger_down;
    int32_t request;
} fpc_nav_data_t;

typedef struct {
    int32_t x;
    int32_t y;
} fpc_nav_pos_t;

#endif // FPC_NAV_TYPES_H
