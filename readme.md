# lcm monitor工具

主要用来监控lcm的到达时间，并记录相应的日志

```
lcmm_v1.0 
         
 Usage   :         
    lcmm [options]         
    options        
        -h,--help                get app help        
        -l,--log          <stored file path>         
            <stored file path>   (default /home/ab64/ab_data/usp/lcmm/)         
        -d,--device       <network_card>        
            <network_card>       the network card name eg:eth0..(defatl any)        
        -c,--channel              channel name         
            -c {[channel_name1]:[name2]:..}        
        -g,--group               group ip address (default 239.255.76.67)        
        -p,--port                monitor port (default 7667)        
        -C,--config              set by config file        
            No parameters will load the default configuration file and write it to log path        
            The absolute path to the configuration file        
                    
        Notice :The -c,--channel and -C,--config can only choose one        
               --channel  Each interval of the specified channel will be recorded        
                          log file will at log path Named by channel        
               --config   Will log by json config file 
```

支持 通过通道名或者json的配置文件进行配置

json的配置文件在src中有个默认的设置，如果选择LOG_MODE，则与-c 的作用是一致的

MONITOR_MODE会产生如下效果的日志
```
Monitor Number Total is                 58821 Abnormal is           7
2020-08-27 09:19:54.360674 <---> 65.469 ms from 192.168.5.53  
2020-08-27 09:20:21.928085 <---> 66.660 ms from 192.168.5.53
```