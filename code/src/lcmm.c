#include "pub.h"
#include "default_config"

struct lcmm_ctrl g_ctrl = {0};
u8 *global_write_structure = NULL;

static u8 help[] =
    "\
        \r\n Usage   : \
        \r\n    lcmm [options] \
        \r\n    options\
        \r\n        -h,--help                get app help\
        \r\n        -l,--log          <stored file path> \
        \r\n            <stored file path>   (default /home/ab64/ab_data/usp/lcmm/) \
        \r\n        -d,--device       <network_card>\
        \r\n            <network_card>       the network card name eg:eth0..(defatl any)\
        \r\n        -c,--channel              channel name \
        \r\n            -c {[channel_name1]:[name2]:..}\
        \r\n        -g,--group               group ip address (default 239.255.76.67)\
        \r\n        -p,--port                monitor port (default 7667)\
        \r\n        -C,--config              set by config file\
        \r\n            No parameters will load the default configuration file and write it to log path\
        \r\n            The absolute path to the configuration file\
        \r\n            \
        \r\n        Notice :The -c,--channel and -C,--config can only choose one\
        \r\n               --channel  Each interval of the specified channel will be recorded\
        \r\n                          log file will at log path Named by channel\
        \r\n               --config   Will log by json config file\
        ";

static u8 exit_prese_msg[] =
    "\
    \r\n Usage   : \
    \r\n    lcmm [options] -D <network_card> -c {[channel_name1]:[name2]:..}\
    \r\nTry `lcmm -h,--help' for more information.\
    ";

static struct option long_options[] =
    {
        {"help", no_argument, NULL, 'h'},
        {"log", required_argument, NULL, 'l'},
        {"device", optional_argument, NULL, 'd'},
        {"channel", required_argument, NULL, 'c'},
        {"group", required_argument, NULL, 'g'},
        {"port", required_argument, NULL, 'p'},
        {"config", required_argument, NULL, 'C'},
        {NULL, 0, NULL, 0},
};

/**
 * @brief lcmm 从传入的参数解析通道名字
 * 
 * @param contrl lcmm contrl数据结构
 * @param optarg 传入的参数
 * @return u32 0 is success
 */
u32 lcmm_parse_channel_name(struct lcmm_ctrl *contrl, u8 *arg)
{
    u32 ret = SUCCESS;
    u8 channel_num = 0;
    u8 *data = NULL;
    u8 i = 0;

    DEBUG("arg is %s \r\n", arg);
    for (data = arg; *data != '\0'; data++)
    {
        if (':' == *data)
            channel_num++; //初步统计一下进程数
    }

    DEBUG("channel_num is %d \r\n", channel_num);
    data = arg;
    DEBUG("data is %s \r\n", data);
    contrl->channel_num = channel_num + 1; //从输入参数解析而来的进程数

    for (i = 0; i < channel_num; i++)
    {
        sscanf(data, "%[^:]%*c", contrl->channel[i].name);
        data += strlen(contrl->channel[i].name) + 1;
        DEBUG("pid_msg.pid_name %s \r\n ", contrl->channel[i].name);
        contrl->channel[i].log_num = DEFAULT_LOG_NUMBER;
        contrl->channel[i].worke_mode = LOG_MODE; //这种方式全部采用log mode进行记录
    }
    DEBUG("i=%d malloc size %ld\r\n ", i, (sizeof(struct write_structure) * i));

    global_write_structure = (void *)malloc(sizeof(struct write_structure) * contrl->channel_num);
    memset(global_write_structure, 0, sizeof(sizeof(struct write_structure) * contrl->channel_num));
    DEBUG("(%p)write_p \r\n", global_write_structure);
    return ret;
}

/**
 * @brief 解析json文件
 * 
 * @param contrl 
 * @param data 
 * @return u32 
 */
