#ifndef UTILS_H
#define UTILS_H

    #include <stdint.h>

    /**************************** Type Definitions ******************************/
    typedef uint8_t   u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;

    #define PAGESIZE 4*1024

    typedef struct{
        u64 phys_addr;
        u64 virt_addr;
    } addr_t;

#endif