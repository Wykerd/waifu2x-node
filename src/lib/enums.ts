namespace Enums {
    export enum W2XConvGPUMode {
        W2XCONV_GPU_DISABLE = 0,
        W2XCONV_GPU_AUTO = 1,
        W2XCONV_GPU_FORCE_OPENCL = 2
    };
    
    export enum W2XConvProcessorType {
        W2XCONV_PROC_UNKNOWN = -1,
        W2XCONV_PROC_HOST = 0,
        W2XCONV_PROC_CUDA = 1,
        W2XCONV_PROC_OPENCL = 2
    };
    
    export enum W2XConvProcessorSubType {
        W2XCONV_PROC_HOST_OPENCV = 0x0000,
        W2XCONV_PROC_HOST_SSE3 = 0x0001,
        W2XCONV_PROC_HOST_AVX = 0x0002,
        W2XCONV_PROC_HOST_FMA = 0x0003,
        W2XCONV_PROC_HOST_NEON = 0x0104,
        W2XCONV_PROC_HOST_ALTIVEC = 0x0205
    }

    export enum W2XConvErrorCode {
        W2XCONV_NOERROR,
        W2XCONV_ERROR_WIN32_ERROR,	/* errno_ = GetLastError() */
        W2XCONV_ERROR_WIN32_ERROR_PATH, /* u.win32_path */
        W2XCONV_ERROR_LIBC_ERROR,	/* errno_ */
        W2XCONV_ERROR_LIBC_ERROR_PATH,	/* libc_path */

        W2XCONV_ERROR_MODEL_LOAD_FAILED, /* u.path */

        W2XCONV_ERROR_IMREAD_FAILED,	/* u.path */
        W2XCONV_ERROR_IMWRITE_FAILED,	/* u.path */

        W2XCONV_ERROR_RGB_MODEL_MISMATCH_TO_Y,
        W2XCONV_ERROR_Y_MODEL_MISMATCH_TO_RGB_F32,

        W2XCONV_ERROR_OPENCL,	/* u.cl_error */

        W2XCONV_ERROR_SCALE_LIMIT,
        W2XCONV_ERROR_SIZE_LIMIT,
        W2XCONV_ERROR_WEBP_SIZE_LIMIT,
        W2XCONV_ERROR_WEBP_LOSSY_SIZE_LIMIT
    }
}

export default Enums;