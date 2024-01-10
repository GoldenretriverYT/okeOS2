#pragma once
#include "stdint.h"
#include "stddef.h"

extern unsigned char inb (int portnum);
extern void outb (int portnum, unsigned char data);

extern unsigned short inw (int portnum);
extern void outw (int portnum, unsigned short data);

extern unsigned int inl (int portnum);
extern void outl (int portnum, unsigned int data);
