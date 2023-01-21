#include "../lib/uart.h"

float request_uart_data(int addr)
{
    float temperature = 0.0;

    //Open uart for communication. In case of failure, retry.
    open_connection();
    if (uart0_filestream == -1)
    {
        return request_uart_data(addr);
    }
    unsigned char msg[9] = {ADDRESS, REQUEST, addr, 4, 4, 1, 1};

    //Generate message and send it to device. In case of failure, retry.
    generate_msg(msg);
    if (send_msg(msg, sizeof(msg) / sizeof(msg[0])) == -1)
    {
        return request_uart_data(addr);
    }

    //Receive message from device.
    temperature = receive_msg(addr);

    //End communication.
    close_uart();

    //Return read value.
    return temperature;
}

void open_connection()
{
    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart0_filestream == -1)
    {
        return;
    }

    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);
}

void generate_msg(unsigned char *initial_msg)
{
    short crc = calcula_CRC(initial_msg, 7);

    unsigned char crc_char[2];
    memcpy(crc_char, &crc, 2);

    memcpy(&initial_msg[7], crc_char, 2);
    return;
}

int send_msg(unsigned char *msg, int size_msg)
{
    if (uart0_filestream != -1)
    {
        int count = write(uart0_filestream, &msg[0], size_msg);
        if (count < 0)
        {
            return -1;
        }
        //Wait for device to validate message and send answer.
        usleep(100000);
    }
    return 0;
}

//Receive and validate message.
float receive_msg(int addr)
{
    float temperature = 0.0;
    if (uart0_filestream != -1)
    {
        //In case of error (empty message, incorrect crc), retry.
        unsigned char rx_buffer[256];
        int rx_length = read(uart0_filestream, (void *)rx_buffer, 255);
        if (rx_length < 0)
        {
            close(uart0_filestream);
            return request_uart_data(addr);
        }
        else if (rx_length == 0)
        {
            close(uart0_filestream);
            return request_uart_data(addr);
        }
        else if (check_crc(rx_buffer) != 0)
        {
            close(uart0_filestream);
            return request_uart_data(addr);
        }
        memcpy(&temperature, &rx_buffer[3], 4);
    }
    return temperature;
}

//Validates message received to avoid errors.
int check_crc(unsigned char *msg)
{
    short crc = calcula_CRC(msg, 7);

    unsigned char my_crc[2];
    memcpy(my_crc, &crc, 2);

    unsigned char received[2];
    memcpy(received, &msg[7], 2);

    return memcmp(my_crc, received, 2);
}

void close_uart()
{
    close(uart0_filestream);
}