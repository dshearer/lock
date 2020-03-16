#ifndef __RADIO_H__
#define __RADIO_H__

#include <stddef.h>
#include <inttypes.h>
#include <mcu_safe_array.h>

/*
A radio using the 2.4GHz band.

Uses nRF24L01 module.  https://www.itead.cc/wiki/NRF24L01_Module
This module's max power is +20dBm

FCC info:
- https://www.sparkfun.com/tutorials/398
- Cf. §15.23   Home-built devices.
*/

#define LOCK_ID        1
#define REMOTE_ID      2

#define KEY_LEN_BYTES 16

namespace radio {

typedef enum {
    REMOTE_MSG_ENGAGE = 1,
    REMOTE_MSG_DISENGAGE,
    LOCK_MSG_SUCCESS,
    LOCK_MSG_UNAUTHN,
    LOCK_MSG_FAILURE,
} msg_code_t;

typedef struct {
    uint8_t code; // must be one of msg_code_t
    uint8_t unused[6];
} __attribute__((packed)) msg_t; // must be 7 bytes

typedef enum {
    ERR_NULL = 0,
    ERR_BAD_MSG,
    ERR_UNAUTHN,
    ERR_TIMEOUT,
} error_t;

void setup(safearray::CByteSlice<KEY_LEN_BYTES> key, uint8_t my_id);

/*
Send message.

@return 0 on success; non-0 if we never got an ACK from the other device.
*/
int send(const msg_t *msg, uint8_t to);

/*
Receive an authentic message.  Does not block: if no message is available,
returns immediately.

@param error_p If a message is available but it can't be parsed or authenticated,
an appropriate code is written here and NULL is returned.  (May be NULL.)

@return A message on success; NULL if no message is available or an error occurs.
*/
const msg_t *recv(error_t *error_p);

/*
Receive an authentic message.  Blocks: won't return until a message is available
or timeout has occurred.

@param timeout Max time to wait (milliseconds).
@param error_p If a message is available but it can't be parsed or authenticated,
an appropriate code is written here and NULL is returned.  (May be NULL.)

@return A message on success; NULL if no message is available, timeout occurs,
or an error occurs.
*/
const msg_t *recvTimeout(uint16_t timeout, error_t *error_p);

void setModeIdle();
void setModeRx();

} // namespace

#endif /* __RADIO_H__ */