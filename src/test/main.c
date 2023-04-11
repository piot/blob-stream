#include "utest.h"
#include <clog/clog.h>
#include <clog/console.h>

clog_config g_clog;

UTEST_STATE();

int main(int argc, const char *const argv[])
{
    g_clog.log = clog_console;
    CLOG_INFO("tests are running")
    return utest_main(argc, argv);
}