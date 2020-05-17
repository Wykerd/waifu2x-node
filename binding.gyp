{
    "targets": [
        {
            "target_name": "w2xcjs",
            "sources": [ "src/main.cc", "src/w2xcjs.cc" ],
            "conditions": [
                [
                    'OS=="win"',
                    {
                        "include_dirs": [
                            "vendor/w2xc/include",
                            "vendor/opencv/build/include"
                        ],
                        "libraries": [
                            "vendor/opencv/build/x64/vc15/lib/opencv_world430.lib",
                            "vendor/w2xc/w2xc.lib"
                        ]
                    }
                ],
                [
                    'OS=="linux"',
                    {
                        "libraries": [
                            "-lw2xc",
                            "-lopencv_imgcodecs"
                        ]
                    }
                ]
            ]
        }
    ],
}