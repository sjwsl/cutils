#include "cutils.h"
#include "crc32c.h"
#include <cstdio>
#include <unistd.h>
#include <vector>
#include <unordered_set>
#include <unordered_map>

using namespace cutils;

int main() {
    AsyncSeqTaskProfiler profiler;
    profiler.concur = 4;
    profiler.max_seq = 1200;
    profiler.beg_ts = time(0);
    profiler.end_ts = time(0) + 1000;
    profiler.worker_beg_ts.resize(4);
    profiler.worker_end_ts.resize(4);
    profiler.task_runtime.resize(1200);
    for (int i = 0; i < 4; ++i) {
        profiler.worker_beg_ts[i] = 100000 * i + 3;
        profiler.worker_end_ts[i] = 100000 * i + 100;
    }
    for (int i = 0; i < 1200; ++i) {
        profiler.task_runtime[i] = i + 1;
    }
    printf("%s\n", profiler.Format().c_str());
    return 0;
}

//gzrd_Lib_CPP_Version_ID--start
#ifndef GZRD_SVN_ATTR
#define GZRD_SVN_ATTR "0"
#endif
static char gzrd_Lib_CPP_Version_ID[] __attribute__((used))="$HeadURL: http://scm-gy.tencent.com/gzrd/gzrd_mail_rep/QQMailcore_proj/trunk/platform/cutils/test.cpp $ $Id: test.cpp 3029578 2019-04-12 04:06:27Z flashlin $ " GZRD_SVN_ATTR "__file__";
// gzrd_Lib_CPP_Version_ID--end

