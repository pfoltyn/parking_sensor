#undef TESTING

#include "pic16f917.h"

#define __16f917


typedef unsigned int config;
config __at 0x2007 __CONFIG =
  _CPD_OFF & _CP_OFF & _FCMEN_ON & _MCLRE_ON & _PWRTE_OFF & _WDT_OFF & _HS_OSC;

static const char
led_left[16] = {
  0b00000000,
  0b00000000,
  0b01000000,
  0b01000000,
  0b01100000,
  0b01100000,
  0b01110000,
  0b01110000,
  0b01111000,
  0b01111000,
  0b01111100,
  0b01111100,
  0b01111110,
  0b01111110,
  0b01111111,
  0b01111111,
};

static const char
led_right[16] = {
  0b00000000,
  0b00000000,
  0b00000010,
  0b00000010,
  0b00000110,
  0b00000110,
  0b00001110,
  0b00001110,
  0b00011110,
  0b00011110,
  0b00111110,
  0b00111110,
  0b01111110,
  0b01111110,
  0b11111110,
  0b11111110,
};

static const unsigned int
led_mask[16] = {
  0b1111111111111110,
  0b1111111111111101,
  0b1111111111111011,
  0b1111111111110111,
  0b1111111111101111,
  0b1111111111011111,
  0b1111111110111111,
  0b1111111101111111,
  0b1111111011111111,
  0b1111110111111111,
  0b1111101111111111,
  0b1111011111111111,
  0b1110111111111111,
  0b1101111111111111,
  0b1011111111111111,
  0b0111111111111111,
};

static volatile char mask_cnt;
static volatile char left;
static volatile char right;
static volatile char data_cnt;
static volatile char snd_cnt;

static void
isr(void) __interrupt 0
{
  if (RBIF)
  {
    CREN = (RB5) ? 0 : 1;
    RBIF = 0;
    data_cnt = 0;
  }
  if (RCIF)
  {
    switch (data_cnt)
    {
    case 0:
      left = RCREG >> 3;
      break;
    case 1:
      right = RCREG >> 3;
      CREN = 0;
      break;
    };
    data_cnt++;
  }
  if (T0IF)
  {
    char limit;
    switch (snd_cnt)
    {
    case 0:
      TMR2 = 0;
      TMR2ON = 1;
      break;
    case 1:
      TMR2ON = 0;
      break;
    };
    if (right < left)
      limit = right & 0x0F;
    else
      limit = left & 0x0F;
    snd_cnt++;
    if (snd_cnt >= limit)
      snd_cnt = 0;
    T0IF = 0;
  }
}

void
main (void)
{
  LCDCON = 0x00;
  INTCON = 0x00;
  SWDTEN = 0;

  TRISE = 0x00;
  PORTE = 0xFF;

  TRISB = 0xF0;
  PORTB = 0xFF;

  TRISC = 0xD0;
  PORTC = 0xFF;

  TRISD = 0x00;
  PORTD = 0xFF;

  TMR0 = 0;
  OPTION_REG &= ~((1 << 5 /*T0CS*/) | (1 << 3 /*PSA*/));
  T0IE = 1;
  
  RBIE = 1;
  IOCB5 = 1;

  SYNC = 1;
  SPEN = 1;
  RCIE = 1;
  PEIE = 1;
  GIE = 1;

  PR2 = 0x7F;
  CCPR1L = 0x4F;
  CCP1CON = 0xFF;
  TMR2 = 0;
  T2CKPS1 = 1;

  left = 15;
  right = 15;
  mask_cnt = 0;
  data_cnt = 0;
  snd_cnt = 0;

  while (1)
  {
    char ll = led_left[left & 0x0F];
    char lr = led_right[right & 0x0F];
    int lm = led_mask[mask_cnt];

    PORTC = (PORTC & 0xF0) | ((ll | (char)lm) & 0x0F);
    PORTD = (lr << 4) | (ll >> 4) | (char)(lm >> 4);
    PORTB = (PORTB & 0xF0) | (lr >> 4) | (char)(lm >> 12);

    mask_cnt = (mask_cnt + 1) & 0x0F;
  }
}

