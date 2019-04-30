#include <string.h>
#include <stdio.h>
#include <Arduino.h>

#include "hal_arduino_Wire.h"

#include "../atca_command.h"
#include "atca_hal.h"
#include "atca_device.h"
#include "hal_arduino.h"
#include "../basic/atca_basic.h"
#include "print.h"

#define DEBUG_LOG( x )

#ifndef NUM_ELEMS
#  define NUM_ELEMS(_array) (sizeof(_array)/sizeof((_array)[0]))
#endif

// #define WIRE_BUF_MAX (SERIAL_BUFFER_SIZE-1)
#define WIRE_BUF_MAX (BUFFER_LENGTH-1)

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

static uint8_t g_buff[WIRE_BUF_MAX] = {0};

extern "C" ATCA_STATUS hal_i2c_init(void *hal, ATCAIfaceCfg *cfg);
extern "C" ATCA_STATUS hal_i2c_wake(ATCAIface iface);
extern "C" ATCA_STATUS hal_i2c_idle(ATCAIface iface);
extern "C" ATCA_STATUS hal_i2c_release(void *hal_data);


static bool wake_complete = false;

// static uint8_t revs508[1][4] = { { 0x00, 0x00, 0x50, 0x00 } };
// static uint8_t revs108[2][4] = { { 0x80, 0x00, 0x10, 0x01 },
// 				 { 0x00, 0x00, 0x10, 0x05 } };
// static uint8_t revs204[3][4] = { { 0x00, 0x02, 0x00, 0x08 },
// 				 { 0x00, 0x02, 0x00, 0x09 },
// 				 { 0x00, 0x04, 0x05, 0x00 } };

struct DevRev
{
  uint8_t rev[4];
  ATCADeviceType type;
};

static const DevRev g_devrevs[] = {
  {{ 0x00, 0x00, 0x50, 0x00 }, ATECC508A},
  {{ 0x80, 0x00, 0x10, 0x01 }, ATECC108A},
  {{ 0x00, 0x00, 0x10, 0x05 }, ATECC108A},
  {{ 0x00, 0x02, 0x00, 0x08 }, ATSHA204A},
  {{ 0x00, 0x02, 0x00, 0x09 }, ATSHA204A},
  {{ 0x00, 0x04, 0x05, 0x00 }, ATSHA204A},
};

extern "C" void debug_out(const char *msg, int arg) {
  Serial.print(msg);
  Serial.println(arg,HEX);
}

extern "C" ATCA_STATUS hal_i2c_discover_buses(int *buses, int buses_len)
{
  if (buses_len < 1) {
    return ATCA_SUCCESS;
  }
  buses[0] = 2;
  for (int i = 1; i < buses_len; ++i) {
    buses[i] = -1;
  }
  return ATCA_SUCCESS;
}

extern "C" ATCA_STATUS hal_i2c_discover_devices(int busNum, ATCAIfaceCfg *cfg, int *found)
{
  Serial.println("hal_i2c_discover_devices");
	ATCAIfaceCfg *head = cfg;
	// uint8_t slaveAddress = 0x01;
	// ATCADevice device;
	// ATCAIface discoverIface;
	// ATCACommand command;
	// ATCAPacket packet;
	// uint32_t execution_time;
	// ATCA_STATUS status;
	// int i;

	ATCAIfaceCfg discoverCfg;
	discoverCfg.iface_type				= ATCA_I2C_IFACE;
	discoverCfg.devtype				= ATECC508A;
	discoverCfg.wake_delay				= 800;
	discoverCfg.rx_retries				= 3;
	discoverCfg.atcai2c = {
	  .slave_address	= 0x07,
	  .bus			= (uint8_t) busNum,
	  .baud			= 400000
	};

	ATCAHAL_t hal;

	// hal_i2c_init( &hal, &discoverCfg );
	// device = newATCADevice( &discoverCfg );
	// discoverIface = atGetIFace( device );
	// command = atGetCommands( device );

	*found = 0;
	// for ( slaveAddress = 0x07; slaveAddress <= 0x78; slaveAddress++ ) {
	// 	discoverCfg.atcai2c.slave_address = slaveAddress << 1;

	// 	if ( hal_i2c_wake( discoverIface ) != ATCA_SUCCESS ) {
  //     continue;
  //   }

    (*found)++;
    memcpy(head, &discoverCfg, sizeof(ATCAIfaceCfg));
    head->devtype = ATCA_DEV_UNKNOWN;

    // memset( packet.data, 0x00, sizeof(packet.data));

    // atInfo( command, &packet );
    // execution_time = atGetExecTime(command, CMD_INFO) + 1;

    // if ( (status = atsend( discoverIface, (uint8_t*)&packet, packet.txsize )) != ATCA_SUCCESS ) {
    //   Serial.println("packet send error");
    //   continue;
    // }

    // atca_delay_ms(execution_time);

    // if ( (status = atreceive( discoverIface, &(packet.data[0]), &(packet.rxsize) )) != ATCA_SUCCESS )
    //   continue;

    // if ( (status = isATCAError(packet.data)) != ATCA_SUCCESS ) {
    //   Serial.println("command response error\r\n");
    //   continue;
    // }

    uint8_t buff[4] = {0};
    if (atcab_info(buff) != ATCA_SUCCESS) {
      Serial.println("atcab_info failed");
      return ATCA_COMM_FAIL;
    }

    for (size_t i = 0 ; i < NUM_ELEMS(g_devrevs); ++i) {
      if (memcmp(buff, g_devrevs[i].rev, sizeof(buff)) == 0) {
        head->devtype = g_devrevs[i].type;
        break;
      }
    }

    // atca_delay_ms(15);
    head++;
  // } // for

  // hal_i2c_idle(discoverIface);

	hal_i2c_release(&hal);

	return ATCA_SUCCESS;
}

