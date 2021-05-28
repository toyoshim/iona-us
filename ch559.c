// Copyright 2021 Takashi Toyoshima <toyoshim@gmail.com>. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ch559.h"

#include <compiler.h>
#include <stdio.h>

SFR(P0, 0x80);  // P0 input/output register
SFR(PCON, 0x87);  // Power control register
SFR(TMOD, 0x89);  // Timer/counter/0/1 mode register
SFR(TH1, 0x8d);	 // High byte of timer1 count
SFR(P1, 0x90);  // P1 input/output register
SFR(SER1_IER, 0x91);  // UART1 interrupt enable
SFR(SER1_DLM, 0x91);  // UART1 divisor latch MSB byte, only when DLAB=1
SFR(SER1_FCR, 0x92);  // UART1 FIFO control
SFR(SER1_LCR, 0x93);  // UART1 line control
SFR(SER1_MCR, 0x94);  // UART1 modem control
SFR(SER1_LSR, 0x95);  // UART1 line status
SFR(SER1_DIV, 0x97);  // UART1 pre-divisor latch byte, only when DLAB=1
SFR(SBUF, 0x99);  // UART0 data buffer register
SFR(SER1_FIFO, 0x9a);  // UART1 FIFO register
SFR(SER1_DLL, 0x9a);  // UART1 divisor latch LSB byte, only when DLAB=1
SFR(PWM_DATA, 0x9c);  // PWM1 data register
SFR(PWM_CTRL, 0x9d);  // PWM control register
SFR(PWM_CK_SE, 0x9e);  // PWM clock divisor register
SFR(PWM_CYCLE, 0x9f);  // PWM cycle period register
SFR(P2, 0xa0);  // P2 input/output register
SFR(SAFE_MOD, 0xa1);  // Safe mode control register
SFR(P3, 0xb0);  // P3 input/output register
SFR(CLOCK_CFG, 0xb3);  // System clock configuration register
SFR(P1_DIR, 0xba);  // P1 direction control register
SFR(P1_PU, 0xbb);  // P1 pull-up enable register
SFR(P2_DIR, 0xbc);  // P2 direction control register
SFR(P2_PU, 0xbd);  // P2 pull-up enable register
SFR(P3_DIR, 0xbe);  // P3 direction control register
SFR(P3_PU, 0xbf);  // P3 pull-up enable register
SFR(P4_OUT, 0xc0);  // P4 output register
SFR(P4_DIR, 0xc2);  // P4 direction control register
SFR(P0_DIR, 0xc4);  // P0 direction control register
SFR(P0_PU, 0xc5);  // P0 pull-up enable register
SFR(PORT_CFG, 0xc6);  // Port configuration register
SFR(T2MOD, 0xc9);  // Timer2 mode register
SFR(PIN_FUNC, 0xce);  // Function pins select register
SFR(RESET_KEEP, 0xfe);  // Reset-keeping register

SBIT(TR1, 0x88, 6);  // TCON - Timer1 start/stop bit
SBIT(SM0, 0x98, 7);  // SCON - UART0 mode bit0, selection data bit
SBIT(SM1, 0x98, 6);  // SCON - UART0 mode bit1, selection baud rate
SBIT(TI, 0x98, 1);  // SCON - Transmit interrupt flag

enum {
  SMOD = 0x80,  // PCON, Baud rate selection for UART0 mode 1/2/3
  bT1_M1 = 0x20,  // TMOD, Timer1 mode high bit
  bIER_PIN_MOD1 = 0x20,  // SER1_IER, UART1 pin mode high bit
  bFCR_FIFO_EN = 0x01,  // SER1_FCR, UART1 FIFO enable
  bFCR_R_FIFO_CLR = 0x02,  // SER1_FCR, UART1 receiver FIFO clear
  bFCR_T_FIFO_CLR = 0x04,  // SER1_FCR, UART1 transmitter FIFO clear
  bLCR_WORD_SZ0 = 0x01,  // SER1_LCR, UART1 word bit length low bit
  bLCR_WORD_SZ1 = 0x02,  // SER1_LCR, UART1 word bit length high bit
  bLCR_DLAB = 0x80,  // SER1_LCR, UART1 divisor latch access bit enable
  bMCR_HALF = 0x80,  // SE1_MCR, UART1 enable half-duplex mode
  bLSR_T_FIFO_EMP = 0x20,  // SER1_LSR, UART1 transmitter FIFO empty status
  bLSR_DATA_RDY = 0x01,  // SER1_LSR, UART1 receiver FIFO data ready status
  bPWM_CLR_ALL = 0x02,  // PWM_CTRL, clear FIFO and count of PWM1/2
  bPWM_OUT_EN = 0x08,  // PWM_CTRL, PWM1 output enable
  MASK_SYS_CK_DIV = 0x1f,  // CLOCK_CFG, system clock divisor factor
  bTMR_CLK = 0x80,  // T2MOD, Fastest internal clock mode for timer 0/1/2
  bT1_CLK = 0x20,  // T2MOD, Timer1 internal clock frequency selection
  bUART0_PIN_X = 0x10,  // PIN_FUNC, Pin UART0 mapping enable bit
  bPWM1_PIN_X = 0x80,  // PIN_FUNC, Pin PWM1/PWM2 mapping enable bit
};

