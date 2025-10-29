/* Minimal freestanding kernel entry in C
 * - prints banner to VGA and serial
 * - prints prompt and echoes typed characters (basic scancode map)
 * - uses inline port I/O (inb/outb)
 */

typedef unsigned long size_t;
typedef unsigned long uintptr_t;

static inline unsigned char inb(unsigned short port) {
	unsigned char val;
	__asm__ volatile ("inb %1, %0" : "=a" (val) : "Nd" (port));
	return val;
}

static inline void outb(unsigned short port, unsigned char val) {
	__asm__ volatile ("outb %0, %1" : : "a" (val), "Nd" (port));
}

static void serial_putc(char c) {
	while ((inb(0x3F8 + 5) & 0x20) == 0);
	outb(0x3F8, (unsigned char)c);
}

static void serial_print(const char *s) {
	while (*s) serial_putc(*s++);
}

static void vga_putc_at(unsigned int idx, char c) {
	volatile unsigned short *vga = (volatile unsigned short *)0xB8000;
	vga[idx] = (unsigned short)((0x07 << 8) | (unsigned char)c);
}

static void vga_print_at(unsigned int *idxp, const char *s) {
	unsigned int idx = *idxp;
	while (*s) {
		if (*s == '\n') {
			/* move to next line */
			idx = ((idx / 80) + 1) * 80;
		} else {
			vga_putc_at(idx, *s);
			idx++;
		}
		s++;
	}
	*idxp = idx;
}

/* small scancode map (pairs: scancode -> ascii). 0 terminates */
static const unsigned short sc_map[] = {
	0x1e, 'a', 0x30, 'b', 0x2e, 'c', 0x20, 'd', 0x12, 'e',
	0x21, 'f', 0x22, 'g', 0x23, 'h', 0x17, 'i', 0x24, 'j',
	0x25, 'k', 0x26, 'l', 0x32, 'm', 0x31, 'n', 0x18, 'o',
	0x19, 'p', 0x10, 'q', 0x13, 'r', 0x1f, 's', 0x14, 't',
	0x16, 'u', 0x2f, 'v', 0x11, 'w', 0x2d, 'x', 0x15, 'y',
	0x2c, 'z', 0x02, '1', 0x03, '2', 0x04, '3', 0x05, '4',
	0x06, '5', 0x07, '6', 0x08, '7', 0x09, '8', 0x0a, '9',
	0x0b, '0', 0x39, ' ', 0x1c, '\r', 0, 0
};

static char scancode_to_ascii(unsigned char sc) {
	for (const unsigned short *p = sc_map; *p; p += 2) {
		if ((unsigned char)p[0] == sc) return (char)p[1];
	}
	return 0;
}

void kernel_main(void) {
	unsigned int cursor = 0; /* position index in text cells (80x25) */

	/* Print banner */
	vga_print_at(&cursor, "WNU OS SERVER (C) Installer\n");
	serial_print("[dbg] BOOT\n");

	/* Print prompt */
	vga_print_at(&cursor, "root@install:~# ");
	serial_print("[dbg] PROMPT\n");

	/* input loop */
	char buf[128];
	unsigned int bl = 0;
	for (;;) {
		/* poll keyboard controller */
		unsigned char status = inb(0x64);
		if (!(status & 1)) continue;
		unsigned char sc = inb(0x60);
		if (sc & 0x80) continue; /* key release */
		char ch = scancode_to_ascii(sc);
		if (!ch) continue;

		if (ch == '\r') {
			/* newline */
			vga_print_at(&cursor, "\n");
			serial_print("[dbg] ENTER\n");
			/* simple echo of buffer as OK */
			vga_print_at(&cursor, "OK\n");
			serial_print("OK\n");
			bl = 0;
			/* reprint prompt */
			vga_print_at(&cursor, "root@install:~# ");
			continue;
		}

		if (ch == 8) { /* backspace */
			if (bl) {
				bl--;
				if (cursor) cursor--;
				vga_putc_at(cursor, ' ');
			}
			continue;
		}

		/* normal char */
		if (bl < (int)sizeof(buf)-1) {
			buf[bl++] = ch;
			vga_putc_at(cursor++, ch);
			/* also send to serial */
			serial_putc(ch);
		}
	}
}