extern "C" ATCA_STATUS hal_i2c_init(void *hal, ATCAIfaceCfg *cfg)
{
    Wire.begin();
    // Wire.setClock(900000);
	  return ATCA_SUCCESS;
}

extern "C" ATCA_STATUS hal_i2c_post_init(ATCAIface iface)
{
	  return ATCA_SUCCESS;
}

// int send_oversized(uint8_t txAddress, uint8_t *data, int size)
// {
  // if (!PERIPH_WIRE.startTransmissionWIRE(txAddress, WIRE_WRITE_FLAG)) {
  //   PERIPH_WIRE.prepareCommandBitsWire(WIRE_MASTER_ACT_STOP);
  //   return 2 ;  // Address error
  // }

  // for (int i = 0; i < size; i++) {
  //   if (!PERIPH_WIRE.sendDataMasterWIRE(data[i])) {
  //     PERIPH_WIRE.prepareCommandBitsWire(WIRE_MASTER_ACT_STOP);
  //     return 3 ;  // Nack or error
  //   }
  // }

  // PERIPH_WIRE.prepareCommandBitsWire(WIRE_MASTER_ACT_STOP);
// }

extern "C" ATCA_STATUS hal_i2c_send(ATCAIface iface, uint8_t *txdata, int txlength)
{
  int res;
  ATCAIfaceCfg *cfg = atgetifacecfg(iface);

  DEBUG_LOG( Serial.print(cfg->atcai2c.slave_address >> 1, HEX); )
  DEBUG_LOG( Serial.print(" > "); )

  txdata[0] = 0x03;   // insert the Word Address Value, Command token
  txlength++;

  if (txlength <= WIRE_BUF_MAX) {
    Wire.beginTransmission(cfg->atcai2c.slave_address >> 1);
    for (uint8_t i = 0; i < txlength; i++) {
      if (!Wire.write(txdata[i])) {
	      Serial.println("Wire.write overflow");
      }
    }
    res = Wire.endTransmission(true);
  } else {
      // res = send_oversized(cfg->atcai2c.slave_address >> 1, txdata, txlength);

      Serial.println("Msg is too big for Wire lib's buffer");
      Serial.print(txlength);
      Serial.print(" vs ");
      Serial.println(WIRE_BUF_MAX);
      res = ATCA_COMM_FAIL;
  }
  if (res != 0) {
    return ATCA_COMM_FAIL;
  }

  DEBUG_LOG( Serial.println(txlength); )

  return ATCA_SUCCESS;
}

