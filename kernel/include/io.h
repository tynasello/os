#ifndef __IO_H
#define __IO_H

#include <stdint.h>

uint8_t port_byte_in(uint16_t port);
void port_byte_out(uint16_t port, uint8_t data);

#endif
