#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include "crc16.h"

#define ADDRESS 0x01

#define REQUEST 0x23
#define SEND 0x16

#define R_TI 0xC1
#define R_TR 0xC2
#define R_UI 0xC3
#define R_CS 0xD1
#define R_RS 0xD2
#define R_SS 0xD3
#define R_MC 0xD4
#define R_FS 0xD5
#define R_AT 0xD6

int check_crc(unsigned char *);
int send_msg(unsigned char *, int);
void open_connection();
void generate_msg(unsigned char *);
void close_uart();
float receive_msg(int);
float request_uart_data(int);

int uart0_filestream;
