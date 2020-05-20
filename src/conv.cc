#include "conv.h"

#include <w2xconv.h>
#include <opencv2/imgcodecs.hpp>

void w2xcjs_convert_buf
(
    struct W2XConv *conv,
    char* image_src, 
    size_t image_src_len,
    std::vector<uchar> &image_dst,
    char* dst_ext,
    int denoise_level,
	double scale,
    std::vector<int> * imwrite_params
)
{
    cv::Mat rawData(1, image_src_len, CV_8UC1, (void *)image_src);

    cv::Mat im_src = cv::imdecode(rawData, cv::IMREAD_UNCHANGED);

    cv::Mat im_dst(im_src.size().height * 2, im_src.size().width * 2, CV_8UC3);

    bool has_alpha = im_src.channels() == 4;
    
    if (has_alpha) {
        std::vector<cv::Mat> src_channels(4);
        cv::split(im_src, src_channels);
        std::vector<cv::Mat> rgb(src_channels.begin(), src_channels.end() - 1);
        im_src.release();
        cv::merge(rgb, im_src);
        w2xconv_convert_rgb(
            conv,
            im_dst.data, im_dst.step[0],
            im_src.data, im_src.step[0],
            im_src.size().width, im_src.size().height,
            denoise_level, scale, 0
        );
        cv::Mat image_dst_rgba;
        cv::cvtColor(im_dst, image_dst_rgba , cv::COLOR_RGB2RGBA);
        im_dst.release();
        im_dst = image_dst_rgba;
        std::vector<cv::Mat> dst_channels(4);
        cv::split(im_dst, dst_channels);
        cv::resize(src_channels[3], dst_channels[3], im_dst.size(), 0, 0, cv::INTER_LINEAR);
        cv::merge(dst_channels, im_dst);
    } else {
        w2xconv_convert_rgb(
            conv,
            im_dst.data, im_dst.step[0],
            im_src.data, im_src.step[0],
            im_src.size().width, im_src.size().height,
            denoise_level, scale, 0
        );
    }

    cv::imencode(dst_ext, im_dst, image_dst, *imwrite_params);

    im_src.release();
    rawData.release();
    im_dst.release();
}