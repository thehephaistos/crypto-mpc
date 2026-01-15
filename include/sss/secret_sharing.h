#ifndef SSS_SECRET_SHARING_H
#define SSS_SECRET_SHARING_H

#include <stdint.h>
#include <stddef.h>
#include <sodium.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Constants
 * ======================================================================== */

#define SSS_MAX_SECRET_SIZE 1024
#define SSS_MAX_SHARES 255
#define SSS_MIN_THRESHOLD 2
#define SSS_SHARE_DATA_SIZE 32

/* ========================================================================
 * Error Codes
 * ======================================================================== */

typedef enum {
    SSS_OK = 0,
    SSS_ERR_INVALID_PARAM = -1,
    SSS_ERR_INVALID_THRESHOLD = -2,
    SSS_ERR_INVALID_SHARES = -3,
    SSS_ERR_BUFFER_TOO_SMALL = -4,
    SSS_ERR_DUPLICATE_SHARE = -5,
    SSS_ERR_RECONSTRUCTION = -6,
    SSS_ERR_MEMORY = -7,
    SSS_ERR_CRYPTO = -8
} sss_error_t;

/* ========================================================================
 * Data Structures
 * ======================================================================== */

typedef struct {
    uint8_t index;
    uint8_t threshold;
    uint8_t data[SSS_SHARE_DATA_SIZE];
    size_t data_len;
} sss_share_t;

typedef struct {
    uint8_t threshold;
    uint8_t num_shares;
    size_t secret_len;
} sss_context_t;

/* ========================================================================
 * Core API Functions
 * ======================================================================== */

int sss_init(void);

int sss_create_shares(
    const uint8_t *secret,
    size_t secret_len,
    uint8_t threshold,
    uint8_t num_shares,
    sss_share_t *shares
);

int sss_combine_shares(
    const sss_share_t *shares,
    uint8_t num_shares,
    uint8_t *secret,
    size_t *secret_len
);

int sss_validate_share(const sss_share_t *share);

const char* sss_strerror(int error_code);

void sss_wipe_share(sss_share_t *share);

void sss_wipe_memory(void *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SSS_SECRET_SHARING_H */