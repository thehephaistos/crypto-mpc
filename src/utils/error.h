#ifndef SSS_UTILS_ERROR_H
#define SSS_UTILS_ERROR_H

#include "sss/secret_sharing.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Error Handling Utilities
 * ======================================================================== */

/**
 * Convert error code to human-readable string
 * 
 * @param error_code The error code
 * @return String description
 */
const char* sss_error_string(int error_code);

/**
 * Check if error code indicates success
 * 
 * @param error_code The error code
 * @return 1 if success, 0 if error
 */
static inline int sss_is_success(int error_code) {
    return error_code == SSS_OK;
}

/**
 * Check if error code indicates failure
 * 
 * @param error_code The error code
 * @return 1 if error, 0 if success
 */
static inline int sss_is_error(int error_code) {
    return error_code != SSS_OK;
}

#ifdef __cplusplus
}
#endif

#endif 

/* SSS_UTILS_ERROR_H */