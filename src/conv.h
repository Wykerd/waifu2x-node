#ifndef CONV_H
#define CONV_H

#define HAVE_OPENCV

#include <vector>
#include <w2xconv.h>

void w2xcjs_convert_buf
(
    struct W2XConv *conv,
    char* image_src, 
    size_t image_src_len,
    std::vector<uint8_t> &image_dst,
    char* dst_ext,
    int denoise_level,
	double scale,
    std::vector<int> * imwrite_params
); 

#endif