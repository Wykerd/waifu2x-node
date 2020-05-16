#define HAVE_OPENCV
#include <w2xconv.h>
#include <node.h>
#include <node_object_wrap.h>

// W2XCONV_EXPORT struct W2XConv *w2xconv_init(enum W2XConvGPUMode gpu, int njob /* 0 = auto */, int log_level);

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

            W2XConv *conv_;
    };
}