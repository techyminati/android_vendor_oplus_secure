/**
 * @copyright
 * Copyright (c) 2016-2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#ifndef __FPC_ERROR_H__
#define __FPC_ERROR_H__

typedef enum {
    FPC_STATUS_WAIT_TIME               = 1,   /**< Indicates additional wait is required      */
    FPC_STATUS_FINGER_PRESENT          = 2,   /**< Indicates finger is present on sensor      */
    FPC_STATUS_FINGER_LOST             = 3,   /**< Indicates finger is not present on sensor  */
    FPC_STATUS_BAD_QUALITY             = 4,   /**< Indicates bad image quality                */
    FPC_STATUS_FINGER_ALREADY_ENROLLED = 5,   /**< Indicates that finger was already enrolled */

    FPC_STATUS_ENROLL_PROGRESS         = 6,   /**< Enroll ok, more images expected            */
    FPC_STATUS_ENROLL_LOW_COVERAGE     = 7,   /**< Low coverage on the sensor                 */
    FPC_STATUS_ENROLL_TOO_SIMILAR      = 8,   /**< Image was too similar                      */
    FPC_STATUS_ENROLL_LOW_QUALITY      = 9,   /**< Image had too low quality                  */
    FPC_STATUS_ENROLL_LOW_MOBILITY     = 10,  /**< Low mobility while doing swipe enroll      */
    FPC_STATUS_NOT_FINGER              = -1001,
} fpc_status_t;

typedef enum {
    FPC_ERROR_NONE                        = 0,
    FPC_ERROR_NOT_FOUND                   = 1,
    FPC_ERROR_CAN_BE_USED_2               = 2,
    FPC_ERROR_CAN_BE_USED_3               = 3,
    FPC_ERROR_CAN_BE_USED_4               = 4,
    FPC_ERROR_PAL                         = 5,
    FPC_ERROR_IO                          = 6,
    FPC_ERROR_CANCELLED                   = 7,
    FPC_ERROR_UNKNOWN                     = 8,
    FPC_ERROR_MEMORY                      = 9,
    FPC_ERROR_PARAMETER                   = 10,
    FPC_ERROR_TEST_FAILED                 = 11,
    FPC_ERROR_TIMEDOUT                    = 12,
    FPC_ERROR_SENSOR                      = 13,
    FPC_ERROR_SPI                         = 14,
    FPC_ERROR_NOT_SUPPORTED               = 15,
    FPC_ERROR_OTP                         = 16,
    FPC_ERROR_STATE                       = 17,
    FPC_ERROR_PN                          = 18,
    FPC_ERROR_DEAD_PIXELS                 = 19,
    FPC_ERROR_TEMPLATE_CORRUPTED          = 20,
    FPC_ERROR_CRC                         = 21,
    FPC_ERROR_STORAGE                     = 22, /**< Errors related to storage                  */
    FPC_ERROR_MAXIMUM_REACHED             = 23, /**< The allowed maximum has been reached       */
    FPC_ERROR_MINIMUM_NOT_REACHED         = 24, /**< The required minimum was not reached       */
    FPC_ERROR_SENSOR_LOW_COVERAGE         = 25, /**< Minimum sensor coverage was not reached    */
    FPC_ERROR_SENSOR_LOW_QUALITY          = 26, /**< Sensor image is considered low quality     */
    FPC_ERROR_SENSOR_FINGER_NOT_STABLE    = 27, /**< Finger was not stable during image capture */
    FPC_ERROR_TOO_MANY_FAILED_ATTEMPTS    = 28, /**< Too many attempts was made                 */
    FPC_ERROR_ALREADY_ENROLLED            = 29, /**< Fingerprint was already enrolled           */
} fpc_error_t;

typedef enum {
    FPC_MODULE_ID_RESERVED_MIN     = 0,   /**< 00000 indicates the error code is of legacy format */
    FPC_MODULE_ID_SENSOR           = 1,   /**< Sensor module ID                                   */
    FPC_MODULE_ID_SETTINGS         = 2,   /**< Settings module ID                                 */
    FPC_MODULE_ID_ACCESS           = 3,   /**< Access module ID                                   */
    FPC_MODULE_ID_CONFIG           = 4,   /**< Config module ID                                   */
    FPC_MODULE_ID_PERIPHIALS       = 5,   /**< Periphials module ID                               */

    FPC_MODULE_ID_DB               = 25,  /**< DB module ID                                       */
    FPC_MODULE_ID_TA               = 26,  /**< TA module ID                                       */
    FPC_MODULE_ID_NAVIGATION       = 27,  /**< Navigation client module ID                        */
    FPC_MODULE_ID_BIO_INTERFACE    = 28,  /**< BIO interface client module ID                     */
    FPC_MODULE_ID_SENSOR_INTERFACE = 29,  /**< Sensor interface client module ID                  */
    FPC_MODULE_ID_CLIENT           = 30,  /**< Generic module ID to be used by clients of the
                                               SDK who benefits from re-using the defined error
                                               code format. For instance if the client relies
                                               on modules using the legacy error code format.     */
    FPC_MODULE_ID_RESERVED         = 31,  /**< 11111 indicates the error code is of legacy format */
} fpc_module_id_t;


