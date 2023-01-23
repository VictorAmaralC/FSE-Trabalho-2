#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "pid.h"
#include "bme280.h"
#include "gpio.h"
#include "uart.h"
#include "display.h"
#include "i2c.h"

int uart_filestream, key_gpio = 1;
struct bme280_dev bme_connection;
pthread_t fornoThread;

int mode = 0; // 1 = Aquecendo forno

void startProgram();
void menu();
void switchMode(int command);
void *PID(void *arg);
void exitProgram();

int main () {
    signal(SIGINT, exitProgram);
    startProgram();
    menu();
    return 0;
}

void startProgram(){
    wiringPiSetup();
    turnResistanceOff();
    turnFanOff();
    connectDisplay();
    bme_connection = connectBme();
    uart_filestream = initUart();
}

void menu() {
    int command;
    do {
        requestToUart(uart_filestream, GET_UC);
        command = readFromUart(uart_filestream, GET_UC).int_value;
        switchMode(command);
        delay(500);
    } while (1);
}

void switchMode(int command) {
    switch(command) {
        case 0xA1:
            printf("Ligou o forno!\n");
            sendToUartByte(uart_filestream, SEND_SYSTEM_STATE, 1);
            break;
        case 0xA2:
            printf("Desligou o forno!\n");
            sendToUartByte(uart_filestream, SEND_SYSTEM_STATE, 0);
            break;
        case 0xA3:
            printf("Iniciou o aquecimento!\n");
            mode = 1;
            sendToUartByte(uart_filestream, SEND_FUNC_STATE, 1);
            pthread_create(&fornoThread, NULL, PID, NULL);
            break;
        case 0xA4:
            printf("Parou o aquecimento!\n");
            sendToUartByte(uart_filestream, SEND_FUNC_STATE, 0);
            mode = 0;
            break;
        default:
            break;
    }
}

void *PID(void *arg) {
    float TI, TR, TE;
    pidSetupConstants(30.0, 0.2, 400.0);
    do {
        requestToUart(uart_filestream, GET_TI);
        TI = readFromUart(uart_filestream, GET_TI).float_value;
        double value = pidControl(TI);
        pwmControl(value);

        requestToUart(uart_filestream, GET_TR);
        TR = readFromUart(uart_filestream, GET_TR).float_value;
        pidUpdateReference(TR);

        TE = getCurrentTemperature(&bme_connection);
        printf("TI: %.2f⁰C - TR: %.2f⁰C - TE: %.2f⁰C\n", TI, TR, TE);

        if(TR > TI){
            turnResistanceOn(100);
            turnFanOff();
            value = 100;
            sendToUart(uart_filestream, SEND_CONTROL_SIGNAL, value);
        } else if(TR <= TI) {
            turnResistanceOff();
            turnFanOn(100);
            value = -100;
            sendToUart(uart_filestream, SEND_CONTROL_SIGNAL, value);
        }
    } while (mode == 1);
}

void exitProgram(){
    printf("Programa encerrado!\n");
    turnResistanceOff();
    turnFanOff();
    closeUart(uart_filestream);
    exit(0);
}