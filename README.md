﻿# zigbee-zstack

## 目录

```javascript

.
├── Projects/zstack          		            # 用户自定义
│     ├── Samples/ZigbeeZstack/Source	    # 源代码       
│     │   └── App				                        # Zstack协议(应用层)
│     │        ├── Base			                  # 基站应用	
│     │        └── Lock			                  # 门锁应用 
│     ├── Tools				                          # 配置文件
│     └── ZMain				                          # 启动程序   
└── Components				                           # Ti公司定义
      ├── hal             			               # 硬件驱动
      ├── mac          			                  # MAC层
      ├── mt          			                   # 串口通信
      ├── osal           			                # 操作系统
      ├── services            		           # 地址处理
      ├── stack               		           # Zstack协议(网络层、AF射频、ZDO等)
      └── zmac				                           # MAC层
```