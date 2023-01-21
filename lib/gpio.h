#ifndef GPIO_H_
#define GPIO_H_

#include <wiringPi.h>
#include <softPwm.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "pid.h"

#define RES_ADDR 4
#define VENT_ADDR 5
#define DELAY_GPIO 10000

void gpio_control(float);
void gpio_init();
void aciona_gpio(int, int);
void gpio_end();

#endif