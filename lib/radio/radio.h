#ifndef __RADIO_H__
#define __RADIO_H__

#include <stddef.h>
#include <inttypes.h>
#include <RH_NRF24.h>
#include <RHReliableDatagram.h>

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

namespace radio {

typedef enum {
    REMOTE_MSG_ENGAGE,
    REMOTE_MSG_DISENGAGE,
    LOCK_MSG_SUCCESS,
    LOCK_MSG_UNAUTHN,
    LOCK_MSG_FAILURE,
} msg_code_t;

typedef struct {
    uint8_t code; // must be one of msg_code_t
    uint8_t unused[6];
} __attribute__((packed)) msg_t; // must be 7 bytes

void setup(const uint8_t *key, uint8_t key_len, uint8_t my_id);

/*
Send message.

@return 0 on success; non-0 on failure.
*/
int send(const msg_t *msg, uint8_t to);

/*
Receive a message.  If no message is available, returns NULL immediately.

@return A message on success; NULL on failure.
*/
const msg_t *recv();

void setModeIdle();
void setModeRx();

} // namespace

#endif /* __RADIO_H__ */