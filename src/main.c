#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "pid.h"
#include "bme280.h"
#include "gpio.h"
#include "uart.h"
#include "display.h"
#include "i2c.h"

int uart_filestream, key_gpio = 1;
struct bme280_dev bme_connection;
pthread_t fornoThread;
float intTemp = 0.0, refTemp = 0.0, extTemp = 0.0;
float kp = 30.0, ki = 0.2, kd = 400.0;

int heating = 0; // 1 = Aquecendo forno
int mode = 0; // 1 - Dashboard; 2 - Debug

void startProgram();
void menu();
void switchMode(int command);
void *PID(void *arg);
void exitProgram();

int main () {
    signal(SIGINT, exitProgram); //Sinal para finalizar programa
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

    printf("Escolha o modo que deseja operar o sistema:\n1 - Dashboard\n2 - Debug (inserindo os dados de temperatura)\n");
    while(mode != 1 && mode != 2){
        scanf("%d", &mode);
    }

    if(mode == 1){
        do {
            requestToUart(uart_filestream, GET_UC);
            command = readFromUart(uart_filestream, GET_UC).int_value;
            writeCsv();
            switchMode(command);
            delay(3000);
        } while (1);
    } else if (mode == 2){
        printf("Defina os valores dos parametros de controle de temperatura do PID:\n");
        printf("Kp: ");
        scanf("%f", &kp);
        printf("Ki: ");
        scanf("%f", &ki);
        printf("Kd: ");
        scanf("%f", &kd);
        printf("Defina uma temperatura de referencia para o forno:\n");
        scanf("%f", &refTemp);
        pidUpdateReference(refTemp);
        switchMode(0xA3);
        while (1){
            writeCsv();
        }
    }
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
            heating = 1;
            sendToUartByte(uart_filestream, SEND_FUNC_STATE, 1);
            pthread_create(&fornoThread, NULL, PID, NULL);
            break;
        case 0xA4:
            printf("Parou o aquecimento!\n");
            sendToUartByte(uart_filestream, SEND_FUNC_STATE, 0);
            heating = 0;
            break;
        default:
            break;
    }
}

void *PID(void *arg) {
    float TI, TR, TE;
    pidSetupConstants(kp, ki, kd);
    do {
        requestToUart(uart_filestream, GET_TI);
        TI = readFromUart(uart_filestream, GET_TI).float_value;
        double value = pidControl(TI);
        pwmControl(value);

        if(mode == 1){
            requestToUart(uart_filestream, GET_TR);
            TR = readFromUart(uart_filestream, GET_TR).float_value;
            pidUpdateReference(TR);
        }

        TE = getCurrentTemperature(&bme_connection);

        intTemp = TI;
        refTemp = TR;
        extTemp = TE;
        
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
    } while (heating == 1);
    pthread_exit(0);
}

void exitProgram(){
    printf("Programa encerrado!\n");
    turnResistanceOff();
    turnFanOff();
    closeUart(uart_filestream);
    exit(0);
}

void writeCsv(){
  FILE * fp = fopen("report.csv", "a");
  time_t rawtime;
  struct tm * timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  fprintf(fp, "%s;%.2f;%.2f;%.2f;\n", asctime(timeinfo), refTemp, intTemp, extTemp);

  fclose(fp);
}
