{
    "targets": [
        {
            "target_name": "w2xcjs",
            "sources": [ "src/main.cc", "src/w2xcjs.cc" ],
            'cflags_cc!': [ '-fno-rtti' ],
            'cflags_cc': [ '-fexceptions' ],
            "conditions": [
                [
                    'OS=="win"',
                    {
                        "include_dirs": [
                            "vendor/w2xc/include",
                            "vendor/opencv/build/include",
                            "<!(node -e \"require('nan')\")"
                        ],
                        "libraries": [
                            "../vendor/opencv/build/x64/vc15/lib/opencv_world430.lib",
                            "../vendor/w2xc/w2xc.lib"
                        ]
                    }
                ],
                [
                    'OS=="linux"',
                    {
                        "include_dirs" : [
                            "/usr/include/opencv4",
                            "<!(node -e \"require('nan')\")"
                        ],
                        "libraries": [
                            "-lw2xc",
                            "-lopencv_core",
                            "-lopencv_imgproc",
                            "-lopencv_imgcodecs"
                        ]
                    }
                ]
            ]
        }
    ],
}