void (*runBootloader)() = 0xf400;

int putchar(int c) {
  while (!TI);
  TI = 0;
  SBUF = c & 0xff;
  return c;
}

static void delayU8us(uint8_t us) {
  us;
__asm
  mov r7,dpl
loop1$:
  mov a,#8
loop2$:
  dec a 
  jnz loop2$
  nop
  nop
  dec r7
  mov a,r7
  jnz loop1$
__endasm;
}

static inline void enter_safe_mode() {
  SAFE_MOD = 0x55;
  SAFE_MOD = 0xaa;
}

static inline void leave_safe_mode() {
  SAFE_MOD = 0;
}

static char U4ToHex(uint8_t val) {
  if (val < 10)
    return '0' + val;
  return 'a' + val - 10;
}

struct SerialLibrary Serial;

static void s_putc(uint8_t val) {
  putchar(val);
}

static void s_printc(uint8_t val, uint8_t type) {
  if (type == BIN) {
    for (int i = 0x80; i; i >>= 1)
      Serial.putc((val & i) ? '1' : '0');
  } else if (type == HEX) {
    if (16 <= val)
      Serial.putc(U4ToHex(val >> 4));
    else
      Serial.putc('0');
    Serial.putc(U4ToHex(val & 0x0f));
  }
}

static void s_print(const char* val) {
  while (*val)
    Serial.putc(*val++);
}

static void s_println(const char* val) {
  Serial.print(val);
  Serial.print("\r\n");
}

void initialize() {
  // Clock
  // Fosc = 12MHz, Fpll = 288MHz, Fusb4x = 48MHz by PLL_CFG default
  enter_safe_mode();
  CLOCK_CFG = (CLOCK_CFG & ~MASK_SYS_CK_DIV) | 6;  // Fsys = 288MHz / 6 = 48MHz
  leave_safe_mode();

  // UART0 115200 TX at P0.3
  P0_DIR |= 0x08;  // Set P0.3(TXD) as output
  P0_PU |= 0x08;  // Pull-up P0.3(TXD)
  PIN_FUNC |= bUART0_PIN_X;  // RXD0/TXD0 enable P0.2/P0.3

  SM0 = 0;  // 8-bits data
  SM1 = 1;  // variable baud rate, based on timer

  TMOD |= bT1_M1;  // Timer1 mode2
  T2MOD |= bTMR_CLK | bT1_CLK;  // use original Fsys, timer1 faster clock
  PCON |= SMOD;  // fast mode
  TH1 = 230;  // 256 - Fsys(48M) / 16 / baudrate(115200)

  TR1 = 1;  // Start timer1
  TI = 1;  // Set transmit interrupt flag for the first transmit

  // GPIO
  PORT_CFG = 0x00;  // 5mA push-pull for port 0-3 by default

  // SerialLibrary
  Serial.putc = s_putc;
  Serial.printc = s_printc;
  Serial.print = s_print;
  Serial.println = s_println;

  if (RESET_KEEP) {
    RESET_KEEP = 0;
    Serial.println("bootloader");
    runBootloader();
  }
  RESET_KEEP = 1;
}

