const w2xcjs = require("../lib");

const converter = new w2xcjs.W2XCJS(1, 0, 0);

console.log(converter.loadModels("/usr/local/share/waifu2x-converter-cpp"));
console.log(converter.getConv());
console.log(converter.convertFile("in.png", "out.webp"));