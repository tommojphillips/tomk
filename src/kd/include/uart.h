/* uart.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8
#define COM5 0x5F8
#define COM6 0x4F8
#define COM7 0x5E8
#define COM8 0x4E8

int serial_init(uint16_t com_port);
int serial_read(uint16_t com_port);
void serial_write(uint16_t com_port, int byte);

#endif
