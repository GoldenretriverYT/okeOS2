#pragma once
#include "stdint.h"
#include "stddef.h"

extern inline unsigned char inb (int portnum);

extern inline void outb (int portnum, unsigned char data);