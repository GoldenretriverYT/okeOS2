#include "term.h"

static struct limine_framebuffer* fb;
static uint64_t term_chr_x = 0;
static uint64_t term_chr_y = 0;
static PSF1_Header* term_font;
static uint8_t term_enabled = 0;

void load_psf() {
    term_font = (PSF1_Header*)&_binary_zap_light16_psf_start;
}

void term_init(struct limine_framebuffer* _fb) {
    term_enabled = 1;
    fb = _fb;

    load_psf();

    write_serial_string("term_init: &_binary_zap_light16_psf_start = ");
    write_serial_u64((uint64_t)&_binary_zap_light16_psf_start, 16);
    write_serial_string("\n");

    write_serial_string("term_init: fb->address = ");
    write_serial_u64((uint64_t)fb->address, 16);
    write_serial_string("\n");

    write_serial_string("term_init: fb->pitch = ");
    write_serial_u64(fb->pitch, 10);
    write_serial_string("\n");

    write_serial_string("term_init: psf->characterSize = ");
    write_serial_u64(term_font->characterSize, 10);
    write_serial_string("\n");
}
void term_write(char c) {
    if (!term_enabled) {
        return;
    }
    
    uint64_t termX = term_chr_x * 8;
    uint64_t termY = term_chr_y * (term_font->characterSize);

    if (c == '\n') {
        term_chr_x = 0;
        term_chr_y++;
    } else {
        uint8_t* glyph = (uint8_t*)term_font + sizeof(PSF1_Header) + (c * term_font->characterSize);

        for (uint64_t y = 0; y < term_font->characterSize; y++) {
            uint64_t fbOffset = (termY + y) * fb->pitch + (termX * (fb->bpp / 8));

            *(uint32_t*)(fb->address + fbOffset + 28) = (glyph[y] >> 0) & 1 ? 0xFFFFFFFF : 0x00000000;
            *(uint32_t*)(fb->address + fbOffset + 24) = (glyph[y] >> 1) & 1 ? 0xFFFFFFFF : 0x00000000;
            *(uint32_t*)(fb->address + fbOffset + 20) = (glyph[y] >> 2) & 1 ? 0xFFFFFFFF : 0x00000000;
            *(uint32_t*)(fb->address + fbOffset + 16) = (glyph[y] >> 3) & 1 ? 0xFFFFFFFF : 0x00000000;
            *(uint32_t*)(fb->address + fbOffset + 12) = (glyph[y] >> 4) & 1 ? 0xFFFFFFFF : 0x00000000;
            *(uint32_t*)(fb->address + fbOffset +  8) = (glyph[y] >> 5) & 1 ? 0xFFFFFFFF : 0x00000000;
            *(uint32_t*)(fb->address + fbOffset +  4) = (glyph[y] >> 6) & 1 ? 0xFFFFFFFF : 0x00000000;
            *(uint32_t*)(fb->address + fbOffset +  0) = (glyph[y] >> 7) & 1 ? 0xFFFFFFFF : 0x00000000;
        }

        term_chr_x++;

        if (term_chr_x >= fb->width / 8) {
            term_chr_x = 0;
            term_chr_y++;
        }
    }

    if (term_chr_y >= fb->height / 16) {
        term_chr_y = 0;
        memset(fb->address, 0, fb->pitch * fb->height);
    }
}

void term_writeline(char* str) {
    while (*str) {
        term_write(*str++);
    }
    term_write('\n');
}