#define HAVE_OPENCV
#include "w2xcjs.h"
#include "conv.h"

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
        NODE_SET_PROTOTYPE_METHOD(tpl, "convertBufferAsync", ConvertBufferAsync);

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

    void W2XCJS::ConvertBufferWorkAsyncComplete(uv_work_t *req, int status) {
        Isolate * isolate = Isolate::GetCurrent();
        v8::HandleScope handleScope(isolate);

        ConvertBufferWork *work = static_cast<ConvertBufferWork *>(req->data);

        Local<Object> resobj = node::Buffer::Copy(isolate, (char *)work->image_dst.data(), work->image_dst.size()).ToLocalChecked();

        Local<Value> argv[] = { resobj };

        Local<Function>::New(isolate, work->callback)->
            Call(isolate->GetCurrentContext(), isolate->GetCurrentContext()->Global(), 1, argv);

        work->callback.Reset();

        free(work->dst_ext);

        delete work;
    }

    void W2XCJS::ConvertBufferWorkAsync(uv_work_t *req) {
        ConvertBufferWork *work = static_cast<ConvertBufferWork *>(req->data);

        w2xcjs_convert_buf(
            work->conv, 
            work->image_src, 
            work->image_src_len, 
            work->image_dst, 
            work->dst_ext, 
            work->denoise_level, 
            work->scale, 
            &work->imwrite_params
        );
    }

    void W2XCJS::ConvertBufferAsync(const v8::FunctionCallbackInfo<v8::Value>& args) {
        Isolate* isolate = args.GetIsolate();
        Local<Context> context = isolate->GetCurrentContext();

        W2XCJS *obj = ObjectWrap::Unwrap<W2XCJS>(args.Holder());

        if (obj->conv_->target_processor->type == W2XCONV_PROC_HOST) {
            isolate->ThrowException(
                Exception::Error(
                    String::NewFromUtf8(isolate, "Async methods are not supported on W2XCONV_PROC_HOST", NewStringType::kNormal).ToLocalChecked()
                )
            );
            return;
        }

        // Args are:
        // 0: input_buffer
        // 1: out_ext
        // 2: options
        // 3: callback

        if  (args.Length() < 4) {
            isolate->ThrowException(
                Exception::TypeError(
                    String::NewFromUtf8(isolate, "Wrong number of arguments", NewStringType::kNormal).ToLocalChecked()
                )
            );
            return;
        };

        if (!args[0]->IsObject() || !args[1]->IsString() || !args[2]->IsObject() || !args[3]->IsFunction()) {
            isolate->ThrowException(
                Exception::TypeError(
                    String::NewFromUtf8(isolate, "Wrong arguments", NewStringType::kNormal).ToLocalChecked()
                )
            );
            return;
        };

        ConvertBufferWork * work = new ConvertBufferWork();
        work->request.data = work;
        
        Local<Function> cb = Local<Function>::Cast(args[3]);

        work->callback.Reset(isolate, cb);

        uint8_t webp_res = 101, jpeg_res = 101, png_res = 5;

        work->denoise_level = -1;

        work->scale = 2.0;

        String::Utf8Value dst_param(isolate, args[1]);

        work->dst_ext = (char *)malloc(strlen(*dst_param));

        strcpy(work->dst_ext, *dst_param);

        Local<Object> options = args[2]->ToObject(context).ToLocalChecked();
        Local<Array> opt_props = options->GetOwnPropertyNames(context).ToLocalChecked();
        uint32_t opt_prop_len = opt_props->Length();

        for (uint32_t i = 0; i < opt_prop_len; i++) {
            Local<Value> lockey = opt_props->Get(context, i).ToLocalChecked();
            Local<Value> locval = options->Get(context, lockey).ToLocalChecked();
            char* key = *String::Utf8Value(isolate, locval);
            if (!strcmp(key, "imwrite_params")) {
                if (locval->IsObject()) {
                    Local<Object> imwrite_opts = args[4]->ToObject(context).ToLocalChecked();

                    Local<Array> props = imwrite_opts->GetOwnPropertyNames(context).ToLocalChecked();
                    
                    uint32_t propLen = props->Length();
                    
                    for (uint32_t i = 0; i < propLen; i++) {
                        Local<Value> lockey_ = props->Get(context, i).ToLocalChecked();
                        Local<Value> locval_ = imwrite_opts->Get(context, lockey_).ToLocalChecked();
                        char* key_ = *String::Utf8Value(isolate, lockey_);
                        if (locval_->IsNumber()) {
                            if (!strcmp(key_, "webp_quality")) {
                                webp_res = locval_->IntegerValue(context).FromMaybe(webp_res);
                            } else if (!strcmp(key_, "jpeg_quality")) {
                                jpeg_res = locval_->IntegerValue(context).FromMaybe(jpeg_res);
                            } else if (!strcmp(key_, "png_compression")) {
                                png_res = locval_->IntegerValue(context).FromMaybe(png_res);
                            }
                        }
                    }
                }
            } else if (!strcmp(key, "scale")) {
                work->scale = locval->NumberValue(context).FromMaybe(2.0);
            } else if (!strcmp(key, "denoise_level")) {
                work->denoise_level = locval->IntegerValue(context).FromMaybe(-1);
            }
        }

        work->imwrite_params.push_back(cv::IMWRITE_WEBP_QUALITY);
        work->imwrite_params.push_back(webp_res);
        work->imwrite_params.push_back(cv::IMWRITE_JPEG_QUALITY);
        work->imwrite_params.push_back(jpeg_res);
        work->imwrite_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
        work->imwrite_params.push_back(png_res);

        Local<Object> buffer = args[0]->ToObject(context).ToLocalChecked();

        work->image_src = node::Buffer::Data(buffer);
        work->image_src_len = node::Buffer::Length(buffer);

        work->conv = obj->conv_;

        uv_queue_work(uv_default_loop(), &work->request, ConvertBufferWorkAsync, ConvertBufferWorkAsyncComplete);

        args.GetReturnValue().Set(Undefined(isolate));
    } 

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

        uint8_t webp_res = 101, jpeg_res = 101, png_res = 5;

        if (args.Length() > 4 && args[4]->IsObject()) {
            Local<Object> imwrite_opts = args[4]->ToObject(context).ToLocalChecked();

            Local<Array> props = imwrite_opts->GetOwnPropertyNames(context).ToLocalChecked();
            
            uint32_t propLen = props->Length();
            
            for (uint32_t i = 0; i < propLen; i++) {
                Local<Value> lockey = props->Get(context, i).ToLocalChecked();
                Local<Value> locval = imwrite_opts->Get(context, lockey).ToLocalChecked();
                char* key = *String::Utf8Value(isolate, lockey);
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

        Local<Object> buffer = args[0]->ToObject(context).ToLocalChecked();

        char* input_buffer = node::Buffer::Data(buffer);
        size_t input_length = node::Buffer::Length(buffer);

        int denoise_level = args.Length() > 2 ? args[2]->IntegerValue(context).FromMaybe(-1) : -1;

        double scale = args.Length() > 3 ? args[3]->NumberValue(context).FromMaybe(2.0) : 2.0;

        std::vector<int> compression_params;
        compression_params.push_back(cv::IMWRITE_WEBP_QUALITY);
        compression_params.push_back(webp_res);
        compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
        compression_params.push_back(jpeg_res);
        compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
        compression_params.push_back(png_res);

        std::vector<uchar> output_buffer;

        W2XCJS *obj = ObjectWrap::Unwrap<W2XCJS>(args.Holder());

        w2xcjs_convert_buf(
            obj->conv_, 
            input_buffer, 
            input_length, 
            output_buffer, 
            *String::Utf8Value(isolate, args[1]), 
            denoise_level, 
            scale, 
            &compression_params
        );

        args.GetReturnValue().Set(node::Buffer::Copy(isolate, (char *)output_buffer.data(), output_buffer.size()).ToLocalChecked());
    }

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

        uint8_t webp_res = 101, jpeg_res = 101, png_res = 5;

        W2XCJS *obj = ObjectWrap::Unwrap<W2XCJS>(args.Holder());

        if (args.Length() > 4 && args[4]->IsObject()) {
            Local<Object> imwrite_opts = args[4]->ToObject(context).ToLocalChecked();

            Local<Array> props = imwrite_opts->GetOwnPropertyNames(context).ToLocalChecked();
            
            uint32_t propLen = props->Length();
            
            for (uint32_t i = 0; i < propLen; i++) {
                Local<Value> lockey = props->Get(context, i).ToLocalChecked();
                Local<Value> locval = imwrite_opts->Get(context, lockey).ToLocalChecked();
                char* key = *String::Utf8Value(isolate, lockey);
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