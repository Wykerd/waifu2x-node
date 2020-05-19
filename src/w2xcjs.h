#ifndef W2XCJS_H
#define W2XCJS_H

#include <w2xconv.h>
#include <node.h>
#include <node_object_wrap.h>
#include <opencv2/opencv.hpp>
#include <uv.h>

namespace w2xcjs {
    struct ConvertBufferWork {
        uv_work_t  request;
        v8::Persistent<v8::Function> callback;
        char* image_src;
        char* dst_ext;
        int denoise_level;
	    double scale;
        size_t image_src_len;
        W2XConv * conv;
        std::vector<int> imwrite_params;
        std::vector<uchar> image_dst;
    };

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

            static void ConvertBufferAsync(const v8::FunctionCallbackInfo<v8::Value>& args);

            static void ConvertBufferWorkAsync(uv_work_t *req);

            static void ConvertBufferWorkAsyncComplete(uv_work_t *req, int status);

            W2XConv *conv_;
    };
}

#endif