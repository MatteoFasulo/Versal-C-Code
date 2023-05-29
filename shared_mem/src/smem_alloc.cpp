#include "smem_alloc.h"


/*****************************************************************************/
/**
* Alloca un buffer di memoria in DDR.
*
* @param	size is the size of the buffer.
*
* @return
*		- virual and physical address of the buffer.
*
* @note		The buffer MUST be Page-aligned.
*
******************************************************************************/
addr_t smem_alloc(u32 phys_addr, size_t size) {

    int fd;
    if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        printf("/dev/mem could not be opened.\n");
    }

    addr_t addr;

    addr.phys_addr = phys_addr;
        
    // mmap
    addr.virt_addr = (u64) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr.phys_addr);

    if(addr.virt_addr == (u64)MAP_FAILED) {
        printf("Failed to mmap %s\n", strerror(errno));
    }

    close(fd);

    return addr;
}

void smem_dealloc(addr_t* addr, size_t size) {
    munmap((void*)addr->virt_addr, size);
}