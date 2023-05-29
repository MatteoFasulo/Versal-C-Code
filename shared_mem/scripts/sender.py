import ctypes
import numpy as np
from copy import copy, deepcopy
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
# write image to shared memory
#   image_to_write: image to write on shared memory location
#   address:     shared memory location (virtual address)
#   sizeInBytes: size of the image
def send (image_to_write, address, sizeInBytes):
    image    = np.ctypeslib.as_array((ctypes.c_uint8 * sizeInBytes).from_address(address))
    image[:] = image_to_write.reshape(sizeInBytes)

#
# map shared address
addr = smem_alloc_functions.smem_alloc(SHARED_MEM, sizeInBytes)
#


# example image
image_tmp = cv2.imread("../test.jpg")
image_tmp = cv2.resize(image_tmp, (width, height), interpolation = cv2.INTER_AREA)
#


start_time = time.time()

send (image_tmp, addr.virt_addr, sizeInBytes)

print("send: " + str((time.time() - start_time) * 1000) + " ms")
#

#
# unmap shared address
smem_alloc_functions.smem_dealloc(addr, sizeInBytes)
#