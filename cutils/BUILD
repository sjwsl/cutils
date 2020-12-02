
cc_library(
    name = "cutils",
    srcs = [
        "async_worker.cpp",
        "coding.cpp",
        "cutils.cpp",
        "file.cpp",
        "freq_ctrl.cpp",
        "random.cpp",
    ],
    hdrs = [
        "async_worker.h",
        "bitops.h",
        "chash.h",
        "coding.h",
        "cqueue.h",
        "cutils.h",
        "file.h",
        "random.h",
        "rob.h",
        "singleton.h",
        "slice.h",
        "timer.h",
        "circle_queue.h",
    ],
    includes = ['.'],
    copts = [
        "-std=c++11",
    ],
    linkopts = [
        "-lpthread",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "crc32c",
    srcs = [
        "crc32c.cpp",
    ],
    hdrs = [
        "crc32c.h",
    ],
    deps = [
        ":cutils",
    ],
    includes = ['.'],
    copts = [
        "-std=c++11",
        "-DHAVE_SSE42",
        "-msse4.2",
        "-mpclmul",
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "test",
    srcs = [
        "test.cpp",
    ],
    includes = ['.'],
    deps = [
        ":cutils",
        ":crc32c",
    ],
    copts = [
        "-std=c++11",
    ],
    linkopts = [
    ],
)

