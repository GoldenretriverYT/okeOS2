#include "serial.h"

int init_serial() {
   outb(PORT + 1, 0x00);    // Disable all interrupts
   outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(PORT + 1, 0x00);    //                  (hi byte)
   outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
   outb(PORT + 4, 0x1E);    // Set in loopback mode, test the serial chip
   outb(PORT + 0, 0xAE);    // Send a test byte
 
   // Check that we received the same test byte we sent
   if(inb(PORT + 0) != 0xAE) {
      return 1;
   }
 
   // If serial is not faulty set it in normal operation mode:
   // not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled
   outb(PORT + 4, 0x0F);
   return 0;
}

int serial_received() {
   return inb(PORT + 5) & 1;
}
 
char read_serial() {
   while (serial_received() == 0);
 
   return inb(PORT);
}

int is_transmit_empty() {
   return inb(PORT + 5) & 0x20;
}
 
void write_serial(char a) {
   while (is_transmit_empty() == 0);
 
   outb(PORT,a);
}

void write_serial_string(char* a) {
   while (*a != '\0') {
      write_serial(*a);
      a++;
   }
}

void write_serial_u64(uint64_t a, int base) {
   char buffer[32];
   itoa_u64(a, buffer, base);
   write_serial_string(buffer);
}

void write_serial_i64(int64_t a, int base) {
   char buffer[32];
   itoa_i64(a, buffer, base);
   write_serial_string(buffer);
}