#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <RH_NRF24.h>
#include <RHReliableDatagram.h>
#include <words.h>
#include <utils.h>
#include <hmac.h>
#include <mcu_safe_array.h>
#include "radio.h"

/*
NOTE: Nonces are stored little-endian.
*/

/*
Max msg size is RH_NRF24_MAX_MESSAGE_LEN = 28 bytes.
*/

using safearray::ByteArray;
using safearray::CByteSlice;

typedef Word<4> nonce_t;

typedef struct {
    nonce_t         nonce; // 4 bytes
    radio::msg_t    msg; // 7 bytes
} __attribute__((packed)) hashed_msg_t; // 11 bytes

typedef enum {
    PACKET_TYPE_1,
    PACKET_TYPE_2,
    PACKET_TYPE_ACK,
} packet_type_t;

typedef struct {
    uint8_t         type; // == PACKET_TYPE_1
    hashed_msg_t    hashed_msg; // 11 bytes
    ByteArray<DIGEST_LEN_BYTES/2>         digest_1; // 16 bytes
} __attribute__((packed)) packet_1_t; // 28 bytes

typedef struct {
    uint8_t type; // == PACKET_TYPE_2
    ByteArray<DIGEST_LEN_BYTES/2>         digest_2; // 16 bytes
} __attribute__((packed)) packet_2_t; // 17 bytes

typedef struct {
    uint8_t type; // == PACKET_TYPE_ACK
} __attribute__((packed)) ack_packet_t; // 1 byte

namespace radio {

static nonce_t              g_my_latest_nonce; // 4 bytes
static nonce_t              g_other_latest_nonce; // 4 bytes
static RH_NRF24             g_driver(8, 10);
static RHReliableDatagram   g_manager(g_driver);

#define MAX_PACKET_SIZE (sizeof(packet_1_t) > sizeof(packet_2_t) ? \
    sizeof(packet_1_t) : sizeof(packet_2_t))

static ByteArray<MAX_PACKET_SIZE>    g_buf = {};
static hashed_msg_t                  g_hashed_msg;
static Hmac                          g_hmac;
static ByteArray<DIGEST_LEN_BYTES>   g_digest = {};

static_assert(sizeof(packet_1_t) <= RH_NRF24_MAX_MESSAGE_LEN, "Bad packet 1 len");
static_assert(sizeof(packet_2_t) <= RH_NRF24_MAX_MESSAGE_LEN, "Bad packet 2 len");


#define CHECK_INTEGRITY
#define SWITCH_RADIO_MODE
// #define SEND_ACK  // adds ~2,302 ms to send operation

void setup(CByteSlice<KEY_LEN_BYTES> key, uint8_t my_id)
{
    Serial.println(F("radio::setup"));
    // g_driver.setRF(RH_NRF24::DataRate250kbps, RH_NRF24::TransmitPower0dBm);
    g_manager.setThisAddress(my_id);
    g_manager.init();
    g_hmac.setKey(key);

    Hmac::implOkay();

    // Serial.print("Pkt 1 len: ");
    // Serial.println(sizeof(packet_1_t));
    // Serial.print("Pkt 2 len: ");
    // Serial.println(sizeof(packet_2_t));
    // Serial.print("Key len: ");
    // Serial.println(key.size());
}

int send(const msg_t *msg, uint8_t to)
{
    const unsigned long start_time = millis();

    // increment nonce
    ++g_my_latest_nonce;

    // make packet 1
    Serial.println(F("Making packets"));
    Serial.flush();
    packet_1_t *pack_1 = safearray::cast<packet_1_t>(g_buf);
    pack_1->type = PACKET_TYPE_1;
    memcpy(&pack_1->hashed_msg.nonce, &g_my_latest_nonce,
        sizeof(pack_1->hashed_msg.nonce));
    memcpy(&pack_1->hashed_msg.msg, msg, sizeof(pack_1->hashed_msg.msg));
    #ifdef CHECK_INTEGRITY
    g_hmac.init() << &pack_1->hashed_msg >> g_digest;
    pack_1->digest_1 << g_digest.cslice<0, DIGEST_LEN_BYTES/2>();
    #endif

    // send packet 1
    Serial.println(F("Sending pkt 1"));
    Serial.flush();
    if (!g_manager.sendtoWait((uint8_t *) pack_1, sizeof(*pack_1), to)) {
        Serial.println(F("sendtoWait 1 err"));
        return -1;
    }

    // make packet 2
    Serial.println(F("Sending pkt 2"));
    Serial.flush();
    packet_2_t *pack_2 = safearray::cast<packet_2_t>(g_buf);
    pack_2->type = PACKET_TYPE_2;
    pack_2->digest_2 << g_digest.cslice<DIGEST_LEN_BYTES/2>();

    // send packet 2
    if (!g_manager.sendtoWait((uint8_t *) pack_2, sizeof(*pack_2), to)) {
        Serial.println(F("sendtoWait 2 err"));
        return -1;
    }
    
    // Serial.println("Computing digest");
    // const unsigned digest_start_time = millis();
    // const unsigned digest_millis_diff = millis() - digest_start_time;
    // Serial.print(F("Digest time (ms): "));
    // Serial.println(digest_millis_diff);
    // Serial.println("Copying digest");
    // Serial.println("Half-Done digest");
    // Serial.println("Done digest");

    #ifdef SEND_ACK
    // listen for ack
    uint8_t len = sizeof(g_buf);
    if (!g_manager.recvfromAckTimeout(g_buf, &len, 5000)) {
        Serial.println(F("Never got ack"));
        return -1;
    }
    ack_packet_t ack = {0};
    if (len != sizeof(ack)) {
        Serial.println(F("Got something other than ack"));
        return -1;
    }
    memcpy(&ack, g_buf, len);
    if (ack.type != PACKET_TYPE_ACK) {
        Serial.println(F("Invalid ack packet"));
        return -1;
    }
    #endif

    const unsigned long time_diff = millis() - start_time;
    Serial.print(F("Send time: "));
    Serial.println(time_diff);

    return 0;
}

