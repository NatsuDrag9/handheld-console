/*
 * debug_conf.c
 *
 *  Created on: Dec 8, 2024
 *      Author: rohitimandi
 */

#ifndef SRC_SYSTEM_DEBUG_CONF_C_
#define SRC_SYSTEM_DEBUG_CONF_C_

#include "Utils/debug_conf.h"

#if DEBUG_ENABLE
static volatile int debug_enabled = 1;
static char printf_buf[256];  // Static buffer for float printing

void debug_enable(void) {
    debug_enabled = 1;
}

void debug_disable(void) {
    debug_enabled = 0;
}

uint8_t is_debug_enabled(void) {
    return debug_enabled;
}

// Overwrite weak implementation of _write() in syscalls.c
// Redirect printf to SWV ITM by adding the following function
int _write(int file, char *ptr, int len) {
    for (int i = 0; i < len; i++) {
        ITM_SendChar((*ptr++));
    }
    return len;
}

void debug_printf(int is_float, const char* format, ...) {
    if (!debug_enabled) return;

    va_list args;
    va_start(args, format);

    if (is_float) {
        vsnprintf(printf_buf, sizeof(printf_buf), format, args);
        // Directly send to ITM
               for(int i = 0; printf_buf[i] != '\0'; i++) {
                   ITM_SendChar(printf_buf[i]);
               }
    } else {
        vprintf(format, args);
    }

    va_end(args);
}

#endif


#endif /* SRC_SYSTEM_DEBUG_CONF_C_ */
