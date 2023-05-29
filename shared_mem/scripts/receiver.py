import ctypes
import numpy as np
import cv2
import time

from ctypes import *

SHARED_MEM  = 0x10000000
height      = 480
width       = 640
channels    = 3

lib = "../build/libsmem_alloc.so"

smem_alloc_functions = CDLL(lib)

class addr_t(Structure):
    _fields_ = [('phys_addr', c_uint64),
                ('virt_addr', c_uint64)]

# alloc function -------------------------------------- #
smem_alloc_functions.smem_alloc.restype = addr_t
# ----------------------------------------------------- #

# dealloc function ------------------------------------ #
smem_dealloc         = smem_alloc_functions.smem_dealloc
smem_dealloc.restype = None
smem_dealloc.argtypes= [POINTER(addr_t)]
# ----------------------------------------------------- #

sizeInBytes = height*width*channels


#
# read image from shared memory
#   address:     shared memory location (virtual address)
#   sizeInBytes: size of the image
def receive (address, sizeInBytes):
    image = np.ctypeslib.as_array((ctypes.c_uint8 * sizeInBytes).from_address(address))
    image = image.reshape(height, width, channels)

    return image

#
# map shared address
addr = smem_alloc_functions.smem_alloc(SHARED_MEM, sizeInBytes)
#


start_time = time.time()

image = receive(addr.virt_addr, sizeInBytes)

print("recv: " + str((time.time() - start_time) * 1000) + " ms")

cv2.imshow("frame", image)
cv2.waitKey(0)

#
# unmap shared address
smem_alloc_functions.smem_dealloc(addr, sizeInBytes)
#