#define SUCCESS(X) ((X) >= 0)
#define FAILED(X) ((X) < 0)

//-------------------------------------------------------------------------------------------------
// Group     : Distribution of the 32 bits. 'Group' refers to a group of bits in the bit-field
//
// E-BIT     : Error bit                                                0|1, 1 indicates error
// Module ID : Module ID                                                Max value is 30     (5 bits)
// External  : External result (defined in external interface)          Max value is 63     (6 bits)
// Internal  : Internal result (Module specific result)                 Max value is 63     (6 bits)
// Extra     : Whatever the client prefers, e.g. __LINE__               Max value is 16383  (14 bits)
//
//-------------------------------------------------------------------------------------------------
// E-BIT | Module ID      |  External         | Internal          | Extra
//-------------------------------------------------------------------------------------------------
//   31  | 30 29 28 27 26 | 25 24 23 22 21 20 | 19 18 17 16 15 14 | 13 12 11 ... ... 04 03 02 01 00
//-------------------------------------------------------------------------------------------------

#define FPC_ERROR_BITS            1   /**< One bit indicating if the result is an error.    @hideinitializer */
#define FPC_ERROR_MODULE_BITS     5   /**< Number of identifying the module.                @hideinitializer */
#define FPC_ERROR_EXTERNAL_BITS   6   /**< Number of identifying the external result.       @hideinitializer */
#define FPC_ERROR_INTERNAL_BITS   6   /**< Number of identifying the internal result.       @hideinitializer */
#define FPC_ERROR_EXTRA_BITS      14  /**< Number of bits left for user defined extra data. @hideinitializer */

// Max value for each group
#define FPC_ERROR_MAX_ERROR    ((1U << FPC_ERROR_BITS)          - 1) /**< Max value in error group     @hideinitializer */
#define FPC_ERROR_MAX_MODULE   ((1U << FPC_ERROR_MODULE_BITS)   - 1) /**< Max value in module group    @hideinitializer */
#define FPC_ERROR_MAX_EXTERNAL ((1U << FPC_ERROR_EXTERNAL_BITS) - 1) /**< Max value in external group  @hideinitializer */
#define FPC_ERROR_MAX_INTERNAL ((1U << FPC_ERROR_INTERNAL_BITS) - 1) /**< Max value in internal group  @hideinitializer */
#define FPC_ERROR_MAX_EXTRA    ((1U << FPC_ERROR_EXTRA_BITS)    - 1) /**< Max value in extra group     @hideinitializer */

// Validates (in compile time) that 32 bits are used.
// If the value does not comply to the allocated limit, a division by zero warning is issued
#define FPC_ERROR_CHECK_BITSUM() (void)(1/!!((FPC_ERROR_BITS + FPC_ERROR_MODULE_BITS + FPC_ERROR_EXTERNAL_BITS + \
                                        FPC_ERROR_INTERNAL_BITS + FPC_ERROR_EXTRA_BITS) == 32))
/// @cond IGNORE_PARSING_DOXYGEN

// Helper macro to mask bits
#define FPC_ERROR_MSB_OFFSET_ERROR    (32 - (FPC_ERROR_BITS))
#define FPC_ERROR_MSB_OFFSET_MODULE   (32 - (FPC_ERROR_BITS + FPC_ERROR_MODULE_BITS))
#define FPC_ERROR_MSB_OFFSET_EXTERNAL (32 - (FPC_ERROR_BITS + FPC_ERROR_MODULE_BITS + FPC_ERROR_EXTERNAL_BITS))
#define FPC_ERROR_MSB_OFFSET_INTERNAL (32 - (FPC_ERROR_BITS + FPC_ERROR_MODULE_BITS + FPC_ERROR_EXTERNAL_BITS + FPC_ERROR_INTERNAL_BITS))
#define FPC_ERROR_MSB_OFFSET_EXTRA    (32 - (FPC_ERROR_BITS + FPC_ERROR_MODULE_BITS + FPC_ERROR_EXTERNAL_BITS + FPC_ERROR_INTERNAL_BITS + FPC_ERROR_EXTRA_BITS))

