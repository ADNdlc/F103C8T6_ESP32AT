# esp32-MQTT

> 官方文档
> [硬件连接ESP-AT 用户指南 文档](https://espressif-docs.readthedocs-hosted.com/projects/esp-at/zh-cn/release-v2.2.0.0_esp8266/Get_Started/Hardware_connection.html)

> [!NOTE]
>
> 以下使用ESP32-WROOM-32模组ESP32-DEVKITV1开发板进行，不同模组和开发板引脚和固件可能不同，详情见官方文档，以下是DEV开发板引脚定义：

![ESP32_Dev引脚定义](C:\Users\12114\Desktop\Arduino开发\ESP32_Dev引脚定义.png)

***

## AT固件烧录

> 烧录工具固件下载地址
> [发布的固件 - ESP32 - — ESP-AT 用户指南 latest 文档](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Binary_Lists/esp_at_binaries.html)
> [Flash 下载工具用户指南 - ESP32 - — ESP 测试工具 latest 文档](https://docs.espressif.com/projects/esp-test-tools/zh_CN/latest/esp32/production_stage/tools/flash_download_tool.html)

> [!IMPORTANT]
>
> 注意！要烧录`下载的固件/factory` 目录下的 `factory_XXX.bin`至 `0x0` 地址：勾选 “DoNotChgBin”

![image-20250517174332912](../../../AppData/Roaming/Typora/typora-user-images/image-20250517174332912.png)

烧录完成后，连接串口2(RX2,TX2)到电脑，发送 ==AT+GMR+回车==，若返回以下信息代表烧录成功：

>AT version:3.4.0.0(s-c31b833 - ESP32 - Jun  7 2024 03:48:17)
>SDK version:v5.0.6-dirty
>compile time(70ff5889):Jun  7 2024 04:46:00
>Bin version:v3.4.0.0(WROOM-32)
>
>OK


---



## 基本功能



### [开启或关闭 AT 回显功能](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/Basic_AT_Commands.html#ate-at)

```c
ATE0
ATE1
//响应：
OK
```

- **ATE0**：关闭回显
- **ATE1**：开启回显

---



## WiFi连接

> 官方AT命令集
> [ESP-AT命令集 — 总目录](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/index.html)
> [Wi-Fi AT 命令集 - ESP32 — ESP-AT 用户指南 latest 文档](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/Wi-Fi_AT_Commands.html)

> [!NOTE]
>
> 要保证WiFi模块供电稳定，建议使用充电器供电

### 查看模工作模式

```c
AT+CWMODE?
//响应：
+CWMODE:<mode>
    
OK
```

<mode>：模式
0: 无 Wi-Fi 模式，并且关闭 Wi-Fi RF
1: Station 模式
2: SoftAP 模式
3: SoftAP+Station 模式

---

## 获取时间戳



**连接上WiFi后**

[AT+CIPSNTPCFG](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/TCP-IP_AT_Commands.html#cmd-sntpcfg) **：查询/设置时区和 SNTP 服务器**

```c
AT+CIPSNTPCFG=<enable>,<timezone>,<"SNTP server1">,<"SNTP server2">,<"SNTP server3">
AT+CIPSNTPCFG=1,8,"cn.ntp.org.cn"
//响应：
OK
//间隔一会
+TIME_UPDATED
```

[AT+CIPSNTPTIME](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/TCP-IP_AT_Commands.html#cmd-sntpt) **：查询 SNTP 时间**

```c
AT+CIPSNTPTIME?
//响应：
+CIPSNTPTIME:Fri May 30 18:06:18 2025
OK
```



—

## [设置 Wi-Fi 模式](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/Wi-Fi_AT_Commands.html#at-cwmode-wi-fi-station-softap-station-softap)

```c
AT+CWMODE=<mode>,<auto_connect>
			   //auto_connect:Station或混合模式自动连接已保存的AP
AT+CWMODE=3,1
//响应：
OK
```



---

### 连接网络

#### 扫描WiFi:

```c
AT+CWLAP
//响应：
+CWLAP:(3,"test2",-24,"ea:54:08:89:21:20",1,-1,-1,4,4,7,1)
......
```

#### 连接WiFi：

```c
AT+CWJAP=<"ssid">,<"pwd">,<"bssid">.......(详细参数见文档)
AT+CWJAP="test2","12345678"
//响应：
WIFI CONNECTED
WIFI GOT IP

OK
```

#### 查询当前WiFi：

```c
AT+CWJAP?
//响应：
busy p...
+CWJAP:"test2","ea:54:08:89:21:20",1,-27,0,1,3,0,1

OK 
```

#### 断开当前WiFi：

```c
AT+CWQAP
//响应
busy p...
WIFI DISCONNECT

OK
```

#### 重连上次的WiFi：

```c
AT+CWJAP
```

默认上电重连WiFi



***

## OneNet云平台连接

> 官方MQTT-AT命令集
> [MQTT AT 命令集 - ESP32 — ESP-AT 用户指南 latest 文档](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#)



### [设置 MQTT 用户属性](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttusercfg-mqtt)

```c
AT+MQTTUSERCFG=<LinkID>,<scheme>,<"client_id">,<"username">,<"password">,<cert_key_ID>,<CA_ID>,<"path">

AT+MQTTUSERCFG=0,1,"temperatureAndHumidity","SQKg9n0Ii0","",0,0,""
//						<设备名称/ID>		<产品ID>
```

- **<LinkID>**：当前仅支持 link ID 0。
- **<”client_id”>**：MQTT 客户端 ID，最大长度：256 字节。
- **<”username”>**：用户名，用于登陆 MQTT broker，最大长度：64 字节。
- **<”password”>**：密码，用于登陆 MQTT broker，最大长度：64 字节。
- **<scheme>**：**==这里使用TCP连接==**
	- 1: MQTT over TCP；
	- 2: MQTT over TLS（不校验证书）；
	- 3: MQTT over TLS（校验 server 证书）；
	- 4: MQTT over TLS（提供 client 证书）；
	- 5: MQTT over TLS（校验 server 证书并且提供 client 证书）；
	- 6: MQTT over WebSocket（基于 TCP）；
	- 7: MQTT over WebSocket Secure（基于 TLS，不校验证书）；
	- 8: MQTT over WebSocket Secure（基于 TLS，校验 server 证书）；
	- 9: MQTT over WebSocket Secure（基于 TLS，提供 client 证书）；
	- 10: MQTT over WebSocket Secure（基于 TLS，校验 server 证书并且提供 client 证书）

**由于OneNet平台的密码非常长，需使用MQTT LONG PASSWORD来设置密码**

> **密码使用工具生成**
> [token生成工具_开发者文档_OneNET](https://open.iot.10086.cn/doc/mqtt/book/manual/auth/tool.html)（[点击下载](https://open.iot.10086.cn/doc/mqtt/images/tools/token.exe)）
>
> - res:	oducts/产品id/devices/设备名称/id
> - et:	  当前时间戳(要改大一点，使用未来的时间戳)
> - key:	“设备密钥”
> - 方法:      md5



---

### [设置 MQTT 客户端 ID](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttlongclientid-mqtt-id)

```c
AT+MQTTLONGCLIENTID=<LinkID>,<length>

//响应
OK

>

```

上述响应表示 AT 已准备好接收 MQTT 客户端 ID，此时您可以输入客户端 ID，当 AT 接收到的客户端 ID 长度达到 `<length>` 后，返回：

```c
OK
```

- **<LinkID>**：当前仅支持 link ID 0。

- **<length>**：MQTT 客户端 ID 长度。范围：[1,1024]。

	

---

### [设置 MQTT 登陆用户名](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttlongusername-mqtt)

```c
AT+MQTTLONGUSERNAME=<LinkID>,<length>

//响应
OK

>
```

上述响应表示 AT 已准备好接收 MQTT 用户名，此时您可以输入 MQTT 用户名，当 AT 接收到的 MQTT 用户名长度达到 `<length>` 后，返回：

```
OK
```

- **<LinkID>**：当前仅支持 link ID 0。

- **<length>**：MQTT 用户名长度。范围：[1,1024]。

	

---

### [设置 MQTT 登陆密码](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttlongpassword-mqtt)

> <length>长度计算
> [在线字符串长度计算工具](https://www.lddgo.net/string/stringlength)
>
> [**OneNET - Token算法**](https://open.iot.10086.cn/doc/v5/fuse/detail/1486)

```c
AT+MQTTLONGPASSWORD=<LinkID>,<length>

AT+MQTTLONGPASSWORD=0,140
//响应
OK

>
//输入：
//version=2018-10-31&res=products%2FSQKg9n0Ii0%2Fdevices%2FtemperatureAndHumidity&et=1757458587&method=md5&sign=YCozJxz%2BPX0Qf1coXSUd0A%3D%3D
//响应：
busy p...

OK
```

上述响应表示 AT 已准备好接收 MQTT 密码，此时您可以输入 MQTT 密码，当 AT 接收到的 MQTT 密码长度达到 `<length>` 后，返回：

```c
busy p...

OK
```

- **<LinkID>**：当前仅支持 link ID 0。

- **<length>**：MQTT 密码长度。范围：[1,1024]

	

---

## [连接 MQTT Broker](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttconn-mqtt-broker)

> [OneNet服务地址](https://open.iot.10086.cn/doc/mqtt/book/device-develop/manual.html)
>
> [OneNET - 通信主题列表](https://open.iot.10086.cn/doc/v5/fuse/detail/920)


```c
AT+MQTTCONN=<LinkID>,<"host">,<port>,<reconnect>
AT+MQTTCONN=0,"mqtts.heclouds.com",1883,0
//响应：
    
OK
//打开云平台可见设备变为在线
```

- **<”host”>**：MQTT broker 域名，最大长度：128 字节。
- **<port>**：MQTT broker 端口，最大端口：65535。
- **<reconnect>**：
	- 0: MQTT 不自动重连。如果 MQTT 建立连接后又断开，则无法再次使用本命令重新建立连接，您需要先发送 [AT+MQTTCLEAN=0](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#cmd-mqttclean) 命令清理信息，重新配置参数，再建立新的连接。
	- 1: MQTT 自动重连，会消耗较多的内存资源。



---

#### 查询 ESP32 设备已连接的 MQTT broker：

```c
AT+MQTTCONN?
//响应
+MQTTCONN:<LinkID>,<state>,<scheme>,<"host">,<port>,<"path">,<reconnect>
OK
    
+MQTTCONN:0,4,1,"mqtts.heclouds.com","1883","",0

OK
```

- **<state>**：MQTT 状态：

	- 0: MQTT 未初始化；

	- 1: 已设置 [AT+MQTTUSERCFG](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#cmd-mqttusercfg)；

	- 2: 已设置 [AT+MQTTCONNCFG](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#cmd-mqttconncfg)；

	- 3: 连接已断开；

	- 4: 已建立连接；

	- 5: 已连接，但未订阅 topic；

	- 6: 已连接，已订阅过 topic。

		

---

### [订阅 MQTT Topic](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttsub-mqtt-topic)

> [OneNET - 通信主题](https://open.iot.10086.cn/doc/v5/fuse/detail/920)



订阅指定 MQTT topic 的指定 QoS，支持订阅多个 topic（最多支持订阅 10 个 topic）

```c
AT+MQTTSUB=<LinkID>,<"topic">,<qos>
//这里订阅设备属性上报响应
AT+MQTTSUB=0,"$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/post/reply",0
//响应：
    
OK
```

#### 查询已订阅的 topic

```c
AT+MQTTSUB?
//响应：
+MQTTSUB:<LinkID>,<state>,<"topic1">,<qos>
+MQTTSUB:<LinkID>,<state>,<"topic2">,<qos>
...
OK

+MQTTSUB:0,6,"$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/post/reply",0

OK
```



---

### [发布长 MQTT 消息](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttpubraw-mqtt)

> [OneNET - 设备属性上报](https://open.iot.10086.cn/doc/v5/fuse/detail/902)

通过 topic 发布长 MQTT 消息。如果您发布消息的数据量相对较少，不大于单条 AT 命令的长度阈值 `256` 字节，也可以使用 [AT+MQTTPUB](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#cmd-mqttpub) 命令。

```c
AT+MQTTPUBRAW=<LinkID>,<"topic">,<length>,<qos>,<retain>
//响应：
OK
>
```

符号 `>` 表示 AT 准备好接收串口数据，此时您可以输入数据，当数据长度达到参数 `<length>` 的值时，数据传输开始。

```c
//若传输成功，则 AT 返回：
+MQTTPUB:OK
```

```c
//若传输失败，则 AT 返回：
+MQTTPUB:FAIL
```

示例：

```c
AT+MQTTPUBRAW=0,"$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/post",146,0,0
//响应：
OK

>
{"id":"123","version":"1.0","params":{"currentTemperature":{"value":22,"time":1747458287111},"currenthumidity":{"value":33,"time":1747458287111}}}
//响应：
busy p...
+MQTTPUB:OK
    
//若订阅了该模块的消息网站会回复：
+MQTTSUBRECV:0,"$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/post/reply",39,{"id":"123","code":200,"msg":"success"}

//打开网站查看设备详情里的属性栏，可见数据点值刷新，推送成功
```



---

### [取消订阅 MQTT Topic](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttunsub-mqtt-topic)

客户端取消订阅指定 topic，可多次调用本命令，以取消订阅不同的 topic。

```c
AT+MQTTUNSUB=<LinkID>,<"topic">
//响应
OK

AT+MQTTUNSUB=0,"$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/post/reply"
OK
```

若未订阅过该 topic，则返回：

```c
NO UNSUBSCRIBE

OK
```



---

### [断开 MQTT 连接](https://docs.espressif.com/projects/esp-at/zh_CN/latest/esp32/AT_Command_Set/MQTT_AT_Commands.html#at-mqttclean-mqtt)

断开 MQTT 连接，释放资源。

```c
AT+MQTTCLEAN=<LinkID>
AT+MQTTCLEAN=0
//响应：
OK

//打开云平台，可见设备变为离线
```

- **<LinkID>**：当前仅支持 link ID 0。