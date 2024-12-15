/*
 * debug_conf.h
 *
 *  Created on: Dec 8, 2024
 *      Author: rohitimandi
 */

#ifndef INC_SYSTEM_DEBUG_CONF_H_
#define INC_SYSTEM_DEBUG_CONF_H_

#ifndef DEBUG_ENABLE
    #define DEBUG_ENABLE 1  // Default to disabled
#endif

#if DEBUG_ENABLE
    #include <stdio.h>
    #include "stm32f4xx_hal.h"

    // Function prototypes
    int _write(int file, char *ptr, int len);
    void debug_enable(void);
    void debug_disable(void);
    uint8_t is_debug_enabled(void);

    // Runtime debug control
    #define DEBUG_PRINTF(...) do { \
        if (is_debug_enabled()) { \
            printf(__VA_ARGS__); \
        } \
    } while(0)
#else
    #define DEBUG_PRINTF(...) /* nothing */
    static inline void debug_enable(void) {}
    static inline void debug_disable(void) {}
    static inline int is_debug_enabled(void) { return 0; }
#endif

#endif /* INC_SYSTEM_DEBUG_CONF_H_ */


/**
 * Example usage
 *
 * DEBUG_PRINTF("Value = %d\n", x);
 *
 * DEBUG_PRINTF("Multiple args: %d, %s, %f\n", num, str, fval);
 */
