#include "../inc/gpio.h"

//Controls wich device runs based on signal received.
void gpio_control(float internal_temp)
{
    int control_signal = (int)pid_controle(internal_temp);
    if (control_signal > 0)
    {
        aciona_gpio(VENT_ADDR, 0);
        usleep(DELAY_GPIO);
        aciona_gpio(RES_ADDR, control_signal);
        usleep(DELAY_GPIO);
    }
    else if (control_signal <= (-40))
    {
        aciona_gpio(RES_ADDR, 0);
        usleep(DELAY_GPIO);
        aciona_gpio(VENT_ADDR, control_signal * (-1));
        usleep(DELAY_GPIO);
    }
    else
    {
        aciona_gpio(VENT_ADDR, 0);
        usleep(DELAY_GPIO);
        aciona_gpio(RES_ADDR, 0);
        usleep(DELAY_GPIO);
    }
}

void gpio_init()
{
    if (wiringPiSetup() == -1)
    {
        gpio_init();
        return;
    }
    pinMode(RES_ADDR, PWM_OUTPUT);
    pinMode(VENT_ADDR, PWM_OUTPUT);
    softPwmCreate(RES_ADDR, 1, 100);
    softPwmCreate(VENT_ADDR, 1, 100);
}

void aciona_gpio(int PWM_pin, int intensidade)
{
    softPwmWrite(PWM_pin, intensidade);
    usleep(DELAY_GPIO);
}

void gpio_end()
{
    aciona_gpio(VENT_ADDR, 0);
    usleep(DELAY_GPIO);
    aciona_gpio(RES_ADDR, 0);
    usleep(DELAY_GPIO);
}