#include <smem_alloc.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <time.h>

//
// indirizzo di esempio relativo alla memoria condivisa
//
#define SHARED_MEM  0x10000000
const size_t height   = 480;
const size_t width    = 640;
const size_t channels = 3;
//

int main (int argc, char** argv) {

    struct timespec t1, t2;

    //
    // create example image
    clock_gettime(CLOCK_REALTIME, &t1);

    cv::Mat frame = cv::imread ("../test.jpg");

    clock_gettime(CLOCK_REALTIME, &t2);

    std::cout << "read from disk: " << ((t2.tv_sec-t1.tv_sec)+(t2.tv_nsec-t1.tv_nsec)/1000000000.0)*1000 << " ms" << std::endl;

    // ---------------------------------------------------------- //

    //
    // create example image
    clock_gettime(CLOCK_REALTIME, &t1);

    cv::imwrite("prova.jpg", frame);

    clock_gettime(CLOCK_REALTIME, &t2);

    std::cout << "write to disk: " << ((t2.tv_sec-t1.tv_sec)+(t2.tv_nsec-t1.tv_nsec)/1000000000.0)*1000 << " ms" << std::endl;

    // ---------------------------------------------------------- //

    cv::resize(frame, frame, cv::Size(width, height));
    cv::Size s = frame.size();

    size_t sizeInBytes = width*height*channels;

    addr_t shared_mem = smem_alloc((u32) SHARED_MEM, sizeInBytes);

    clock_gettime(CLOCK_REALTIME, &t1);

    //
    // copy image to shared mem
    memcpy((uint8_t*)shared_mem.virt_addr, frame.data, sizeInBytes);

    clock_gettime(CLOCK_REALTIME, &t2);

    // ---------------------------------------------------------- //

    // time
    std::cout << "write: " << ((t2.tv_sec-t1.tv_sec)+(t2.tv_nsec-t1.tv_nsec)/1000000000.0)*1000 << " ms" << std::endl;

    smem_dealloc(&shared_mem, sizeInBytes);

    return EXIT_SUCCESS;
}