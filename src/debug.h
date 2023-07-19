#ifndef DEBUG_H
#define DEBUG_H

void debug_init(void);

void debug_write_byte(char c);
void debug_write_string(char *str);
void debug_write_uint32(uint32_t val);
void debug_write_uint64(uint64_t val);

#endif
