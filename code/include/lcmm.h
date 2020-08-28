/**
 * @file lcmm.h
 * @author zhao.wei (hw)
 * @brief lcmm头文件
 * @version 0.1
 * @date 2020-07-17
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef _LCMM_H_
#define _LCMM_H_

#define smm_deal(cat, dealmode, format, args...)                                        \
    zlog(cat, __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, \
         dealmode, format, ##args)

#define DISPLAY_MODE 0 //使用打印模式
#define LOG_MODE 1     //使用zlog的info模式

#define CHANLE_NAME_LEN 32 //通道名字最大长度
#define CHANLE_SIZE 128    //通道个数

#define DEFAULT_LEN 128
#define ETH_ALL 0xff
#define DEFAULT_PORT 7667
#define DEFAULT_GROUP "239.255.76.67"
#define DEFAULT_FILEPATH "/home/ab64/ab_data/usp/lcmm/"

#define DEFAULT_LOG_NUMBER 1000 //汇总多少条后写入

#define MAGIC1 0x4c
#define MAGIC2 0x43
#define MAGIC3 0x30
#define MAGIC4 0x32

#define LOG_MODE 1     //逐条记录
#define MONITOR_MODE 2 //异常记录
/**
 * @brief lcm的通道名字
 * 
 */
struct lcmm_channel
{
    u8 name[CHANLE_NAME_LEN];
    u32 worke_mode;
    u32 low_limit;
    u32 high_limit;
    u32 log_num;
};

/**
 * @brief lcm的控制结构
 * 
 */
struct lcmm_ctrl
{
    u8 w_mode;
    u8 eth_index;
    u8 channel_num;
    u8 pad;
    u32 m_port;
    struct lcmm_channel channel[CHANLE_SIZE];
    u8 file_path[DEFAULT_LEN];
    u8 group_ip[DEFAULT_LEN];
};

struct write_buf
{
    u8 buf[CHANLE_NAME_LEN];
    float time_interval;
};

struct write_structure
{
    u8 file_path_name[DEFAULT_LEN];
    struct timeval pretime; //上一次的时间戳
    struct write_buf w_buf[1024];
    u32 buf_num;
    u64 total_item;
    u64 abnormal_item;
};

u32 lcm_write(void *file_path, void *data);
u32 get_full_time(struct write_structure *w_stru);
u32 check_lcmm_channel(void *data, void *channel_name);
void write_to_msg(struct write_structure *w_stru, struct lcmm_channel *channle, void *addr);

#endif