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

    size_t sizeInBytes = width*height*channels;

    cv::Size s(width, height);

    struct timespec t1, t2;

    addr_t shared_mem = smem_alloc((u32) SHARED_MEM, sizeInBytes);

    //
    // read image from shared mem
    clock_gettime(CLOCK_REALTIME, &t1);

    cv::Mat frame_copy = cv::Mat(s, CV_8UC3, (uint8_t*)(shared_mem.virt_addr));

    clock_gettime(CLOCK_REALTIME, &t2);

    // ---------------------------------------------------------- //

    // time
    std::cout << "read: " << ((t2.tv_sec-t1.tv_sec)+(t2.tv_nsec-t1.tv_nsec)/1000000000.0)*1000 << " ms" << std::endl;

    cv::imshow("Display window", frame_copy);
    cv::waitKey(0);

    smem_dealloc(&shared_mem, sizeInBytes);

    return EXIT_SUCCESS;
}