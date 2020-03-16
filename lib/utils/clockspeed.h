#ifndef __CLOCKSPEED_H__
#define __CLOCKSPEED_H__

#ifndef PROJECT
#error PROJECT not defined
#endif

#define CLOCK_PRESCALER_1   (0x0)  //  16MHz   (FYI: 15mA   on an Arduino Pro Mini @5V Vcc w. LEDs removed)
#define CLOCK_PRESCALER_2   (0x1)  //   8MHz   (FYI: 10mA   on an Arduino Pro Mini @5V Vcc w. LEDs removed)
#define CLOCK_PRESCALER_4   (0x2)  //   4MHz   (FYI:  8mA   on an Arduino Pro Mini @5V Vcc w. LEDs removed)
#define CLOCK_PRESCALER_8   (0x3)  //   2MHz   (FYI:  6.5mA on an Arduino Pro Mini @5V Vcc w. LEDs removed)
#define CLOCK_PRESCALER_16  (0x4)  //   1MHz   (FYI:  5.5mA on an Arduino Pro Mini @5V Vcc w. LEDs removed)
#define CLOCK_PRESCALER_32  (0x5)  // 500KHz   (FYI:  5mA   on an Arduino Pro Mini @5V Vcc w. LEDs removed)
#define CLOCK_PRESCALER_64  (0x6)  // 250KHz   (FYI:  4.8mA on an Arduino Pro Mini @5V Vcc w. LEDs removed)
#define CLOCK_PRESCALER_128 (0x7)  // 125KHz   (FYI:  4.8mA on an Arduino Pro Mini @5V Vcc w. LEDs removed)
#define CLOCK_PRESCALER_256 (0x8)  // 62.5KHz  (FYI:  4.6mA on an Arduino Pro Mini @5V Vcc w. LEDs removed)

static void setPrescale(uint8_t val) {
  #if PROJECT==lock
  //To ensure timed events, first turn off interrupts
  cli();                   // Disable interrupts
  CLKPR = _BV(CLKPCE);     //  Enable change. Write the CLKPCE bit to one and all the other to zero. Within 4 clock cycles, set CLKPR again
  CLKPR = val; // Change clock division. Write the CLKPS0..3 bits while writing the CLKPE bit to zero
  sei();                   // Enable interrupts
  #endif
}

#endif