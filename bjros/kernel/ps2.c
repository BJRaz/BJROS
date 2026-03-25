#include <ps2.h>

// PS/2 timeout constants (in iterations)
// Adjust these if needed based on system speed
#define PS2_TIMEOUT_ITERATIONS 100000

void ps2_controller_send_command(uint8_t command)
{
	uint32_t timeout = PS2_TIMEOUT_ITERATIONS;
	while(timeout > 0 && (inb(PS2_CMD) & 0x2)) {	// wait for input buffer empty
		timeout--;
	}
	outb(PS2_CMD, command); 
}

void ps2_controller_write_data(uint8_t data) 
{
	uint32_t timeout = PS2_TIMEOUT_ITERATIONS;
	while(timeout > 0 && (inb(PS2_CMD) & 0x2)) {	// wait for input buffer empty
		timeout--;
	}
	outb(PS2_DATA, data); 
}

uint8_t ps2_controller_read_data()
{
	uint32_t timeout = PS2_TIMEOUT_ITERATIONS;
	while(timeout > 0 && !(inb(PS2_CMD) & 0x1)) {	// wait for output buffer full
		timeout--;
	}
	return inb(PS2_DATA);
}

uint8_t ps2_output_buffer_status() {
	return inb(PS2_CMD) & 0x1; 
}

uint8_t ps2_input_buffer_status() {
	return inb(PS2_CMD) & 0x2;
}


