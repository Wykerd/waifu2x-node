import W2XCJS from './constructor';
import Types from './types';
import fs from 'fs';
import path from 'path';
import log from 'npmlog';
import os from 'os';

// The Windows of w2xc does not allow you to use w2xconv_convert_mat due to the way it links, so on Windows we'll have to use this hack to make it work. 
// I don't recommend you use this in production, please run linux in production!

/**
 * Apply pollyfill to add convertBuffer support to Windows.
 * @param showWarning Should a warning be outputted to console if the pollyfill is used.
 */
export default function applyPollyfil (showWarning : boolean = true) {
    W2XCJS.prototype.convertBuffer = function (src_buffer: Buffer, dst_ext: string, denoise_level: number = -1, scale: number = 2.0, imwrite_params: Types.W2XJSImwriteParams = { webp_quality: 101, jpeg_quality: 90, png_compression: 5 }) : Buffer {
        if (showWarning) log.warn('W2XCJS', 'W2XCJS.convertBuffer is not natively supported on Windows! Using a pollyfill instead... Please use Linux in production!');
        const input_file_path = path.resolve(os.tmpdir(), Date.now() + '_w2xcjs_in');
        fs.writeFileSync(input_file_path, src_buffer);
        const output_file_path = path.resolve(os.tmpdir(), Date.now() + '_w2xcjs_out' + dst_ext);
        this.convertFile(input_file_path, output_file_path, denoise_level, scale, imwrite_params);
        const ret_buffer = fs.readFileSync(output_file_path);
        fs.unlinkSync(input_file_path);
        fs.unlinkSync(output_file_path);
        return ret_buffer;
    }
}
