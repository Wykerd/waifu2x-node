#include "w2xcjs.h"
#include <string.h>
#include <nan.h>
#include <opencv2/imgcodecs.hpp>

#if defined(_WIN32)
#include <string>
#define _UNICODE
#define _tstring std::wstring
#define str2wstr(X) std::wstring(X.begin(), X.end())
#endif

using namespace Nan;

namespace w2xcjs {
    using v8::Context;
    using v8::Function;
    using v8::FunctionCallbackInfo;
    using v8::FunctionTemplate;
    using v8::Isolate;
    using v8::Local;
    using v8::NewStringType;
    using v8::Number;
    using v8::Object;
    using v8::ObjectTemplate;
    using v8::String;
    using v8::Value;
    using v8::Exception;
    using v8::Array;

    W2XCJS::W2XCJS(enum W2XConvGPUMode gpu, int njob, int log_level) {
        conv_ = w2xconv_init(gpu, njob, log_level);
    }

    W2XCJS::~W2XCJS() {
        w2xconv_free(conv_);
    }

    void W2XCJS::Init(Local<Object> exports) {
        Isolate* isolate = exports->GetIsolate();
        Local<Context> context = isolate->GetCurrentContext();

        Local<ObjectTemplate> addon_data_tpl = ObjectTemplate::New(isolate);
        addon_data_tpl->SetInternalFieldCount(3); 

        Local<Object> addon_data = addon_data_tpl->NewInstance(context).ToLocalChecked();

        Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New, addon_data);
        tpl->SetClassName(String::NewFromUtf8(
            isolate, "W2XCJS", NewStringType::kNormal).ToLocalChecked());
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        NODE_SET_PROTOTYPE_METHOD(tpl, "getConv", GetConv);
        NODE_SET_PROTOTYPE_METHOD(tpl, "loadModels", LoadModels);
        NODE_SET_PROTOTYPE_METHOD(tpl, "convertFile", ConvertFile);
        NODE_SET_PROTOTYPE_METHOD(tpl, "convertBuffer", ConvertBuffer);

