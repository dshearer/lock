#ifndef __STATUS_H__
#define __STATUS_H__

typedef enum {
    STATUS_SUCCESS,
    STATUS_UNAUTHN,
    STATUS_LOCK_ERROR,
    STATUS_RADIO_ERROR,
} status_t;

#endif // __STATUS_H__