void rs485_init() {
  SER1_IER |= bIER_PIN_MOD1;  // Use XA/XB

  SER1_LCR |= bLCR_DLAB;  // Allow SER1_DIV, SER1_DLM, and SER1_DLL use
  SER1_DIV = 1;
  // { SER1_DLM, SER1_DLL } = Fsys(48M) * 2 / SER1_DIV / 16 / baudrate(115200)
  SER1_DLM = 0;
  SER1_DLL = 52;  // should be set before enabling FIFO
  SER1_LCR &= ~bLCR_DLAB;

  // no parity, stop bit 1-bit, no interrupts by default
  SER1_LCR |= bLCR_WORD_SZ0 | bLCR_WORD_SZ1;  // data length 8-bits
  SER1_MCR |= bMCR_HALF;  // enable half-duplex mode
  SER1_FCR = bFCR_FIFO_EN;  // Enable FIFO
}

void rs485_send(uint8_t val) {
  while (!(SER1_LSR & bLSR_T_FIFO_EMP));
  SER1_FIFO = val;
}

bool rs485_ready() {
  return (SER1_LSR & bLSR_DATA_RDY) != 0;
}

uint8_t rs485_recv() {
  while (!rs485_ready());
  return SER1_FIFO;
}

void pwm1_init() {
  // Use P4.3
  PIN_FUNC |= bPWM1_PIN_X;
  P4_DIR |= (1 << 3);
  P4_OUT &= ~(1 << 3);

  // Clock divisor
  PWM_CK_SE = 1;  // Fsys(48M) / 1

  // Clear FIFO and count
  PWM_CTRL &= ~bPWM_CLR_ALL;

  // Enable
  PWM_CTRL |= bPWM_OUT_EN;

  // PWM cycle = 2t, duty = 1:1
  PWM_CYCLE = 2;
  PWM_DATA = 1;
}

void pwm1_enable(bool enable) {
  if (enable)
    PWM_CTRL |= bPWM_OUT_EN;
  else
    PWM_CTRL &= ~bPWM_OUT_EN;
}

void delayMicroseconds(uint32_t us) {
  while (us > 255) {
    delayU8us(255);
    us -= 255;
  }
  delayU8us(us & 0xff);
}

void delay(uint32_t ms) {
  for (uint32_t i = 0; i < ms; ++i)
    delayMicroseconds(1000);
}

void pinMode(uint8_t port, uint8_t bit, uint8_t mode) {
  uint8_t mask = 1 << bit;
  if (mode == INPUT_PULLUP) {
    switch (port) {
      case 0:
        P0_PU |= mask;
        break;
      case 1:
        P1_PU |= mask;
        break;
      case 2:
        P2_PU |= mask;
        break;
      case 3:
        P3_PU |= mask;
        break;
      default:
        Serial.println("N/A");
        break;
    }
  } else {
    mask = ~mask;
    switch (port) {
      case 0:
        P0_PU &= mask;
        break;
      case 1:
        P1_PU &= mask;
        break;
      case 2:
        P2_PU &= mask;
        break;
      case 3:
        P3_PU &= mask;
        break;
      default:
        Serial.println("N/A");
        break;
    }
    mask = ~mask;
  }
  if (mode == OUTPUT) {
    switch (port) {
      case 0:
        P0_DIR |= mask;
        break;
      case 1:
        P1_DIR |= mask;
        break;
      case 2:
        P2_DIR |= mask;
        break;
      case 3:
        P3_DIR |= mask;
        break;
      default:
        Serial.println("N/A");
        break;
    }
  } else {
    mask = ~mask;
    switch (port) {
      case 0:
        P0_DIR &= mask;
        break;
      case 1:
        P1_DIR &= mask;
        break;
      case 2:
        P2_DIR &= mask;
        break;
      case 3:
        P3_DIR &= mask;
        break;
      default:
        Serial.println("N/A");
        break;
    }
  }
}

void digitalWrite(uint8_t port, uint8_t bit, uint8_t value) {
  uint8_t mask = 1 << bit;
  if (value == HIGH) {
    switch (port) {
      case 0:
        P0 |= mask;
        break;
      case 1:
        P1 |= mask;
        break;
      case 2:
        P2 |= mask;
        break;
      case 3:
        P3 |= mask;
        break;
      default:
        Serial.println("N/A");
        break;
    }
  } else {
    mask = ~mask;
    switch (port) {
      case 0:
        P0 &= mask;
        break;
      case 1:
        P1 &= mask;
        break;
      case 2:
        P2 &= mask;
        break;
      case 3:
        P3 &= mask;
        break;
      default:
        Serial.println("N/A");
        break;
    }
  }
}
