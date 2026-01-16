#include "error.h"
#include "sss/secret_sharing.h"
#include <stddef.h>

/* ========================================================================
 * Error Code to String Conversion
 * ======================================================================== */

/**
 * Convert error code to human-readable string
 */
const char* sss_error_string(int error_code) {
    switch (error_code) {
        case SSS_OK:
            return "Success";
        
        case SSS_ERR_INVALID_PARAM:
            return "Invalid parameters";
        
        case SSS_ERR_INVALID_THRESHOLD:
            return "Invalid threshold value";
        
        case SSS_ERR_INVALID_SHARES:
            return "Invalid shares";
        
        case SSS_ERR_BUFFER_TOO_SMALL:  
            return "Buffer too small";
        
        case SSS_ERR_DUPLICATE_SHARE:
            return "Duplicate share indices detected";
        
        case SSS_ERR_RECONSTRUCTION: 
            return "Secret reconstruction failed";
        
        case SSS_ERR_MEMORY:  
            return "Memory allocation failed";
        
        case SSS_ERR_CRYPTO: 
            return "Cryptographic operation failed";
        
        default: 
            return "Unknown error";
    }
}