// Helper macro to mask bits
#define FPC_ERROR_SHIFT_ERROR    (0)
#define FPC_ERROR_SHIFT_MODULE   (FPC_ERROR_BITS)
#define FPC_ERROR_SHIFT_EXTERNAL (FPC_ERROR_BITS + FPC_ERROR_MODULE_BITS)
#define FPC_ERROR_SHIFT_INTERNAL (FPC_ERROR_BITS + FPC_ERROR_MODULE_BITS + FPC_ERROR_EXTERNAL_BITS)
#define FPC_ERROR_SHIFT_EXTRA    (FPC_ERROR_BITS + FPC_ERROR_MODULE_BITS + FPC_ERROR_EXTERNAL_BITS + FPC_ERROR_INTERNAL_BITS)

// Shifted masks for the defined groups to be used in get macros      // (Comment assumes 1-5-6-6-14 distribution)
#define FPC_ERROR_MASK           (FPC_ERROR_MAX_ERROR    << FPC_ERROR_MSB_OFFSET_ERROR)     // 0x80000000
#define FPC_ERROR_MODULE_MASK    (FPC_ERROR_MAX_MODULE   << FPC_ERROR_MSB_OFFSET_MODULE)    // 0x7c000000
#define FPC_ERROR_EXTERNAL_MASK  (FPC_ERROR_MAX_EXTERNAL << FPC_ERROR_MSB_OFFSET_EXTERNAL)  // 0x03f00000
#define FPC_ERROR_INTERNAL_MASK  (FPC_ERROR_MAX_INTERNAL << FPC_ERROR_MSB_OFFSET_INTERNAL)  // 0x000fc000
#define FPC_ERROR_EXTRA_MASK     (FPC_ERROR_MAX_EXTRA    << FPC_ERROR_MSB_OFFSET_EXTRA)     // 0x00003fff

/// @endcond

//-------------------------------------------------------------------------------------------------
// Set/Get Interface - Truncates invalid input in run time.
//-------------------------------------------------------------------------------------------------
#define FPC_ERROR_SET_ERROR_GROUP(x)    ((((x) & FPC_ERROR_MAX_ERROR)    << FPC_ERROR_MSB_OFFSET_ERROR))    /**< @hideinitializer */
#define FPC_ERROR_SET_MODULE_GROUP(x)   ((((x) & FPC_ERROR_MAX_MODULE)   << FPC_ERROR_MSB_OFFSET_MODULE))   /**< @hideinitializer */
#define FPC_ERROR_SET_EXTERNAL_GROUP(x) ((((x) & FPC_ERROR_MAX_EXTERNAL) << FPC_ERROR_MSB_OFFSET_EXTERNAL)) /**< @hideinitializer */
#define FPC_ERROR_SET_INTERNAL_GROUP(x) ((((x) & FPC_ERROR_MAX_INTERNAL) << FPC_ERROR_MSB_OFFSET_INTERNAL)) /**< @hideinitializer */
#define FPC_ERROR_SET_EXTRA_GROUP(x)    ((((x) & FPC_ERROR_MAX_EXTRA)    << FPC_ERROR_MSB_OFFSET_EXTRA))    /**< @hideinitializer */

#define FPC_ERROR_GET_ERROR_GROUP(x)    (((uint32_t)(x)) >> FPC_ERROR_MSB_OFFSET_ERROR)                                /**< @hideinitializer */
#define FPC_ERROR_GET_MODULE_GROUP(x)   ((((uint32_t)(x)) & FPC_ERROR_MODULE_MASK)   >> FPC_ERROR_MSB_OFFSET_MODULE)   /**< @hideinitializer */
#define FPC_ERROR_GET_EXTERNAL_GROUP(x) ((((uint32_t)(x)) & FPC_ERROR_EXTERNAL_MASK) >> FPC_ERROR_MSB_OFFSET_EXTERNAL) /**< @hideinitializer */
#define FPC_ERROR_GET_INTERNAL_GROUP(x) ((((uint32_t)(x)) & FPC_ERROR_INTERNAL_MASK) >> FPC_ERROR_MSB_OFFSET_INTERNAL) /**< @hideinitializer */
#define FPC_ERROR_GET_EXTRA_GROUP(x)    ((((uint32_t)(x)) & FPC_ERROR_EXTRA_MASK)    >> FPC_ERROR_MSB_OFFSET_EXTRA)    /**< @hideinitializer */

