#pragma once
#include <global.h>
#include <limine.h>
#include "../io/serial.h"
#include "../utils/mem.h"

extern char _binary_zap_light16_psf_start;

#define PSF1_FONT_MAGIC 0x0436
 
typedef struct {
    uint16_t magic; // Magic bytes for idnetiifcation.
    uint8_t fontMode; // PSF font mode
    uint8_t characterSize; // PSF character size.
} PSF1_Header;
 
 
#define PSF_FONT_MAGIC 0x864ab572
 
typedef struct {
    uint32_t magic;         /* magic bytes to identify PSF */
    uint32_t version;       /* zero */
    uint32_t headersize;    /* offset of bitmaps in file, 32 */
    uint32_t flags;         /* 0 if there's no unicode table */
    uint32_t numglyph;      /* number of glyphs */
    uint32_t bytesperglyph; /* size of each glyph */
    uint32_t height;        /* height in pixels */
    uint32_t width;         /* width in pixels */
} PSF_font;

void term_init(struct limine_framebuffer* fb);
void term_write(char c);
void term_writeline(char* str);