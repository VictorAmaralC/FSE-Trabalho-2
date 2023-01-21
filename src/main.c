#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <ncurses.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include "../lib/gpio.h"
#include "../lib/i2c.h"
#include "../lib/uart.h"
#include "../lib/pid.h"

void sys_init();
void sys_run();
void sys_end();
void box_around(int y, int x, int h, int w);
void update_temp();
void *menu();
void *reference_input();
void print_menu();
void static_menu();

char *choices[] = {"1. Ler temperatura de referência por input no teclado.",
                   "2. Ler temperatura de referência pelo potenciômetro.",
                   "3. Sair."};
float internal_temp, external_temp, reference_temp;
float user_defined_temp = 0;
int choice = 1;
int selected_row = 0;
WINDOW *win = NULL;

int main()
{
    sys_init();

    sys_run();

    sys_end();

    return 0;
}

void sys_init()
{
    //Signal set to end all communications and exit program.
    signal(SIGINT, sys_end);

    FILE *fp = fopen("output.csv", "w");

    fprintf(fp, "TIME,TEMP_INT,TEMP_EXT,TEMP_USR,SIGNAL;\n");

    fclose(fp);

    //Starting communications wich stays open for the remainder of the program.
    gpio_init();
    pid_configura_constantes(30.0, 0.2, 400.0);

    //Setting up ncurses.
    initscr();
    curs_set(0);
    start_color();
    cbreak();
    keypad(stdscr, TRUE);

    //Color pairs for customization using ncurses.
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);

    //Print menu before retrieving values.
    static_menu();

    //Start menu thread.
    pthread_t menu_thread;
    pthread_create(&menu_thread, NULL, menu, NULL);
}

void sys_run()
{
    int i = 0;
    while (1)
    {
        internal_temp = request_uart_data(R_TI);
        if (choice == 1)
        {
            reference_temp = request_uart_data(R_TR);
        }
        external_temp = bme_start();

        pid_atualiza_referencia(reference_temp);
        gpio_control(internal_temp);

        //Saves log roughly every 2 seconds.
        if (i%2 == 0)
        {
            time_t rawtime;
            struct tm *timeinfo;

            time(&rawtime);
            timeinfo = localtime(&rawtime);

            FILE *fp = fopen("output.csv", "a");

            fprintf(fp, "%s,%.2f,%.2f,%.2f,%.2lf\n", asctime(timeinfo), internal_temp, external_temp, user_defined_temp, pid_controle(internal_temp));

            fclose(fp);
        }

        static_menu();

        update_temp(internal_temp, external_temp, reference_temp);

        if (choice == 2)
        {
            break;
        }
        else if (choice == -2)
        {
            refresh();
            redrawwin(win);
            wrefresh(win);
        }
        else
        {
            refresh();
        }
        i++;
        usleep(550000);
    }
    return;
}

//Ends communication and exits program.
void sys_end()
{
    gpio_end();
    close_uart();
    endwin();
    exit(0);
}

//Updates highlighted menu option.
void print_menu()
{
    for (int i = 10, j = 0; i < 13; i++, j++)
    {
        if (j == selected_row)
        {
            attron(COLOR_PAIR(1));
            mvaddstr(i, 2, choices[j]);
            attroff(COLOR_PAIR(1));
        }
        else
        {
            mvaddstr(i, 2, choices[j]);
        }
    }
}

//Used in thread to read user input for reference temperature.
void *reference_input()
{
    win = newwin(14, 2, 14, 47);
    wscanw(win, "%f", &reference_temp);
    if (reference_temp > 100 || reference_temp < external_temp)
    {
        reference_temp = request_uart_data(R_TR);
        choice = 1;
        mvprintw(14, 2, "Temperatura invalida. Revertendo para potenciometro.");
        mvprintw(15, 2, "Certifique-se de que a temperatura escolhida estah dentro do intervalo:");
        mvprintw(16, 2, "Temperatura Ambiente < Temperatura de Referencia < 100");
        delwin(win);
        return NULL;
    }
    else
    {
        user_defined_temp = reference_temp;
        choice = -1;
        delwin(win);
        clear();
        return NULL;
    }
}

//Running thread for menu input.
void *menu()
{
    usleep(1000000);
    int c;
    while (choice != 2)
    {
        if (choice != -2)
        {
            c = getch();
            if (c == KEY_UP && selected_row > 0)
            {
                selected_row -= 1;
            }
            else if (c == KEY_DOWN && selected_row < 2)
            {
                selected_row += 1;
            }
            else if (c == 10)
            {
                clear();
                choice = selected_row;
                if (choice == 0)
                {
                    choice = -2;
                    pthread_t t2;
                    pthread_create(&t2, NULL, reference_input, NULL);
                    pthread_join(t2, NULL);
                }
            }
            print_menu();
        }
    }
    return NULL;
}