/**
 * @brief FPC_ERROR_SET_GROUPS macro set the supplied groups in the bit-field.
 *
 * @param[in] error     0 or 1. Set to 1 to indicate error (negative value as int32_t).
 * @param[in] module    Module Id, owner of this error.
 * @param[in] external  External code, one part of the external interface.
 * @param[in] internal  Internal code, internal to the module.
 * @param[in] extra     User defined data. Use to pass __LINE__, session data etc. @hideinitializer
 *
 * To set individual groups, see
 *
 * @ref FPC_ERROR_SET_ERROR_GROUP
 *
 * @ref FPC_ERROR_SET_MODULE_GROUP
 *
 * @ref FPC_ERROR_SET_EXTERNAL_GROUP
 *
 * @ref FPC_ERROR_SET_INTERNAL_GROUP
 *
 * @ref FPC_ERROR_SET_EXTRA_GROUP
 *
 * To get individual groups, see
 *
 * @ref FPC_ERROR_GET_ERROR_GROUP
 *
 * @ref FPC_ERROR_GET_MODULE_GROUP
 *
 * @ref FPC_ERROR_GET_EXTERNAL_GROUP
 *
 * @ref FPC_ERROR_GET_INTERNAL_GROUP
 *
 * @ref FPC_ERROR_GET_EXTRA_GROUP
 */
#define FPC_ERROR_SET_GROUPS(error, module, external, internal, extra)         \
                            (FPC_ERROR_SET_ERROR_GROUP(error)                | \
                             FPC_ERROR_SET_MODULE_GROUP(module)              | \
                             FPC_ERROR_SET_EXTERNAL_GROUP(external)          | \
                             FPC_ERROR_SET_INTERNAL_GROUP(internal)          | \
                             FPC_ERROR_SET_EXTRA_GROUP(extra))

/**
 * @brief Helper macro for legacy format errors (sets error bit and module bit).
 *
 * Legacy error codes always use the module id FPC_MODULE_ID_RESERVED
 *
 * @param[in] external  External code, one part of the external interface.
 * @param[in] internal  Internal code, internal to the module.
 * @param[in] extra     User defined data. Use to pass __LINE__, session data etc.
 *
 * @hideinitializer
 */
#define FPC_ERROR_LEGACY_FORMAT(external, internal, extra)                                   \
                               (FPC_ERROR_SET_ERROR_GROUP(1)                               | \
                                FPC_ERROR_SET_MODULE_GROUP(FPC_MODULE_ID_RESERVED)         | \
                                FPC_ERROR_SET_EXTERNAL_GROUP(external)                     | \
                                FPC_ERROR_SET_INTERNAL_GROUP(internal)                     | \
                                FPC_ERROR_SET_EXTRA_GROUP(extra))

#define FPC_ERROR_IS_LEGACY_FORMAT(fpc_error)                      \
                                  ((FPC_ERROR_GET_MODULE_GROUP(fpc_error) == FPC_MODULE_ID_RESERVED) || \
                                   (FPC_ERROR_GET_MODULE_GROUP(fpc_error) == FPC_MODULE_ID_RESERVED_MIN))

#define FPC_ERROR_IS_NEW_FORMAT(fpc_error)                         \
                               ((FPC_ERROR_GET_MODULE_GROUP(fpc_error) != FPC_MODULE_ID_RESERVED) && \
                                (FPC_ERROR_GET_MODULE_GROUP(fpc_error) != FPC_MODULE_ID_RESERVED_MIN))

#define FPC_ERROR_GET_EXTERNAL_ERROR(fpc_error)                   \
                                    (FPC_ERROR_IS_NEW_FORMAT(fpc_error) ? \
                                            fpc_error > 0 ? \
                                                    ((int32_t)FPC_ERROR_GET_EXTERNAL_GROUP(fpc_error)) \
                                                    : (-(int32_t)FPC_ERROR_GET_EXTERNAL_GROUP(fpc_error)) \
                                            : fpc_error)

#ifndef FPC_ERROR_HANDLER
/**
 * @brief Default error handler macro (pass through).
 *
 * @param[in] fpc_error  An fpc_error code.
 */
#define FPC_ERROR_HANDLER(fpc_error) (fpc_error)
#endif

#endif /* __FPC_ERROR_H__ */
