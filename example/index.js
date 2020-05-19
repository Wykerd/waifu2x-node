const w2xcjs = require("../lib");
const fs = require('fs');

const converter = new w2xcjs.W2XCJS(1, 0, 2);

console.log(converter.loadModels(w2xcjs.DEFAULT_MODELS_DIR));
// console.log(converter.getConv());
// console.log(converter.convertFile("../img.png", "out.webp"));

const file = fs.readFileSync('../img.png');

console.log(process.memoryUsage().heapUsed / 1024 / 1024);

let newfile;

for (let i = 0; i < 10; i++) {
    console.time('conv');
    newfile = converter.convertBuffer(file, '.PNG');
    console.timeEnd('conv');
    console.log(process.memoryUsage().heapUsed / 1024 / 1024);    
}

fs.writeFileSync("out.png", newfile);