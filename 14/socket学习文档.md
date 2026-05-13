# Socket 零基础学习文档（基于本次两个项目）

## 1. 先用一句话理解 Socket

可以把 socket 理解为程序在网络上的“插口”。

- 发送方把数据写进插口，操作系统负责把数据发到网络。
- 接收方从插口读数据，操作系统负责把网络数据交给程序。

你在代码里最常看到的就是：

1. 初始化网络环境
2. 创建 socket
3. 绑定地址/端口（接收端常见）
4. 发送或接收数据
5. 关闭 socket

---

## 2. 这两个项目在做什么

### 项目1：多播分发模拟

目录：14/project1_multicast

目标：一个发送端，把同一个文件发给多个接收端（组播地址）。

- 发送端：multicast_sender.c
- 接收端：multicast_receiver.c
- 公共定义：common.h

核心特征：UDP + 多播组地址。

### 项目2：TCP/UDP 对比模拟

目录：14/project2_tcp_udp

目标：同样的文件分别走 TCP 和 UDP，比较速率和完整性。

- TCP：tcp_client.c / tcp_server.c
- UDP：udp_client.c / udp_server.c
- 公共定义：common.h

核心特征：同一测试文件，不同传输协议。

---

## 3. 每个 socket 环节在代码里是怎么运作的

## 3.1 初始化：为什么先调用 WSAStartup

在 Windows 上，网络库不是开机就给你当前进程直接用。
所以程序开始时要调用 WSAStartup，结束时调用 WSACleanup。

对应文件：
- 14/project1_multicast/common.h
- 14/project2_tcp_udp/common.h

这一步就像“先给网络功能通电”。

---

## 3.2 创建 socket：我到底创建了什么

典型调用是 socket(地址族, 类型, 协议)。

- AF_INET：IPv4
- SOCK_STREAM：TCP（面向连接）
- SOCK_DGRAM：UDP（无连接）

在项目中：

- 项目1发送端和接收端都用 SOCK_DGRAM（UDP）。
- 项目2里 TCP 用 SOCK_STREAM，UDP 用 SOCK_DGRAM。

---

## 3.3 绑定端口 bind：谁需要绑定

接收端通常需要 bind。

原因：如果你不告诉系统“我监听哪个端口”，系统不知道把来的数据交给谁。

项目里的例子：

- 项目1接收端绑定了 9000 端口。
- 项目2 tcp_server 和 udp_server 都绑定了 9100 端口。

发送端一般不强制 bind，系统会自动分配临时端口。

---

## 3.4 发送和接收：TCP 与 UDP 的区别

### UDP 路径（项目1 + 项目2 UDP）

发送端常用 sendto，接收端常用 recv 或 recvfrom。

特点：
- 不建立连接，直接发。
- 可能丢包、乱序、重复。
- 快，但不保证完整。

所以你的代码加了一个简单包头：
- magic：区分数据包还是结束包
- seq：分片序号
- data_len：有效负载长度

这就是“自己给 UDP 加一点最基础的可识别信息”。

### TCP 路径（项目2 TCP）

服务端流程：bind -> listen -> accept
客户端流程：connect
然后双方用 send/recv 按字节流传输。

特点：
- 有连接、可靠、按序。
- 适合文件完整传输。
- 代码里还写了 send_all 和 recv_all，确保把指定长度收发完整。

---

## 3.5 多播（项目1）是怎么实现“一发多收”的

发送端把目的地址设成组播地址 239.0.0.1:9000。

接收端除了 bind 端口，还要做一步“加入组播组”：
- setsockopt + IP_ADD_MEMBERSHIP

加入后，系统才会把这个组地址的数据包投递给该接收程序。

所以你可以同时开两个接收端，都能收到发送端同一份数据。

---

## 3.6 为什么你测试里会出现 UDP 丢包

这和代码逻辑及 UDP 天性有关，不是你操作错了。

- UDP 不保证送达。
- 大文件拆成很多小包后，任何环节都可能丢。
- 如果结束包丢了，接收端可能一直等。
- 你看到的 seq 跳变告警，就是接收端发现中间有包没到。

这也是为什么 TCP 的完整性通常明显好于 UDP。

---

## 4. 把两个项目当作一张“学习地图”

你可以按下面顺序学，很容易建立直觉：

1. 先看项目2的 TCP
- 能最稳定地把文件发完。
- 学会 client/server、connect/accept、send/recv。

2. 再看项目2的 UDP
- 代码更短，但会出现丢包和乱序问题。
- 理解“快”和“可靠”常常不能同时极致。

3. 最后看项目1的多播
- 在 UDP 基础上再加“组播组”概念。
- 体会一对多发送的网络模型。

---

## 5. 本项目里每个文件的学习重点

### 项目1

- 14/project1_multicast/common.h
  - Windows socket 初始化和清理
  - 包头定义（magic/seq/data_len）

- 14/project1_multicast/multicast_sender.c
  - 分块读文件
  - sendto 发往组播地址
  - 发送结束标记

- 14/project1_multicast/multicast_receiver.c
  - bind 端口
  - 加入组播组
  - 识别包头并写文件
  - 检测序号跳变

### 项目2

- 14/project2_tcp_udp/tcp_server.c
  - bind/listen/accept 三件套
  - 先收文件大小，再收文件内容

- 14/project2_tcp_udp/tcp_client.c
  - connect 到服务端
  - 先发文件大小，再发文件内容

- 14/project2_tcp_udp/udp_client.c
  - 分片并 sendto
  - 发送结束标记

- 14/project2_tcp_udp/udp_server.c
  - bind + recv
  - 按包头落盘并做丢包提示

- 14/project2_tcp_udp/common.h
  - send_all/recv_all 用于 TCP 完整收发

---

## 6. 零基础常见误区

1. 误区：UDP 比 TCP 快，所以一定更适合文件传输。
- 实际：如果没有可靠性机制，UDP 在大文件上很容易不完整。

2. 误区：能跑通一次就代表协议设计没问题。
- 实际：网络程序要看多轮测试和完整性。

3. 误区：发送端结束了，接收端一定也会自然结束。
- 实际：UDP 的结束包也可能丢。

---

## 7. 你现在可以这样继续学

1. 先把 TCP 路径读懂并复述一遍调用链。
2. 对照 UDP 的调用链，找出“为什么会丢包”。
3. 在 UDP 中尝试补一个最小机制：重复发送结束包，或者超时退出。
4. 再进阶：加入 ACK 重传和窗口机制。

这样你会非常清楚：
Socket 编程不是只会调用函数，而是围绕“可靠性、顺序、时延、吞吐”做设计取舍。
