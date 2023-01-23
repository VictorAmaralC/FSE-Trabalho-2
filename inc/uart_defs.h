#ifndef UART_DEFS_H
#define UART_DEFS_H

#define GET_TI 0xC1
#define GET_TR 0xC2
#define GET_UC 0xC3
#define SEND_CONTROL_SIGNAL 0xD1
#define SEND_SYSTEM_STATE 0xD3
#define SEND_FUNC_STATE 0xD5

typedef struct Number_type {
  int int_value;
  float float_value;
} Number_type;

#endif