        Local<Function> constructor = tpl->GetFunction(context).ToLocalChecked();
        addon_data->SetInternalField(0, constructor);
        exports->Set(context, String::NewFromUtf8(
            isolate, "W2XCJS", NewStringType::kNormal).ToLocalChecked(),
                    constructor).FromJust();
    }

    void W2XCJS::New(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        Local<Context> context = isolate->GetCurrentContext();

        if (args.IsConstructCall()) {
            // Invoked as constructor: `new MyObject(...)`
            long gpu_num = args[0]->IsUndefined() ? 1 : args[0]->IntegerValue(context).FromMaybe(0);
            enum W2XConvGPUMode gpu;
            switch (gpu_num)
            {
                case 0:
                    gpu = W2XCONV_GPU_DISABLE;
                    break;
                case 1:
                    gpu = W2XCONV_GPU_AUTO;
                    break;
                case 2:
                    gpu = W2XCONV_GPU_FORCE_OPENCL;
                    break;
                default:
                    gpu = W2XCONV_GPU_AUTO;
                    break;
            }
            long job = args[1]->IsUndefined() ? 0 : args[1]->IntegerValue(context).FromMaybe(0);
            long log = args[2]->IsUndefined() ? 0 : args[2]->IntegerValue(context).FromMaybe(0);
            W2XCJS* obj = new W2XCJS(gpu, job, log);
            obj->Wrap(args.This());
            args.GetReturnValue().Set(args.This());
        } else {
            isolate->ThrowException(
                v8::Exception::TypeError(
                    String::NewFromUtf8(isolate, "Must be called as contructor", NewStringType::kNormal).ToLocalChecked()
                )
            );
        }
    }

    #if !defined(_WIN32)
    void W2XCJS::ConvertBuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
        Isolate* isolate = args.GetIsolate();
        Local<Context> context = isolate->GetCurrentContext();

        if  (args.Length() < 2) {
            isolate->ThrowException(
                Exception::TypeError(
                    String::NewFromUtf8(isolate, "Wrong number of arguments", NewStringType::kNormal).ToLocalChecked()
                )
            );
            return;
        };

        if (!args[0]->IsObject() || !args[1]->IsString()) {
            isolate->ThrowException(
                Exception::TypeError(
                    String::NewFromUtf8(isolate, "Wrong arguments", NewStringType::kNormal).ToLocalChecked()
                )
            );
            return;
        };

        uint8_t webp_res = 101, jpeg_res = 90, png_res = 5;

        if (args.Length() > 4 && args[4]->IsObject()) {
            Local<Object> imwrite_opts = args[4]->ToObject(context).ToLocalChecked();

            Local<Array> props = imwrite_opts->GetOwnPropertyNames(context).ToLocalChecked();
            
            uint32_t propLen = props->Length();
            
            for (uint32_t i = 0; i < propLen; i++) {
                Local<Value> lockey = props->Get(context, i).ToLocalChecked();
                Local<Value> locval = imwrite_opts->Get(context, lockey).ToLocalChecked();
                char* key = *String::Utf8Value(isolate, locval);
                if (locval->IsNumber()) {
                    if (!strcmp(key, "webp_quality")) {
                        webp_res = locval->IntegerValue(context).FromMaybe(webp_res);
                    }
                    if (!strcmp(key, "jpeg_quality")) {
                        jpeg_res = locval->IntegerValue(context).FromMaybe(jpeg_res);
                    }
                    if (!strcmp(key, "png_compression")) {
                        png_res = locval->IntegerValue(context).FromMaybe(png_res);
                    }
                }
            }
        }

        int denoise_level = args.Length() > 2 ? args[2]->IntegerValue(context).FromMaybe(-1) : -1;

        double scale = args.Length() > 3 ? args[3]->NumberValue(context).FromMaybe(2.0) : 2.0;
        
        Local<Object> buffer = args[0]->ToObject(context).ToLocalChecked();

        char* input_buffer = node::Buffer::Data(buffer);
        size_t input_length = node::Buffer::Length(buffer);

        cv::Mat rawData(1, input_length, CV_8UC1, (void *)input_buffer);

        cv::Mat image_src = cv::imdecode(rawData, cv::IMREAD_UNCHANGED);

        cv::Mat image_dst;

        w2xconv_rgb_float3 background;
        background.r = background.g = background.b = 1.0f;

        W2XCJS *obj = ObjectWrap::Unwrap<W2XCJS>(args.Holder());

        bool has_alpha = image_src.channels() == 4;

        w2xconv_convert_mat(
            obj->conv_,
            &image_dst,
            &image_src,
            denoise_level,
            scale,
            0,
            background,
            has_alpha,
            has_alpha
        );

        std::vector<uchar> output_buffer;

        std::vector<int> compression_params;
        compression_params.push_back(cv::IMWRITE_WEBP_QUALITY);
        compression_params.push_back(webp_res);
        compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
        compression_params.push_back(jpeg_res);
        compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
        compression_params.push_back(png_res);

        cv::imencode(*String::Utf8Value(isolate, args[1]), image_dst, output_buffer, compression_params);

        image_src.release();
        rawData.release();
        image_dst.release();

        args.GetReturnValue().Set(node::Buffer::Copy(isolate, (char *)output_buffer.data(), output_buffer.size()).ToLocalChecked());
    }
    #endif

    void W2XCJS::ConvertFile(const v8::FunctionCallbackInfo<v8::Value>& args) {
        Isolate* isolate = args.GetIsolate();
        Local<Context> context = isolate->GetCurrentContext();

        if  (args.Length() < 2) {
            isolate->ThrowException(
                Exception::TypeError(
                    String::NewFromUtf8(isolate, "Wrong number of arguments", NewStringType::kNormal).ToLocalChecked()
                )
            );
            return;
        };

        if (!args[0]->IsString() || !args[1]->IsString()) {
            isolate->ThrowException(
                Exception::TypeError(
                    String::NewFromUtf8(isolate, "Wrong arguments", NewStringType::kNormal).ToLocalChecked()
                )
            );
            return;
        };

        uint8_t webp_res = 101, jpeg_res = 90, png_res = 5;

        W2XCJS *obj = ObjectWrap::Unwrap<W2XCJS>(args.Holder());

        if (args.Length() > 4 && args[4]->IsObject()) {
            Local<Object> imwrite_opts = args[4]->ToObject(context).ToLocalChecked();

            Local<Array> props = imwrite_opts->GetOwnPropertyNames(context).ToLocalChecked();
            
            uint32_t propLen = props->Length();
            
            for (uint32_t i = 0; i < propLen; i++) {
                Local<Value> lockey = props->Get(context, i).ToLocalChecked();
                Local<Value> locval = imwrite_opts->Get(context, lockey).ToLocalChecked();
                char* key = *String::Utf8Value(isolate, locval);
                if (locval->IsNumber()) {
                    if (!strcmp(key, "webp_quality")) {
                        webp_res = locval->IntegerValue(context).FromMaybe(webp_res);
                    }
                    if (!strcmp(key, "jpeg_quality")) {
                        jpeg_res = locval->IntegerValue(context).FromMaybe(jpeg_res);
                    }
                    if (!strcmp(key, "png_compression")) {
                        png_res = locval->IntegerValue(context).FromMaybe(png_res);
                    }
                }
            }
        }

        int imwrite_params[] =
        {
            cv::IMWRITE_WEBP_QUALITY,
            webp_res,
            cv::IMWRITE_JPEG_QUALITY,
            jpeg_res,
            cv::IMWRITE_PNG_COMPRESSION,
            png_res
        };

        int denoise_level = args.Length() > 2 ? args[2]->IntegerValue(context).FromMaybe(-1) : -1;

        double scale = args.Length() > 3 ? args[3]->NumberValue(context).FromMaybe(2.0) : 2.0;

        #if defined(_WIN32)
        std::string out(*String::Utf8Value(isolate, args[1]));
        std::string in(*String::Utf8Value(isolate, args[0]));

        int res = w2xconv_convert_file(
            obj->conv_, 
            (char *)str2wstr(out).c_str(), 
            (char *)str2wstr(in).c_str(), 
            denoise_level, scale, 0, imwrite_params);
        #else
        int res = w2xconv_convert_file(
            obj->conv_, 
            *String::Utf8Value(isolate, args[1]), 
            *String::Utf8Value(isolate, args[0]), 
            denoise_level, scale, 0, imwrite_params);
        #endif

        args.GetReturnValue().Set(Number::New(isolate, res));
    }

    void W2XCJS::LoadModels(const FunctionCallbackInfo<Value>& args)  {
        Isolate* isolate = args.GetIsolate();

        if  (args.Length() < 1) {
            isolate->ThrowException(
                Exception::TypeError(
                    String::NewFromUtf8(isolate, "Wrong number of arguments", NewStringType::kNormal).ToLocalChecked()
                )
            );
            return;
        };

        if (!args[0]->IsString()) {
            isolate->ThrowException(
                Exception::TypeError(
                    String::NewFromUtf8(isolate, "Expected argument of type string", NewStringType::kNormal).ToLocalChecked()
                )
            );
            return;
        };

        W2XCJS *obj = ObjectWrap::Unwrap<W2XCJS>(args.Holder());

        #if defined(_WIN32)
        std::string models_dir(*String::Utf8Value(isolate, args[0]));
        int res = w2xconv_load_models(obj->conv_, (char *)str2wstr(models_dir).c_str());
        #else
        String::Utf8Value models_dir(isolate, args[0]);
        int res = w2xconv_load_models(obj->conv_, *models_dir);
        #endif

        args.GetReturnValue().Set(Number::New(isolate, res));
    }

    void W2XCJS::GetConv(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        Local<Context> context = isolate->GetCurrentContext();

        W2XCJS *obj = ObjectWrap::Unwrap<W2XCJS>(args.Holder());

        Local<Object> conv = Object::New(isolate);

        Local<Object> target_processor = Object::New(isolate);

        int proc_type = -1;

        switch (obj->conv_->target_processor->type)
        {
            case W2XCONV_PROC_HOST:
                proc_type = 0;
                break;
            case W2XCONV_PROC_CUDA:
                proc_type = 1;
                break;
            case W2XCONV_PROC_OPENCL:
                proc_type = 3;
                break;
        }

        target_processor->Set(
            context,
            String::NewFromUtf8(isolate, "type", NewStringType::kNormal).ToLocalChecked(),
            Number::New(isolate, proc_type))
        .FromJust();

        target_processor->Set(
            context,
            String::NewFromUtf8(isolate, "sub_type", NewStringType::kNormal).ToLocalChecked(),
            Number::New(isolate, obj->conv_->target_processor->sub_type))
        .FromJust();

        target_processor->Set(
            context,
            String::NewFromUtf8(isolate, "dev_id", NewStringType::kNormal).ToLocalChecked(),
            Number::New(isolate, obj->conv_->target_processor->dev_id))
        .FromJust();

        target_processor->Set(
            context,
            String::NewFromUtf8(isolate, "num_core", NewStringType::kNormal).ToLocalChecked(),
            Number::New(isolate, obj->conv_->target_processor->num_core))
        .FromJust();

        target_processor->Set(
            context,
            String::NewFromUtf8(isolate, "dev_name", NewStringType::kNormal).ToLocalChecked(),
            String::NewFromUtf8(isolate, obj->conv_->target_processor->dev_name, NewStringType::kNormal).ToLocalChecked())
        .FromJust();

        Local<Object> last_error = Object::New(isolate);

        int error_code = -1;

        switch (obj->conv_->last_error.code)
        {
            case W2XCONV_NOERROR:
                error_code = 0;
                break;
            case W2XCONV_ERROR_WIN32_ERROR:
                error_code = 1;
                break;
            case W2XCONV_ERROR_WIN32_ERROR_PATH:
                error_code = 2;
                break;
            case W2XCONV_ERROR_LIBC_ERROR:
                error_code = 3;
                break;
            case W2XCONV_ERROR_LIBC_ERROR_PATH:
                error_code = 4;
                break;
            case W2XCONV_ERROR_MODEL_LOAD_FAILED:
                error_code = 5;
                break;
            case W2XCONV_ERROR_IMREAD_FAILED:
                error_code = 6;
                break;
            case W2XCONV_ERROR_IMWRITE_FAILED:
                error_code = 7;
                break;
            case W2XCONV_ERROR_RGB_MODEL_MISMATCH_TO_Y:
                error_code = 8;
                break;
            case W2XCONV_ERROR_Y_MODEL_MISMATCH_TO_RGB_F32:
                error_code = 9;
                break;
            case W2XCONV_ERROR_OPENCL:
                error_code = 10;
                break;
            case W2XCONV_ERROR_SCALE_LIMIT:
                error_code = 11;
                break;
            case W2XCONV_ERROR_SIZE_LIMIT:
                error_code = 12;
                break;
            case W2XCONV_ERROR_WEBP_SIZE_LIMIT:
                error_code = 13;
                break;
            case W2XCONV_ERROR_WEBP_LOSSY_SIZE_LIMIT:
                error_code = 14;
                break;
        }

        last_error->Set(
            context,
            String::NewFromUtf8(isolate, "code", NewStringType::kNormal).ToLocalChecked(),
            Number::New(isolate, error_code))
        .FromJust();

        Local<Object> flop = Object::New(isolate);

        flop->Set(
            context,
            String::NewFromUtf8(isolate, "flop", NewStringType::kNormal).ToLocalChecked(),
            Number::New(isolate, obj->conv_->flops.flop))
        .FromJust();

        flop->Set(
            context,
            String::NewFromUtf8(isolate, "filter_sec", NewStringType::kNormal).ToLocalChecked(),
            Number::New(isolate, obj->conv_->flops.filter_sec))
        .FromJust();

        flop->Set(
            context,
            String::NewFromUtf8(isolate, "process_sec", NewStringType::kNormal).ToLocalChecked(),
            Number::New(isolate, obj->conv_->flops.process_sec))
        .FromJust();

        conv->Set(
            context, 
            String::NewFromUtf8(isolate, "flops", NewStringType::kNormal).ToLocalChecked(), 
            flop)
        .FromJust();

        conv->Set(
            context,
            String::NewFromUtf8(isolate, "log_level", NewStringType::kNormal).ToLocalChecked(),
            Number::New(isolate, obj->conv_->log_level))
        .FromJust();

        conv->Set(
            context,
            String::NewFromUtf8(isolate, "tta_mode", NewStringType::kNormal).ToLocalChecked(),
            v8::Boolean::New(isolate, obj->conv_->tta_mode))
        .FromJust();

        conv->Set(
            context, 
            String::NewFromUtf8(isolate, "last_error", NewStringType::kNormal).ToLocalChecked(), 
            last_error)
        .FromJust();

        conv->Set(
            context, 
            String::NewFromUtf8(isolate, "target_processor", NewStringType::kNormal).ToLocalChecked(), 
            target_processor)
        .FromJust();

        args.GetReturnValue().Set(conv);
    }
}