#define RET_ERR(_err) \
do { \
    if (error_p != NULL) { \
        *error_p = (_err); \
    } \
} while (false)

template<typename T>
static const T *interp_buffer(size_t len, packet_type_t ptype) {
    if (len != sizeof(T)) {
        Serial.print(F("Wrong size for packet "));
        Serial.print(ptype);
        Serial.print(F(": "));
        Serial.print(len);
        Serial.print(F(" vs "));
        Serial.println(sizeof(T));
        return NULL;
    }
    const T* packet = safearray::cast<T>(g_buf);
    if (packet->type != ptype) {
        Serial.print(F("Wrong type for packet "));
        Serial.println(ptype);
        return NULL;
    }
    return packet;
}

const msg_t *recv(error_t *error_p)
{
    if (!g_manager.available()) {
        return NULL;
    }
    Serial.println(F("recv"));

    // get packet 1
    uint8_t len = sizeof(g_buf);
    uint8_t from = 0;
    if (!g_manager.recvfromAck((uint8_t *) g_buf.data(), &len, &from)) {
        return NULL;
    }
    const packet_1_t *pack_1 = interp_buffer<packet_1_t>(len, PACKET_TYPE_1);
    if (pack_1 == NULL) {
        RET_ERR(ERR_BAD_MSG);
        return NULL;
    }
    Serial.println(F("Got pkt 1"));
    Serial.flush();

    bool badDigest = false;

    // save important stuff from packet 1
    memcpy(&g_hashed_msg, &pack_1->hashed_msg, sizeof(g_hashed_msg));

    #ifdef CHECK_INTEGRITY
    // start checking digest
    g_hmac.init() << &pack_1->hashed_msg >> g_digest;
    if (!Hmac::digestsEqual(g_digest.cslice<0, DIGEST_LEN_BYTES/2>(),
        pack_1->digest_1.cslice())) {
        badDigest = true;
    }
    #endif

    // get packet 2
    len = sizeof(g_buf);
    if (!g_manager.recvfromAckTimeout((uint8_t *) g_buf.data(), &len, 5000)) {
        Serial.println(F("Never got packet 2"));
        RET_ERR(ERR_BAD_MSG);
        return NULL;
    }
    const packet_2_t *pack_2 = interp_buffer<packet_2_t>(len, PACKET_TYPE_2);
    if (pack_2 == NULL) {
        RET_ERR(ERR_BAD_MSG);
        return NULL;
    }
    Serial.println(F("Got pkt 2"));

    #ifdef CHECK_INTEGRITY
    // finish checking digest
    if (!Hmac::digestsEqual(g_digest.cslice<DIGEST_LEN_BYTES/2>(),
        pack_2->digest_2.cslice())) {
        badDigest = true;
    }
    #endif

    // check packet's digest
    if (badDigest) {
        Serial.println(F("Bad digest"));
        RET_ERR(ERR_UNAUTHN);
        return NULL;
    }
    Serial.println(F("Digest ok"));

    // check packet's nonce
    if (g_hashed_msg.nonce < g_other_latest_nonce) {
        Serial.println(F("Bad nonce"));
        RET_ERR(ERR_UNAUTHN);
        return NULL;
    }
    Serial.println(F("Nonce ok"));

    /* Msg's integrity is good. */

    #ifdef SEND_ACK
    // send ack
    ack_packet_t ack = {.type = PACKET_TYPE_ACK};
    if (!g_manager.sendtoWait((uint8_t *) &ack, sizeof(ack), from)) {
        Serial.println(F("Failed to send ack"));
    }
    #endif

    // update nonce
    g_other_latest_nonce = g_hashed_msg.nonce;

    // return payload to caller
    return &g_hashed_msg.msg;
}

const msg_t *recvTimeout(uint8_t timeout, error_t *error_p)
{
    const unsigned long end_time = millis() + (timeout * 1000UL);
    error_t local_err = ERR_NULL;
    while (millis() < end_time)
    {
        const msg_t *msg = recv(&local_err);
        if (msg == NULL && local_err == ERR_NULL) {
            continue;
        }

        /* Got msg or error */
        RET_ERR(local_err);
        return msg;
    }

    /* Timeout */
    RET_ERR(ERR_TIMEOUT);
    return NULL;
}

void setModeIdle()
{
    #ifdef SWITCH_RADIO_MODE
    g_driver.setModeIdle();
    #endif
}

void setModeRx()
{
    #ifdef SWITCH_RADIO_MODE
    g_driver.setModeRx();
    #endif
}

}