u32 pares_json(struct lcmm_ctrl *contrl, char *data)
{
    cJSON *root = NULL;
    cJSON *filter = NULL;
    cJSON *sub_obj = NULL;
    cJSON *channle_name = NULL;
    cJSON *work_mode = NULL;
    cJSON *low_limit = NULL;
    cJSON *high_limit = NULL;
    cJSON *log_num = NULL;
    u32 size = 0, i = 0;

    DEBUG("pares_json in \r\n");
    root = cJSON_Parse(data);
    DEBUG("pares_json root success %p \r\n", root);
    if (NULL == root)
    {
        printf("config format error please check .\r\n");
        exit(0);
    }
    else
    {
        DEBUG("pares_json root success %p \r\n", root);
        filter = cJSON_GetObjectItem(root, "channle");
        if (NULL == filter)
        {
            printf("config format error please check .\r\n");
            exit(0);
        }
        size = cJSON_GetArraySize(filter);
        DEBUG("channle size %d \r\n", size);
        contrl->channel_num = size;
        if (size > CHANLE_SIZE)
        {
            printf("monitor size is overflow .\r\n");
            exit(0);
        }
        for (i = 0; i < size; i++)
        {
            sub_obj = cJSON_GetArrayItem(filter, i);
            if (NULL == sub_obj)
            {
                printf("config format error please check .\r\n");
                exit(0);
            }
            channle_name = cJSON_GetObjectItem(sub_obj, "channle_name");
            if (channle_name->valuestring == NULL)
            {
                printf("config format error please check .\r\n");
                exit(0);
            }
            memcpy(contrl->channel[i].name, channle_name->valuestring, strlen(channle_name->valuestring));

            work_mode = cJSON_GetObjectItem(sub_obj, "work_mode");
            if (work_mode->valuestring == NULL)
            {
                printf("config format error please check .\r\n");
                exit(0);
            }
            if (SUCCESS == strcmp(work_mode->valuestring, "LOG_MODE"))
                contrl->channel[i].worke_mode = LOG_MODE;
            if (SUCCESS == strcmp(work_mode->valuestring, "MONITOR_MODE"))
                contrl->channel[i].worke_mode = MONITOR_MODE;

            low_limit = cJSON_GetObjectItem(sub_obj, "low_limit");
            if (NULL == low_limit)
            {
                printf("config format error please check .\r\n");
                exit(0);
            }
            contrl->channel[i].low_limit = low_limit->valueint;

            high_limit = cJSON_GetObjectItem(sub_obj, "high_limit");
            if (NULL == high_limit)
            {
                printf("config format error please check .\r\n");
                exit(0);
            }
            contrl->channel[i].high_limit = high_limit->valueint;

            log_num = cJSON_GetObjectItem(sub_obj, "log_num");
            if (NULL == log_num)
            {
                printf("config format error please check .\r\n");
                exit(0);
            }
            contrl->channel[i].log_num = log_num->valueint;

            DEBUG("channlename %s low_limit %d high_limit %d num %d \r\n", contrl->channel[i].name,
                  contrl->channel[i].low_limit, contrl->channel[i].high_limit, contrl->channel[i].log_num);
        }
    }
    return SUCCESS;
}

/**
 * @brief 解析json的config文件
 * 
 * @param contrl 
 * @param arg 
 * @return u32 
 */