void setup_ps2()
{
	uint8_t ps2_response = 0;

	// step 1: Initialize all USB devices, and disable USB legacy support
	//
	// step 2: check for existence of i8042 controller
	// Note: should be done via ACPI "IA PC Boot architecture" flags 
	// offset 109 in FADT (	Fixed ACPI description table )
	// At the moment, if its not present the system may fail

	// STEP 3-4: disable devices and read result
	// disable first PS2 port (kbd)
	ps2_controller_send_command(0xAD);
	kprintf(">ps2: Disable kbd PS2 port 1\n");
	/*ps2_response = ps2_controller_read_data();
	kprintf("Ps2 response: 0x%x\n", ps2_response);*/

	// disable second PS2 port (mouse)
	ps2_controller_send_command(0xA7);
	kprintf(">ps2: Disable mouse PS2 port 2\n");
	/*ps2_response = ps2_controller_read_data();
	kprintf("Ps2 response: 0x%x\n", ps2_response);*/
	
	// STEP 5: Get and set configuration byte 
	ps2_controller_send_command(0x20);
	kprintf(">ps2: read config: ");
	ps2_response = ps2_controller_read_data();
	kprintf("<ps2: config. byte: 0x%x\n", ps2_response);
	
	ps2_response &= 0b1011100;		// clear bits 0,1 and 6

	kprintf(">Ps2: changing configuration byte to: 0x%x\n", ps2_response);
	ps2_controller_send_command(0x60);
	ps2_controller_write_data(ps2_response);
	
	// STEP 6: Do self test - should result in 0x55 
	ps2_controller_send_command(0xAA);
	kprintf(">ps2: (self test) ");
	ps2_response = ps2_controller_read_data();
	kprintf("<ps2: self test status: 0x%x - %s\n", ps2_response, (ps2_response == 0x55) ? "OK": "NOK OK");

	// STEP 7: (optional)
	// STEP 8: first and second port status test 
	ps2_controller_send_command(0xAB);
	kprintf(">ps2: port 1 status test ");
	ps2_response = ps2_controller_read_data();
	kprintf("<Ps2: port 1 status: 0x%x - %s\n", ps2_response, (ps2_response == 0x00) ? "OK" : "NOT OK");
	ps2_controller_send_command(0xA9);
	kprintf(">ps2: port 2 status test ");
	ps2_response = ps2_controller_read_data();
	kprintf("<Ps2: port 2 status: 0x%x - %s\n", ps2_response, (ps2_response == 0x00) ? "OK" : "NOT OK");

	// STEP 9: enable ports 1 and 2
	ps2_controller_send_command(0xAE);
	kprintf(">ps2: port 1 enable)\n");
	ps2_controller_send_command(0xA8);
	kprintf(">ps2: port 2 enable\n");
	ps2_controller_send_command(0x20);
	kprintf(">ps2: read config. byte ");
	ps2_response = ps2_controller_read_data();
	kprintf("<Ps2: configuration byte: 0x%x\n", ps2_response);
	ps2_response |= 0b0100011;		// enable bits 0,1 and 6

	// write back new config. byte
	kprintf(">Ps2: changing configuration byte to: 0x%x\n", ps2_response);
	ps2_controller_send_command(0x60);
	ps2_controller_write_data(ps2_response);
	
	// STEP 10: reset devices
	//ps2_controller_send_command(0xD1);		// maybe, maybe not..
	ps2_controller_write_data(0xFF);
	kprintf(">ps2: reset kbd ");
	//while(ps2_output_buffer_status()) {
		ps2_response = ps2_controller_read_data();
		kprintf("<Ps2: 0x%x\n", ps2_response);	// read acknowledge

	//}
	ps2_response = ps2_controller_read_data();
	kprintf("<Ps2: 0x%x - %s\n", ps2_response, (ps2_response == 0xaa) ? "Self test OK" : "NOT OK");	// read response AA (passed)

	// send RESET to mouse device
	ps2_controller_send_command(0xD4);
	ps2_controller_write_data(0xFF);
	kprintf(">ps2: reset mouse ");
	ps2_response = ps2_controller_read_data();
	kprintf("<Ps2: 0x%x\n", ps2_response);	// read ack
	ps2_response = ps2_controller_read_data();
	kprintf("<Ps2: 0x%x - %s\n", ps2_response, (ps2_response == 0xaa) ? "Self test OK" : "NOT OK"); 	// read response AA

/*	

	ps2_controller_send_command(0xD4);	// get mouse info
	ps2_controller_write_data(0xE9);
	ps2_response = ps2_controller_read_data();
	
	kprintf("PS2 mouse info: 0x%x\n", ps2_response);
*/
	ps2_controller_send_command(0xD4);	// 
	ps2_controller_write_data(0xF3);	// set sample rate command
	kprintf(">ps2: sample rate 0xF3 ");
	ps2_response = ps2_controller_read_data();
	kprintf("<Ps2: 0x%x - %s\n", ps2_response, (ps2_response == 0xFA) ? "OK" : "NOT OK");

	ps2_controller_send_command(0xD4);	// 
	ps2_controller_write_data(100);		// set sample rate to 100
	kprintf(">ps2: sample rate 100 ");
	ps2_response = ps2_controller_read_data();
	kprintf("<Ps2: 0x%x - %s\n", ps2_response, (ps2_response == 0xFA) ? "OK" : "NOT OK");


	ps2_controller_send_command(0xD4);	// 
	ps2_controller_write_data(0xF4);	// enable data reporting - mouse wont work until this is set
	kprintf(">ps2: enable data reporting ");
	ps2_response = ps2_controller_read_data();
	kprintf("<Ps2: 0x%x - %s\n", ps2_response, (ps2_response == 0xFA) ? "OK" : "NOT OK");
/*
 * This should disable keyboard, and return the keyboards IDENTIFICATION byte(s)
 * does not work as of sept. 20 2021.
 * TODO: make it work 
 *
 *
	ps2_controller_send_command(0xF5);
	ps2_response = ps2_controller_read_data();
	if(ps2_response == 0xFA) 
	{
		ps2_controller_send_command(0xF2);	// identify command
		if((ps2_response = ps2_controller_read_data()) == 0xFA) {
			ps2_response = ps2_controller_read_data();
			kprintf("IDENT: 0x%x\n", ps2_response);
		} else
			kprintf("Returned: 0x%x\n", ps2_response);

	} else
			kprintf("Returned: 0x%x\n", ps2_response);
*/
	return;	
		

}


