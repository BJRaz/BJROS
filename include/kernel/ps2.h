#ifndef PS2_H
#define PS2_H

#include <kernel.h>

#define PS2_DATA	0x60			// i8042 ps/2 controller data port (r/w)
#define PS2_CMD		0x64			// i8042 ps/2 status register (read), command register (write)

void ps2_controller_send_command(uint8_t command);
void ps2_controller_write_data(uint8_t data); 
uint8_t ps2_controller_read_data();
uint8_t ps2_output_buffer_status();
uint8_t ps2_input_buffer_status();
void setup_ps2();

#endif

