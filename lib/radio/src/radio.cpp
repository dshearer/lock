#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <RH_NRF24.h>
#include <RHReliableDatagram.h>
#include <words.h>
#include <utils.h>
#include <hmac.h>
#include "radio.h"

/*
NOTE: Nonces are stored little-endian.
*/

/*
Max msg size is RH_NRF24_MAX_MESSAGE_LEN = 28 bytes.
*/

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
    uint8_t         digest_1[DIGEST_LEN_BYTES/2]; // 16 bytes
} __attribute__((packed)) packet_1_t; // 28 bytes

typedef struct {
    uint8_t type; // == PACKET_TYPE_2
    uint8_t digest_2[DIGEST_LEN_BYTES/2]; // 16 bytes
} __attribute__((packed)) packet_2_t; // 17 bytes

typedef struct {
    uint8_t type; // == PACKET_TYPE_ACK
} __attribute__((packed)) ack_packet_t; // 1 byte

namespace radio {

static nonce_t              g_my_latest_nonce; // 4 bytes
static nonce_t              g_other_latest_nonce; // 4 bytes
static RH_NRF24             g_driver(8, 10);
static RHReliableDatagram   g_manager(g_driver);
static uint8_t              g_buf[RH_NRF24_MAX_MESSAGE_LEN] = {0}; // 28 bytes
static packet_1_t           g_packet_1 = {0}; // 28 bytes
static packet_2_t           g_packet_2 = {0}; // 16 bytes
static const uint8_t       *g_key = NULL; // 16 bytes
static uint8_t              g_key_len = 0; // 8 bytes

#define CHECK_INTEGRITY
#define SWITCH_RADIO_MODE
// #define SEND_ACK  // adds ~2,302 ms to send operation

void setup(const uint8_t *key, uint8_t key_len, uint8_t my_id)
{
    ASSERT(key != NULL);
    g_key = key;
    g_key_len = key_len;
    // g_driver.setRF(RH_NRF24::DataRate250kbps, RH_NRF24::TransmitPower0dBm);
    g_manager.setThisAddress(my_id);
    g_manager.init();
}

int send(const msg_t *msg, uint8_t to)
{
    const unsigned long start_time = millis();

    // increment nonce
    ++g_my_latest_nonce;

    // make packet
    Serial.println("Making packets");
    memset(&g_packet_1, 0, sizeof(g_packet_1));
    memset(&g_packet_2, 0, sizeof(g_packet_2));
    g_packet_1.type = PACKET_TYPE_1;
    g_packet_2.type = PACKET_TYPE_2;
    memcpy(&g_packet_1.hashed_msg.nonce,   &g_my_latest_nonce,
        sizeof(g_packet_1.hashed_msg.nonce));
    memcpy(&g_packet_1.hashed_msg.msg, msg, sizeof(g_packet_1.hashed_msg.msg));
    
    #ifdef CHECK_INTEGRITY
    Serial.println("Computing digest");
    const unsigned digest_start_time = millis();
    const uint8_t *digest = hmac::compute(&g_packet_1.hashed_msg,
        sizeof(g_packet_1.hashed_msg), g_key, g_key_len);
    const unsigned digest_millis_diff = millis() - digest_start_time;
    Serial.print("Digest time (ms): ");
    Serial.println(digest_millis_diff);
    Serial.println("Copying digest");
    memcpy(
        g_packet_1.digest_1, 
        digest, 
        sizeof(g_packet_1.digest_1));
    memcpy(
        g_packet_2.digest_2, 
        digest + sizeof(g_packet_1.digest_1),
        sizeof(g_packet_2.digest_2));
    #endif

    // send msgs
    ASSERT(sizeof(g_packet_1) <= RH_NRF24_MAX_MESSAGE_LEN);
    if (!g_manager.sendtoWait((uint8_t *) &g_packet_1, sizeof(g_packet_1), to)) {
        Serial.println(F("sendtoWait 1 err"));
        return -1;
    }
    ASSERT(sizeof(g_packet_2) <= RH_NRF24_MAX_MESSAGE_LEN);
    if (!g_manager.sendtoWait((uint8_t *) &g_packet_2, sizeof(g_packet_2), to)) {
        Serial.println(F("sendtoWait 2 err"));
        return -1;
    }

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
    Serial.print("Send time: ");
    Serial.println(time_diff);

    return 0;
}

#define RET_ERR(_err) \
do { \
    if (error_p != NULL) { \
        *error_p = (_err); \
    } \
} while (false)

const msg_t *recv(error_t *error_p)
{
    if (!g_manager.available()) {
        return NULL;
    }

    // get packet 1
    uint8_t len = sizeof(g_buf);
    uint8_t from = 0;
    if (!g_manager.recvfromAck(g_buf, &len, &from)) {
        return NULL;
    }
    if (len != sizeof(g_packet_1)) {
        Serial.println(F("Didn't get packet 1"));
        RET_ERR(ERR_BAD_MSG);
        return NULL;
    }
    memcpy(&g_packet_1, g_buf, len);
    if (g_packet_1.type != PACKET_TYPE_1) {
        Serial.println(F("Invalid packet"));
        RET_ERR(ERR_BAD_MSG);
        return NULL;
    }

    // get packet 2
    len = sizeof(g_buf);
    if (!g_manager.recvfromAckTimeout(g_buf, &len, 5000)) {
        Serial.println(F("Never got packet 2"));
        RET_ERR(ERR_BAD_MSG);
        return NULL;
    }
    if (len != sizeof(g_packet_2)) {
        Serial.println(F("Didn't get packet 2"));
        RET_ERR(ERR_BAD_MSG);
        return NULL;
    }
    memcpy(&g_packet_2, g_buf, len);
    if (g_packet_2.type != PACKET_TYPE_2) {
        Serial.println(F("Invalid packet"));
        RET_ERR(ERR_BAD_MSG);
        return NULL;
    }

    #ifdef CHECK_INTEGRITY

    // check packet's digest
    const bool digest_ok  = 
        hmac::verify(&g_packet_1.hashed_msg, sizeof(g_packet_1.hashed_msg),
            g_key, sizeof(g_key), g_packet_1.digest_1, g_packet_2.digest_2);
    if (!digest_ok) {
        Serial.println(F("Bad digest"));
        RET_ERR(ERR_UNAUTHN);
        return NULL;
    }

    // check packet's nonce
    if (g_packet_1.hashed_msg.nonce < g_other_latest_nonce) {
        Serial.println(F("Bad nonce"));
        RET_ERR(ERR_UNAUTHN);
        return NULL;
    }

    #endif

    /* Msg's integrity is good. */

    #ifdef SEND_ACK
    // send ack
    ack_packet_t ack = {.type = PACKET_TYPE_ACK};
    if (!g_manager.sendtoWait((uint8_t *) &ack, sizeof(ack), from)) {
        Serial.println(F("Failed to send ack"));
    }
    #endif

    // update nonce
    g_other_latest_nonce = g_packet_1.hashed_msg.nonce;

    // return payload to caller
    return &g_packet_1.hashed_msg.msg;
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