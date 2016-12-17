﻿# zigbee-zstack

## 目录

```javascript

.
├── Projects/zstack          		          # 用户自定义
│     ├── Samples/ZigbeeZstack/Source	    # 源代码       
│     │   └── App				              # Zstack协议(应用层)
│     │      ├── Lock			           # 门锁应用	
│     |      |   ├── Drivers                # 门锁硬件驱动
│     |      |   └── Devices                # 门锁设备应用
│     │      └── Base			           # 基站应用 
│     ├── Tools				                # 配置文件
│     └── ZMain				                # 启动程序   
└── Components				              # Ti公司定义
      ├── hal             			      # 硬件驱动
      ├── mac          			          # MAC层
      ├── mt          			          # 串口通信
      ├── osal           			          # 操作系统
      ├── services            		      # 地址处理
      ├── stack               		      # Zstack协议(网络层、AF射频、ZDO等)
      └── zmac				              # MAC层
```


## 硬件配置


| 端口或定时器          |     配置说明 | 
| :--------     | :--------|
| P1_5          |  LED灯引脚，配置为输出(修改了Z-Stack中的LED配置) |
| P0_1          |  钥匙开门引脚，配置为输入，上升沿触发中断(和Z-Stack独立按键引脚一样) |
| P2_0          |  蜂鸣器引脚，配置为输入(修改了Z-Stack的配置，在Z-Stack中是摇杆按键引脚) |
| T4定时器       |  T4用作PWM输出，用于控制蜂鸣器的发音频率(覆盖了默认的Z-Stack定时器配置) |

## 门锁硬件驱动

| 驱动文件       |     配置说明 | 
| :--------     | :--------|
| dri_buzzer.c  |  蜂鸣器引脚配置，P2_O引脚配置为输入，启用T4定时器设置为PWM输出，输出到引脚P2_0 |
| dri_delay.c   |  延时函数，延时ms和us |

>提示：硬件驱动程序主要在Projects/zstack/Samples/ZigbeeZstack/Source/App/Lock/Drivers文件夹下。


## 门锁设备应用

| 应用文件       |     应用说明 | 
| :--------     | :--------|
| dev_buzzer.c  | 蜂鸣器的应用程序，主要用于刷卡提示，系统启动提示以及钥匙开关门提示等声音提示 |

>提示：硬件驱动程序主要在Projects/zstack/Samples/ZigbeeZstack/Source/App/Lock/Devices文件夹下。

## 进度记录

| 日期      |     进度 |
| :-------- | :--------|
| 2016/12/13    |  Z-Stack工程的建立，加入了基站和门锁的应用层程序。 |
| 2016/12/14    |  门锁低功耗模式，设计为PM2功耗模式,采用休眠-唤醒-休眠的工作模式。每1s向基站发送data request，获取基站的命令数据。 |
| 2016/12/15    |  设置门锁离线定时程序，每隔1s启动离线程序，修改了LED底层驱动，目前只是实现了LED 1s闪烁，后期需要加入RFID读卡程序。 |
| 2016/12/16    |  修改底层按键硬件驱动，测试了按键的轮询扫描模式和中断触发模式成功，最后使用中断模式，轮询扫描模式每隔100ms扫描按键有无按下，效率低反应慢。 |
