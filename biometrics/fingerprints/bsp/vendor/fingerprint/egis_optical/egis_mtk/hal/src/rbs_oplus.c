
#include "rbs_oplus.h"


int egis_tp_enable() 
{
    int err = 0;
    do
    {
        char buff[2];
        int fd = open(FP_ENABLE_TP_PATH, O_WRONLY);
        if(fd < 0) {
            err =  fd;
            egislog_e("[%s] failed to open ENABLE TP file\n", __func__);
            break;
        }
        sprintf(buff, "%d", (int)EGIS_TP_ENABLE);
        write(fd, buff, 2);
        close(fd);
    }
    while (0);

    return err;
}

int egis_tp_disable() 
{
    int err = 0;
    do
    {
        char buff[2];
        int fd = open(FP_ENABLE_TP_PATH, O_WRONLY);
        if(fd < 0) {
            err =  fd;
            egislog_e("[%s] failed to open ENABLE TP file\n", __func__);
            break;
        }
        sprintf(buff, "%d", (int)EGIS_TP_DISABLE);
        write(fd, buff, 2);
        close(fd);
    }
    while (0);

    return err;
}
