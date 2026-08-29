/* uart.h
 * Thomas J. Armytage 2026 ( https://github.com/tommojphillips/ )
 */

#ifndef UART_H
#define UART_H

int serial_init(void);
int serial_read(void);
void serial_write(int byte);

#endif
