const w2xcjs = require("../lib");
const fs = require('fs');

const converter = new w2xcjs.W2XCJS(1, 0, 0);

console.log(converter.loadModels(w2xcjs.DEFAULT_MODELS_DIR));
console.log(converter.getConv());

const file = fs.readFileSync('../img.png');

/*
console.log(converter.convertFile("../img.png", "out.webp"));

const newfile = converter.convertBuffer(file, '.PNG');

fs.writeFileSync("out.png", newfile);
*/

converter.convertBufferAsync(file, '.PNG', {}, function (e) {
    fs.writeFileSync("out.png", e)
});

const promise = new w2xcjs.W2XCJSPromises(converter);

promise.convertBuffer(file, '.WEBP', {})
    .then(function (e) {
        fs.writeFileSync("out_async.webp", e)
    });