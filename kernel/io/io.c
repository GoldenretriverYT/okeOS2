#include "io.h"

extern inline unsigned char inb (int portnum) {
  unsigned char data=0;
  __asm__ __volatile__ ("inb %%dx, %%al" : "=a" (data) : "d" (portnum));
  return data;
}

extern inline void outb (int portnum, unsigned char data) {
  __asm__ __volatile__ ("outb %%al, %%dx" :: "a" (data),"d" (portnum));
}

extern inline unsigned short inw (int portnum) {
  unsigned short data=0;
  __asm__ __volatile__ ("inw %%dx, %%ax" : "=a" (data) : "d" (portnum));
  return data;
}

extern inline void outw (int portnum, unsigned short data) {
  __asm__ __volatile__ ("outw %%ax, %%dx" :: "a" (data),"d" (portnum));
}

extern inline unsigned int inl (int portnum) {
  unsigned int data=0;
  __asm__ __volatile__ ("inl %%dx, %%eax" : "=a" (data) : "d" (portnum));
  return data;
}

extern inline void outl (int portnum, unsigned int data) {
  __asm__ __volatile__ ("outl %%eax, %%dx" :: "a" (data),"d" (portnum));
}