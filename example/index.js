const { W2XCJS, DEFAULT_MODELS_DIR, W2XCJSPromises } = require('../lib');
const fs = require('fs');

const promises = new W2XCJSPromises(new W2XCJS());

const err = promises.converter.loadModels(DEFAULT_MODELS_DIR); // model loading is synchronous

if (!err) {
    (async () => {
        const input_buffer = await fs.promises.readFile("in.png");
        const dst_buffer = await promises.convertBuffer(input_buffer, '.WEBP', { /* AsyncOptions */ });
        await fs.promises.writeFile("out.webp", dst_buffer);
    })();
}