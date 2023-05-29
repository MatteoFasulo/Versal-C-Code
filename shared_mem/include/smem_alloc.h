#ifndef ALLOC_H
#define ALLOC_H

    #include <fcntl.h>
    #include <stdio.h>
    #include <errno.h>
    #include <string.h>
    #include <sys/mman.h>
    #include <unistd.h>

    #include <utils.h>

    /**************************** Type Definitions ******************************/

    #define PAGESIZE 4*1024

    extern "C" {
        void smem_dealloc (addr_t* addr,  size_t size);
        addr_t smem_alloc (u32 phys_addr, size_t size);
    }
#endif