extern "C" ATCA_STATUS hal_i2c_receive( ATCAIface iface, uint8_t *rxdata, uint16_t *rxlength)
{
  ATCAIfaceCfg *cfg = atgetifacecfg(iface);
  int retries = cfg->rx_retries;

  DEBUG_LOG( Serial.print(cfg->atcai2c.slave_address >> 1, HEX); )
  DEBUG_LOG( Serial.print(" < "); )

  // Get length of response
  while (Wire.requestFrom(cfg->atcai2c.slave_address >> 1, 1) == 0 &&
	 retries > 0) {
    delay(100);
    retries--;
  }
  if (retries < cfg->rx_retries) {
    DEBUG_LOG( Serial.print(cfg->rx_retries - retries); )
    DEBUG_LOG( Serial.print( " retries"); )
  }
  if (retries == 0) {
    Serial.println("no reponse error");
    return ATCA_COMM_FAIL;
  }
  if (Wire.available() != 1) {
    Serial.print("invalid response ");
    return ATCA_COMM_FAIL;
  }
  rxdata[0] = Wire.read();

  uint8_t bufpos = 1;
  uint8_t todo = rxdata[0]-1;
  while (todo > 0) {
    uint8_t n = todo <= WIRE_BUF_MAX ? todo : WIRE_BUF_MAX;
    Wire.requestFrom((uint8_t) (cfg->atcai2c.slave_address >> 1), n);
    if (Wire.available() != n) {
      Serial.println("response size error");
      return ATCA_COMM_FAIL;
    }
    for (uint8_t i = 0; i < n && bufpos < *rxlength; i++, bufpos++) {
      rxdata[bufpos] = Wire.read();
      DEBUG_LOG( Serial.print(":"); )
      DEBUG_LOG( Serial.print(rxdata[bufpos], HEX); )
    }
    todo -= n;
  }
  DEBUG_LOG( Serial.println(); )

  return ATCA_SUCCESS;
}

extern "C" ATCA_STATUS hal_i2c_wake(ATCAIface iface)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    int retries = cfg->rx_retries;

    DEBUG_LOG( Serial.print("wake "); )
    DEBUG_LOG( Serial.println(cfg->atcai2c.slave_address >> 1, HEX); )

    Wire.beginTransmission(0x00);
    Wire.endTransmission(true);

    atca_delay_us(cfg->wake_delay);

    while (Wire.requestFrom(cfg->atcai2c.slave_address >> 1, 4) == 0 && retries > 0) {
        delay(100);
        Serial.println("Retrying");
        retries--;
    }
    if (retries <= 0) {
        Serial.println("no response");
        return ATCA_COMM_FAIL;
    }

    if (wake_complete) {
        return ATCA_SUCCESS;
    }
    wake_complete = true;

    // read response
    const int len = Wire.available();
    if (len > (int) sizeof(g_buff)) {
        Serial.print("Response too big: ");
        Serial.println(len);
        return ATCA_COMM_FAIL;
    }
    Wire.readBytes(g_buff, len);

    // check response
    if (hal_check_wake(g_buff, len) != ATCA_SUCCESS) {
        Serial.print("incorrect response ");
        for (int i = 0; i < len; i++) {
            Serial.print(":");
            Serial.print(g_buff[i], HEX);
        }
        Serial.println("");
        return ATCA_COMM_FAIL;
    }

    DEBUG_LOG(Serial.println("Wake successful");)
    return ATCA_SUCCESS;
}

extern "C" ATCA_STATUS hal_i2c_idle(ATCAIface iface)
{
  ATCAIfaceCfg *cfg = atgetifacecfg(iface);

  DEBUG_LOG( Serial.print("idle "); )
  DEBUG_LOG( Serial.println(cfg->atcai2c.slave_address >> 1, HEX); )

  Wire.beginTransmission(cfg->atcai2c.slave_address >> 1);
  Wire.write(0x02); // idle word address value
  Wire.endTransmission(true);
  wake_complete = false;

  return ATCA_SUCCESS;
}

extern "C" ATCA_STATUS hal_i2c_sleep(ATCAIface iface)
{
  ATCAIfaceCfg *cfg = atgetifacecfg(iface);

  DEBUG_LOG( Serial.print("sleep "); )
  DEBUG_LOG( Serial.println(cfg->atcai2c.slave_address >> 1, HEX); )

  Wire.beginTransmission(cfg->atcai2c.slave_address >> 1);
  Wire.write(0x01); // sleep word address value
  Wire.endTransmission(true);
  wake_complete = false;

  return ATCA_SUCCESS;
}

extern "C" ATCA_STATUS hal_i2c_release( void *hal_data )
{
  DEBUG_LOG(Serial.println("hal_i2c_release");)
  return ATCA_SUCCESS;
}

extern "C" void atca_delay_ms(uint32_t ms)
{
	delay(ms);
}

extern "C" void atca_delay_us(uint32_t us)
{
	delayMicroseconds(us);
}