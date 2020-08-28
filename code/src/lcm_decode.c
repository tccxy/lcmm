/**
 * @file lcm_decode.c
 * @author zhao.wei (hw)
 * @brief 
 * @version 0.1
 * @date 2020-08-20
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include "pub.h"

/**
 * @brief Get the full time object
 * 
 * @param data 
 * @return int 
 */
#define BEIJINGTIME 8
#define DAY (60 * 60 * 24)
#define YEARFIRST 2001
#define YEARSTART (365 * (YEARFIRST - 1970) + 8)
#define YEAR400 (365 * 4 * 100 + (4 * (100 / 4 - 1) + 1))
#define YEAR100 (365 * 100 + (100 / 4 - 1))
#define YEAR004 (365 * 4 + 1)
#define YEAR001 365
u32 get_full_time(struct write_structure *w_stru)
{
    struct timeval tv;
    long sec = 0, usec = 0;
    int yy = 0, mm = 0, dd = 0, hh = 0, mi = 0, ss = 0, ms = 0;
    int ad = 0;
    int y400 = 0, y100 = 0, y004 = 0, y001 = 0;
    int m[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int i;

    DEBUG("get_full_time .\r\n");

    tv = w_stru->pretime;
    sec = tv.tv_sec;
    usec = tv.tv_usec;
    sec = sec + (60 * 60) * BEIJINGTIME;
    ad = sec / DAY;
    ad = ad - YEARSTART;
    y400 = ad / YEAR400;
    y100 = (ad - y400 * YEAR400) / YEAR100;
    y004 = (ad - y400 * YEAR400 - y100 * YEAR100) / YEAR004;
    y001 = (ad - y400 * YEAR400 - y100 * YEAR100 - y004 * YEAR004) / YEAR001;
    yy = y400 * 4 * 100 + y100 * 100 + y004 * 4 + y001 * 1 + YEARFIRST;
    dd = (ad - y400 * YEAR400 - y100 * YEAR100 - y004 * YEAR004) % YEAR001;
    //月 日
    if (0 == yy % 1000)
    {
        if (0 == (yy / 1000) % 4)
        {
            m[1] = 29;
        }
    }
    else
    {
        if (0 == yy % 4)
        {
            m[1] = 29;
        }
    }
    for (i = 1; i <= 12; i++)
    {
        if (dd - m[i] < 0)
        {
            break;
        }
        else
        {
            dd = dd - m[i];
        }
    }
    mm = i;
    //小时
    hh = sec / (60 * 60) % 24;
    //分
    mi = sec / 60 - sec / (60 * 60) * 60;
    //秒
    ss = sec - sec / 60 * 60;
    ms = usec;

    sprintf((char *)(w_stru->w_buf[w_stru->buf_num].buf),
            "%d-%02d-%02d %02d:%02d:%02d.%06d\n", yy, mm, dd, hh, mi, ss, ms);
    w_stru->w_buf[w_stru->buf_num].time_interval = 0.0;
    //DEBUG("get_full_time is %s \r\n",(char *)(w_stru->w_buf[w_stru->buf_num].buf));
    DEBUG("get_full_time sec %ld usec %ld \r\n", w_stru->pretime.tv_sec,
          w_stru->pretime.tv_usec);
    return SUCCESS;
}

/**
 * @brief Get the relatively time object
 * 
 * @param w_stru 
 * @return u32 
 */
u32 get_relatively_time(struct write_structure *w_stru, void *addr)
{
    struct timeval tv;
    struct timeval tv_pre;
    struct sockaddr_in *c_addr = (struct sockaddr_in *)addr;

    DEBUG("get_relatively_time .\r\n");
    tv_pre = w_stru->pretime; //旧的时间
    DEBUG(" tv_pre sec %ld usec %ld \r\n", tv_pre.tv_sec, tv_pre.tv_usec);
    memset(&tv, 0, sizeof(struct timeval));
    gettimeofday(&tv, NULL);
    w_stru->pretime = tv;
    DEBUG("get_relatively_time tv sec %ld usec %ld \r\n", tv.tv_sec, tv.tv_usec);
    float time_val = (tv.tv_sec - tv_pre.tv_sec) * 1000000 + tv.tv_usec - tv_pre.tv_usec;
    time_val = time_val / 1000.0;
    w_stru->w_buf[w_stru->buf_num].time_interval = time_val;
    //sprintf((char *)(w_stru->w_buf[w_stru->buf_num].buf), "%6.3f ms\n", time_val);
    sprintf((char *)(w_stru->w_buf[w_stru->buf_num].buf), "%s ", inet_ntoa(c_addr->sin_addr));
    return SUCCESS;
}

/**
 * @brief 校验lcm信息
 * 
 * @param data udp包
 * @param channel_name 通道名字
 * @return u32 
 */
u32 check_lcmm_channel(void *data, void *channel_name)
{
    u8 *data_p = NULL;
    u8 *name_p = NULL;
    s32 ret = 0;

    data_p = data;

    if ((MAGIC1 == *data_p) && (MAGIC2 == *(data_p + 1)) &&
        (MAGIC3 == *(data_p + 2)) && (MAGIC4 == *(data_p + 3)))
    {
        DEBUG("channel name %s magic success. %ld\r\n", channel_name,
              strlen((char *)channel_name));
        DEBUG("get channel name %s-- \r\n", (char *)(data_p + 7));

        name_p = data_p + 8;

        DEBUG("%s @@ %s \r\n", name_p, channel_name);
        ret = strncmp((char *)name_p, (char *)channel_name,
                      strlen((char *)channel_name));
        DEBUG("match ret %x \r\n", ret);
        if (SUCCESS == ret)
        {
            return SUCCESS;
        }
        else
        {
            DEBUG("channel name check error .\r\n");
            return ERROR;
        }
    }
    else
    {
        DEBUG("magic check error .\r\n");
        return ERROR;
    }
}

/**
 * @brief 以log的模式写入到文件中
 * 
 * @param w_stru 写ram的数据结构
 * @param channle 通道所属的配置
 */
void write_log_to_file(struct write_structure *w_stru, struct lcmm_channel *channle)
{
    FILE *fp;
    size_t len;
    char msg[DEFAULT_LEN] = {0};

    if (w_stru->w_buf[w_stru->buf_num].time_interval < 0.001)
        return;

    if (w_stru->buf_num >= channle->log_num)
    {
        fp = fopen((char *)w_stru->file_path_name, "a+");
        if (NULL == fp)
        {
            printf("Fail to open file!\n");
            return;
        }
        for (u32 loop = 0; loop <= w_stru->buf_num; loop++)
        {
            memset(msg, 0, sizeof(msg));
            sprintf(msg, "time %6.3f ms from %s \n", w_stru->w_buf[loop].time_interval, w_stru->w_buf[loop].buf);
            len = fwrite(msg, sizeof(u8),
                         strlen((char *)msg), fp);
            if (len != strlen((char *)msg))
            {
                printf("Fail to write\n");
                fclose(fp);
                return;
            }
        }

        //操作结束后关闭文件
        fclose(fp);
        DEBUG("<--- real write success !---->\n");
        w_stru->buf_num = 0;
        return;
    }
    else
    {
        w_stru->buf_num += 1;
    }
}

/**
 * @brief 以monitor的模式写入到文件中
 * 
 * @param w_stru 写ram的数据结构
 * @param channle 通道所属的配置
 */
void write_monitor_to_file(struct write_structure *w_stru, struct lcmm_channel *channle)
{
    FILE *fp;
    size_t len;
    char ip_msg[20] = {0};
    float abnormal_time_interval = 0;
    char msg[DEFAULT_LEN] = {0};

    if (w_stru->w_buf[w_stru->buf_num].time_interval < 0.001)
        return;

    w_stru->total_item++;
    //异常发生

    if ((w_stru->w_buf[w_stru->buf_num].time_interval > channle->high_limit) ||
        (w_stru->w_buf[w_stru->buf_num].time_interval < channle->low_limit))
    {
        w_stru->abnormal_item++;

        //记录本次错误的时间间隔
        abnormal_time_interval = w_stru->w_buf[w_stru->buf_num].time_interval;
        memcpy(ip_msg,(char *)(w_stru->w_buf[w_stru->buf_num].buf),strlen((char *)(w_stru->w_buf[w_stru->buf_num].buf)));
        get_full_time(w_stru); //转换绝对时间
        //去掉/n符
        strncpy(msg, (char *)(w_stru->w_buf[w_stru->buf_num].buf),
                strlen((char *)(w_stru->w_buf[w_stru->buf_num].buf)) - 1);
        sprintf(msg, "%s %s %6.3f ms from %s \n", msg, "<--->", abnormal_time_interval,ip_msg);
        DEBUG("msg %s \r\n", msg);

        //记录文件，尾部插入异常
        fp = fopen((char *)w_stru->file_path_name, "a+");
        DEBUG("filefp %p \r\n", fp);
        if (NULL == fp)
        {
            printf("Fail to open file!\n");
            return;
        }
        len = fwrite(msg, sizeof(u8), strlen(msg), fp);
        if (len != strlen(msg))
        {
            printf("Fail to write\n");
            fclose(fp);
            return;
        }
        fseek(fp, 0, SEEK_SET); //准备更新日志的头文件
        fclose(fp);
        fp = fopen((char *)w_stru->file_path_name, "r+");
        memset(msg, 0, sizeof(msg));

        //rewind(fp);   // 回到文件最开头
        sprintf(msg, "%s %20ld %s %10ld\n", "Monitor Number Total is ",
                w_stru->total_item, "Abnormal is ", w_stru->abnormal_item);
        DEBUG("msg %s filefp %p \r\n", msg, fp);

        len = fwrite(msg, sizeof(u8), strlen(msg), fp);
        if (len != strlen(msg))
        {
            printf("Fail to write\n");
            fclose(fp);
            return;
        }
        //操作结束后关闭文件
        fclose(fp);

        DEBUG("<--- real write success !---->\n");
        w_stru->buf_num = 0;
    }
    else //没有异常发生，节省io资源，隔指定的条数记录一次
    {
        if (w_stru->buf_num >= channle->log_num)
        {
            fp = fopen((char *)w_stru->file_path_name, "r+");
            if (NULL == fp)
            {
                printf("Fail to open file!\n");
                return;
            }

            fseek(fp, 0, SEEK_SET); //准备更新日志的头文件
            sprintf(msg, "%s %20ld %s %10ld\n", "Monitor Number Total is ",
                    w_stru->total_item, "Abnormal is ", w_stru->abnormal_item);
            len = fwrite(msg, sizeof(u8), strlen(msg), fp);
            if (len != strlen(msg))
            {
                printf("Fail to write\n");
                fclose(fp);
                return;
            }
            //操作结束后关闭文件
            fclose(fp);

            DEBUG("<--- real write success !---->\n");
            w_stru->buf_num = 0;
        }
        else
        {
            w_stru->buf_num++;
        }
    }
}

/**
 * @brief 记录消息，每1000条写一次
 * 
 * @param w_stru 
 */
void write_to_msg(struct write_structure *w_stru, struct lcmm_channel *channle, void *addr)
{
    struct timeval tv;
    FILE *fp = NULL;
    char msg[DEFAULT_LEN] = {0};
    char *p_total_item = NULL;
    char *p_abnormal_item = NULL;
    size_t len;

    DEBUG("write_to_msg .\r\n");
    if ((w_stru->pretime.tv_sec == 0) && (w_stru->pretime.tv_usec == 0))
    {
        //第一次获取完整的时间戳
        memset(&tv, 0, sizeof(struct timeval));
        gettimeofday(&tv, NULL);
        w_stru->pretime = tv;
        //第一次启动获取绝对时间
        //get_full_time(w_stru);
        fp = fopen((char *)w_stru->file_path_name, "r+");
        if (NULL == fp)
        {
            //表示没有该文件,以a+的方式重新打开，然后占位
            fp = fopen((char *)w_stru->file_path_name, "a+");
            sprintf(msg, "%s %20ld %s %10ld\n", "Monitor Number Total is ",
                    w_stru->total_item, "Abnormal is ", w_stru->abnormal_item);
            len = fwrite(msg, sizeof(u8), strlen(msg), fp);
            if (len != strlen(msg))
            {
                printf("Fail to write\n");
                fclose(fp);
                return;
            }
            //操作结束后关闭文件
            fclose(fp);
        }
        else
        {
            fgets(msg, sizeof(msg), fp);
            DEBUG("r+ open success get msg %s \r\n", msg);
            p_total_item = msg;
            p_abnormal_item = msg;
            p_total_item += sizeof("Monitor Number Total is ");
            w_stru->total_item = atol(p_total_item);
            p_abnormal_item += sizeof("Monitor Number Total is ") + 20 + sizeof("Abnormal is ");
            w_stru->abnormal_item = atol(p_abnormal_item);
            DEBUG("p_total_num %s total_item %ld p_abnormal_item %s abnormal_item %ld \r\n",
                  p_total_item, w_stru->total_item, p_abnormal_item, w_stru->abnormal_item);
            fclose(fp);
        }
        return;
    }
    else
    {
        get_relatively_time(w_stru, addr);
    }
    if (channle->worke_mode == LOG_MODE)
        write_log_to_file(w_stru, channle);
    if (channle->worke_mode == MONITOR_MODE)
        write_monitor_to_file(w_stru, channle);
}