void box_around(int y, int x, int h, int w)
{
    mvaddch(y, x, ACS_ULCORNER);
    for (int j = 0; j < w; j++)
        addch(ACS_HLINE);
    addch(ACS_URCORNER);

    for (int j = 0; j < h; j++)
    {
        mvaddch(y + 1 + j, x, ACS_VLINE);
        mvaddch(y + 1 + j, x + w + 1, ACS_VLINE);
    }

    mvaddch(y + h + 1, x, ACS_LLCORNER);

    for (int j = 0; j < w; j++)
        addch(ACS_HLINE);
    addch(ACS_LRCORNER);
}

void static_menu()
{
    int height, width;

    getmaxyx(stdscr, height, width);

    box_around(0, 0, 1, width - 3);
    box_around(0, 0, 5, width - 3);
    mvaddch(2, 0, ACS_LTEE);
    mvaddch(2, width - 2, ACS_RTEE);
    mvaddch(4, 0, ACS_LTEE);
    for (int i = 0; i < width - 3; i++)
    {
        addch(ACS_HLINE);
    }
    mvaddch(4, width - 2, ACS_RTEE);

    mvaddch(2, width / 3, ACS_TTEE);
    mvaddch(3, width / 3, ACS_VLINE);
    mvaddch(4, width / 3, ACS_PLUS);

    mvaddch(2, 2 * width / 3, ACS_TTEE);
    mvaddch(3, 2 * width / 3, ACS_VLINE);
    mvaddch(4, 2 * width / 3, ACS_PLUS);

    mvaddch(5, width / 3, ACS_VLINE);
    mvaddch(6, width / 3, ACS_BTEE);

    mvaddch(5, 2 * width / 3, ACS_VLINE);
    mvaddch(6, 2 * width / 3, ACS_BTEE);

    box_around(7, 0, height - 9, width / 2 - 2);
    box_around(7, width / 2 + 1, height - 9, width / 2 - 3);

    box_around(7, 0, 1, width / 2 - 2);
    mvaddch(9, 0, ACS_LTEE);
    mvaddch(9, width / 2 - 1, ACS_RTEE);

    box_around(7, width / 2 + 1, 1, width / 2 - 3);
    mvaddch(9, width / 2 + 1, ACS_LTEE);
    mvaddch(9, width - 2, ACS_RTEE);

    char msg[] = "Sistema de monitoramento de temperatura - RaspberryPi";
    mvprintw(1, (width - strlen(msg)) / 2, "%s", msg);
    mvprintw(3, (width / 3 - 19) / 2, "Temperatura Interna");
    mvprintw(3, (width - 20) / 2, "Temperatura Ambiente");
    mvprintw(3, (width + 2 * width / 3 - 23) / 2, "Temperatura Referencia");

    mvprintw(8, width / 4 - 3, "Opcoes");

    mvprintw(8, (width / 4) * 3, "Status");

    if (choice == -2)
    {
        mvprintw(14, 2, "Digite a temperatura de referência desejada: ");
    }
    if (choice == 1 || choice == 0)
    {
        mvprintw(10, (width / 4) * 2 + 5, "Temperatura de referência atual: ");
        attron(COLOR_PAIR(2));
        printw("Potenciômetro");
        attroff(COLOR_PAIR(2));
    }
    else if (choice == -1)
    {
        mvprintw(10, (width / 4) * 2 + 5, "Temperatura de referência atual: ");
        attron(COLOR_PAIR(2));
        printw("Usuário");
        attroff(COLOR_PAIR(2));
    }

    double result = pid_controle(internal_temp);
    if (result > 0)
    {
        mvprintw(11, (width / 4) * 2 + 5, "Ventoinha: ");
        attron(COLOR_PAIR(3));
        printw("Desligado                 ");
        attroff(COLOR_PAIR(3));
        mvprintw(12, (width / 4) * 2 + 5, "Resistencia: ");
        attron(COLOR_PAIR(2));
        printw("Ligado(%.2lf)           ", result);
        attroff(COLOR_PAIR(2));
    }
    else if (result < -40)
    {
        mvprintw(11, (width / 4) * 2 + 5, "Ventoinha: ");
        attron(COLOR_PAIR(2));
        printw("Ligado(%.2lf)             ", result * -1);
        attroff(COLOR_PAIR(2));
        mvprintw(12, (width / 4) * 2 + 5, "Resistencia: ");
        attron(COLOR_PAIR(3));
        printw("Desligado             ");
        attroff(COLOR_PAIR(3));
    }
    else
    {
        mvprintw(11, (width / 4) * 2 + 5, "Ventoinha: ");
        attron(COLOR_PAIR(3));
        printw("Desligado                  ");
        attroff(COLOR_PAIR(3));
        mvprintw(12, (width / 4) * 2 + 5, "Resistencia: ");
        attron(COLOR_PAIR(3));
        printw("Desligado                 ");
        attroff(COLOR_PAIR(3));
    }

    print_menu();
}

void update_temp()
{
    int width;

    width = getmaxx(stdscr);

    mvprintw(5, (width / 3 - 7) / 2, "%.2f ºC    ", internal_temp);
    mvprintw(5, (width - 7) / 2, "%.2f ºC    ", external_temp);
    mvprintw(5, (width + 2 * width / 3 - 7) / 2, "%.2f ºC    ", reference_temp);
}