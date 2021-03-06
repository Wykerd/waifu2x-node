"use strict";
var os = require('os');
var fs = require('fs');
var path = require('path');
var log = require('npmlog');
var get = require('simple-get');
if (os.platform() === 'win32') {
    var child_process = require('child_process');
    function locate7z() {
        if (fs.existsSync("C:\\Program Files\\7-Zip\\7z.exe")) {
            return "C:\\Program Files\\7-Zip\\7z.exe";
        } else if (fs.existsSync("C:\\Program Files (x86)\\7-Zip\\7z.exe")) {
            return "C:\\Program Files (x86)\\7-Zip\\7z.exe";
        } else return false;
    }
    var decompress_path = locate7z();
    if (!decompress_path) {
        throw new Error("Could not locate 7-zip in default install locations. Please install to C:\\Program Files\\7-Zip\\7z.exe or C:\\Program Files (x86)\\7-Zip\\7z.exe");
    };
    var include_path = path.resolve(__dirname, "../vendor/w2xc/include");
    if (!fs.existsSync(include_path)) fs.mkdirSync(include_path, { recursive: true });
    log.info("W2XCJS", "Downloading OpenCV binaries");
    get.concat({url:"https://sourceforge.net/projects/opencvlibrary/files/4.3.0/opencv-4.3.0-vc14_vc15.exe/download", followRedirects: true}, function (err, _res, data) {
        if (err) throw err;
        var arch_path = path.resolve(__dirname, "opencv_archive.exe");
        fs.writeFileSync(arch_path, data);
        log.info("W2XCJS", "Extracting OpenCV binaries");
        child_process.execSync(`"${decompress_path}" x "${arch_path}" -o"${path.resolve(__dirname, "../vendor")}" opencv/build/include/* opencv/build/x64/vc15/*`);
        fs.unlinkSync(arch_path);
    });
    log.info('W2XCJS', 'Dowloading waifu2x-converter-cpp binaries');
    get.concat({url:"https://github.com/DeadSix27/waifu2x-converter-cpp/releases/download/v5.3.3/waifu2x-DeadSix27-win64_v533.zip", followRedirects: true}, function (err, _res, data) {
        if (err) throw err;
        var arch_path = path.resolve(__dirname, "w2xconvcpp_archive.zip");
        fs.writeFileSync(arch_path, data);
        log.info("W2XCJS", "Extracting waifu2x-converter-cpp binaries");
        child_process.execSync(`"${decompress_path}" x "${arch_path}" -o"${path.resolve(__dirname, "../vendor/w2xc")}" *.lib *.exp *.dll models_rgb/*`);
        fs.unlinkSync(arch_path);
    });
    log.info("W2XCJS", "Downloading w2xconv headers");
    get.concat("https://raw.githubusercontent.com/DeadSix27/waifu2x-converter-cpp/master/src/w2xconv.h", function (err, _res, data) {
        if (err) throw err;
        fs.writeFileSync(path.resolve(include_path, "w2xconv.h"), data);
    });
} else if (os.platform() === 'linux') {
    var path_to_models = fs.existsSync("/usr/local/share/waifu2x-converter-cpp") ? "/usr/local/share/waifu2x-converter-cpp" : fs.existsSync("/usr/share/waifu2x-converter-cpp") ? "/usr/share/waifu2x-converter-cpp" : false;
    var w2xc_path = path.resolve(__dirname, "../vendor/w2xc");
    if (!fs.existsSync(w2xc_path)) fs.mkdirSync(w2xc_path, { recursive: true });
    if (path_to_models) {
        try {
            log.info("W2XCJS", "Using existing models found in " + path_to_models);
            fs.symlinkSync(path_to_models, path.resolve(w2xc_path, "models_rgb"), 'dir');   
        } catch (error) {
            if (error.code !== "EEXIST") throw error;
        }
    } else {
        // Download the models
        var models_path = path.resolve(w2xc_path, "models_rgb");
        if (!fs.existsSync(models_path)) fs.mkdirSync(models_path, { recursive: true });
        if (!fs.existsSync())
        var saveToFile = function (name) {
            return function (err, _res, data) {
                if (err) throw err;
                fs.writeFileSync(path.resolve(models_path, name), data);
                log.info("W2XCJS", "Downloaded " + name);
            }
        }
        log.info("W2XCJS", "Could not locate existing models, downloading from source...");
        get.concat("https://raw.githubusercontent.com/DeadSix27/waifu2x-converter-cpp/master/models_rgb/noise0_model.json", saveToFile("noise0_model.json"));
        get.concat("https://raw.githubusercontent.com/DeadSix27/waifu2x-converter-cpp/master/models_rgb/noise1_model.json", saveToFile("noise1_model.json"));
        get.concat("https://raw.githubusercontent.com/DeadSix27/waifu2x-converter-cpp/master/models_rgb/noise2_model.json", saveToFile("noise2_model.json"));
        get.concat("https://raw.githubusercontent.com/DeadSix27/waifu2x-converter-cpp/master/models_rgb/noise3_model.json", saveToFile("noise3_model.json"));
        get.concat("https://raw.githubusercontent.com/DeadSix27/waifu2x-converter-cpp/master/models_rgb/scale2.0x_model.json", saveToFile("scale2.0x_model.json"));
    }
}