import W2XCJS from './constructor';
import Types from './types';

export default class W2XCJSPromises {
    public converter : W2XCJS;
    
    constructor (converter : W2XCJS) {
        this.converter = converter;
    }

    public convertBuffer(src_buffer: Buffer, dst_ext: string, options : Types.AsyncOptions) : Promise<Buffer> {
        return new Promise(resolve => this.converter.convertBufferAsync(src_buffer, dst_ext, options, resolve));
    }
}