u32 lcmm_parse_config(struct lcmm_ctrl *contrl, u8 *arg)
{
    u32 ret = SUCCESS;
    char config_path[DEFAULT_LEN] = {0};
    FILE *fp;
    size_t len;
    char *data;

    DEBUG("lcmm_parse_config in \r\n");
    if (NULL == arg)
    {
        pares_json(contrl, default_config);

        cJSON *json;
        json = cJSON_Parse(default_config);
        if (!json)
        {
            printf("Error before: [%s]\n", cJSON_GetErrorPtr());
        }
        else
        {
            data = cJSON_Print(json);
            cJSON_Delete(json);
            //printf("%s\n", out);
            strcpy(config_path, (char *)g_ctrl.file_path);
            strcat(config_path, "/");
            strcat(config_path, "lcmm_config");
            fp = fopen(config_path, "w+");
            if (NULL == fp)
            {
                printf("Fail to open file!\n");
                return ERROR;
            }

            len = fwrite(data, sizeof(u8), strlen(data), fp);
            if (len != strlen(data))
            {
                printf("Fail to write\n");
                fclose(fp);
                return ERROR;
            }

            //操作结束后关闭文件
            fclose(fp);
            free(data);
        }
    }
    else
    {
        memcpy(config_path, arg, strlen(arg));
        fp = fopen(config_path, "rb");
        if (NULL == fp)
        {
            printf("Fail to open config file!\n");
            exit(0);
        }
        fseek(fp, 0, SEEK_END);
        len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = (char *)malloc(len + 1);
        fread(data, 1, len, fp);
        fclose(fp);
        pares_json(contrl, data);
        free(data);
    }
    global_write_structure = (void *)malloc(sizeof(struct write_structure) * contrl->channel_num);
    memset(global_write_structure, 0, sizeof(sizeof(struct write_structure) * contrl->channel_num));
    DEBUG("(%p)write_p \r\n", global_write_structure);
    //执行默认操作
    return ret;
}
/**
 * 
 * @brief 打印帮助信息
 * 
 */
void printf_help_usage()
{
    printf("lcmm_v%d.%d \r\n %s \r\n", MAJOR_VER, MINOR_VER, help);
    exit(1);
}

/**
 * @brief 异常退出消息提示
 * 
 */
void exit_usage()
{
    printf("lcmm_v%d.%d \r\n %s \r\n", MAJOR_VER, MINOR_VER, exit_prese_msg);
    //日志系统资源释放
    exit(1);
}

/**
 * @brief lcmm的命令参数解析
 * 
 * @param opt 
 * @param optarg 
 * @param argv 
 */
static void lcmm_cmd_parse(u32 opt, u8 *optarg, u8 *argv)
{
    static u8 choice_flag = 0;
    if (opt == 'd')
    {
        DEBUG("device is %s \r\n", (char *)optarg);
        g_ctrl.eth_index = if_nametoindex("eth0");
    }

    if (opt == 'c')
    {
        if (choice_flag == 2)
            exit_usage();

        lcmm_parse_channel_name(&g_ctrl, optarg);

        choice_flag = 1;
    }
    if (opt == 'C')
    {
        if (choice_flag == 1)
            exit_usage();

        lcmm_parse_config(&g_ctrl, argv);
        choice_flag = 2;
    }
    if (opt == 'l')
    {
        DEBUG("log path is %s \r\n", (char *)optarg);
        memset(g_ctrl.file_path, 0, sizeof(g_ctrl.file_path));
        memcpy(g_ctrl.file_path, optarg, strlen((char *)optarg));
    }
    if (opt == 'g')
    {
        DEBUG("group ip is %s \r\n", (char *)optarg);
        memcpy(g_ctrl.group_ip, optarg, strlen((char *)optarg));
        DEBUG("group ip is %s \r\n", g_ctrl.group_ip);
    }
    if (opt == 'p')
    {
        DEBUG("port is %d \r\n", atoi((char *)optarg));
        g_ctrl.m_port = atoi((char *)optarg);
    }
}

u8 recv_buf[1024] = {0};

/**
 * @brief udp初始化
 * 
 * @return u32 fd
 */
