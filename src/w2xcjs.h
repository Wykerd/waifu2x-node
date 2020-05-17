#ifndef W2XCJS_H
#define W2XCJS_H

#define HAVE_OPENCV
#include <w2xconv.h>
#include <node.h>
#include <node_object_wrap.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>

W2XCONV_EXPORT void w2xconv_convert_mat
(
	struct W2XConv *conv,
	cv::Mat* image_dst, 
	cv::Mat* image_src, 
	int denoise_level, 
	double scale, 
	int blockSize,
	w2xconv_rgb_float3 background,
	bool has_alpha,
	bool dst_alpha
);

namespace w2xcjs {
    class W2XCJS : public node::ObjectWrap {
        public:
            static void Init(v8::Local<v8::Object> exports);
            W2XCJS(enum W2XConvGPUMode gpu, int njob, int log_level);
            ~W2XCJS();
        private:
            static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

            static void GetConv(const v8::FunctionCallbackInfo<v8::Value>& args);

            static void LoadModels(const v8::FunctionCallbackInfo<v8::Value>& args);

            static void ConvertFile(const v8::FunctionCallbackInfo<v8::Value>& args);

            static void ConvertBuffer(const v8::FunctionCallbackInfo<v8::Value>& args);

            W2XConv *conv_;
    };
}

#endif