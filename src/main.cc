#define HAVE_OPENCV
#include <w2xconv.h>
#include <opencv2/imgcodecs.hpp>
#include <node.h>
#include <stdio.h>
#include "w2xcjs.h"

namespace w2xcjs {
    using v8::Local;
    using v8::Object;

    void Initialize(Local<Object> exports) {
        // NODE_SET_METHOD(exports, "convertFile", convert_file);
        W2XCJS::Init(exports);
    }

    NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize);
}