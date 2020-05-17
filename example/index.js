const w2xcjs = require("../lib");
const fs = require('fs');

const converter = new w2xcjs.W2XCJS(1, 0, 0);

console.log(converter.loadModels(w2xcjs.DEFAULT_MODELS_DIR));
console.log(converter.getConv());
console.log(converter.convertFile("in.png", "out.webp"));

const file = fs.readFileSync('in.png');

const newfile = converter.convertBuffer(file, '.JPG');

fs.writeFileSync("out.jpg", newfile);