void udp_init()
{
    s32 sockfd;
    s32 ret;
    u32 len;
    u8 loop = 0;
    struct sockaddr_in client = {0};
    u32 client_address_size = (sizeof(client));
    struct sockaddr_in serveraddr;
    struct ip_mreqn ngroup = {0};
    struct ip_mreq group = {0};
    /* 构造用于UDP通信的套接字 */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        printf("creat socket fail\n");
        exit(0);
        return;
    }
    if (ETH_ALL == g_ctrl.eth_index)
    {
        /*设置组地址*/
        /*本地任意IP*/
        group.imr_multiaddr.s_addr = inet_addr((char *)(g_ctrl.group_ip));
        group.imr_interface.s_addr = inet_addr("0.0.0.0");
        ret = setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(group));
        if (ret < 0)
        {
            printf("setsockopt failure %x ,please Wait a moment try again .\r\n", ret);
            exit(0);
            return;
        }
        DEBUG("setsockopt success \r\n");
    }
    else
    {
        /*设置组地址*/
        inet_pton(AF_INET, (char *)g_ctrl.group_ip, &ngroup.imr_multiaddr);
        /*本地任意IP*/
        inet_pton(AF_INET, "0.0.0.0", &ngroup.imr_address);
        /* eth0 --> 编号 命令：ip ad */
        ngroup.imr_ifindex = g_ctrl.eth_index;
        setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &ngroup, sizeof(group));
    }

    int opt = 1;
    // sockfd为需要端口复用的套接字
    ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
    if (ret < 0)
    {
        printf("setsockopt reuse failure %x  .\r\n", ret);
        exit(0);
        return;
    }
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;                /* IPv4 */
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); /* 本地任意IP INADDR_ANY = 0 */
    serveraddr.sin_port = htons(g_ctrl.m_port);
    ret = bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (ret < 0)
    {
        printf("bind failure %x ,please Wait a moment try again .\r\n", ret);
        exit(0);
        return;
    }

    DEBUG("bind success g_ctrl.eth_index %x \r\n", g_ctrl.eth_index);

    struct write_structure *write_p = NULL;

    write_p = (struct write_structure *)global_write_structure;

    for (loop = 0; loop < g_ctrl.channel_num; loop++, write_p++)
    {
        strcpy((char *)write_p->file_path_name, (char *)g_ctrl.file_path);
        strcat((char *)write_p->file_path_name, "/");
        strcat((char *)write_p->file_path_name, (char *)g_ctrl.channel[loop].name);
        DEBUG("(%p)write_p->file_path_name %s \r\n", write_p, write_p->file_path_name);
    }

    while (1)
    {
        DEBUG("will recv \r\n");
        write_p = (struct write_structure *)global_write_structure;
        len = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&client, &client_address_size);
        DEBUG("recv something len %d \r\n", len);

        for (loop = 0; loop < g_ctrl.channel_num; loop++, write_p++)
        {
            ret = check_lcmm_channel(recv_buf, g_ctrl.channel[loop].name);
            if (SUCCESS == ret)
            {
                write_to_msg(write_p, &g_ctrl.channel[loop],&client);
            }
            DEBUG("write success \r\n");
        }
        DEBUG("\r\n");
    }
    return;
}

/**
 * @brief 主函数
 * 
 * @param argc 
 * @param argv 
 * @return u32 
 */
int main(int argc, char *argv[])
{
    u32 opt;
    //u32 ret;
    u32 option_index = 0;
    u8 *string = "hd:c:l:g:p:C::";
    //pthread_t thread_id = 0;

    //默认参数
    g_ctrl.w_mode = LOG_MODE;
    g_ctrl.eth_index = ETH_ALL;
    g_ctrl.m_port = DEFAULT_PORT;
    memcpy(g_ctrl.group_ip, DEFAULT_GROUP, strlen(DEFAULT_GROUP));
    memcpy(g_ctrl.file_path, DEFAULT_FILEPATH, strlen(DEFAULT_FILEPATH));

    while ((opt = getopt_long_only(argc, argv, string, long_options, &option_index)) != -1)
    {
        //printf("opt = %c\t\t", opt);
        //printf("optarg = %s\t\t", optarg);
        //printf("optind = %d\t\t", optind);
        //printf("argv[optind] =%s\t\t", argv[optind]);
        //printf("option_index = %d\n", option_index);
        switch (opt)
        {
        case 'l':
        case 'd':
        case 'c':
        case 'g':
        case 'p':
        case 'C':
            lcmm_cmd_parse(opt, optarg, argv[optind]);
            break;
        case 'h':
            printf_help_usage();
            break;
        default:
            exit_usage(0);
            break;
        }
    }

    udp_init();

    while (1)
    {
        sleep(1);
    }
    return 0;
}