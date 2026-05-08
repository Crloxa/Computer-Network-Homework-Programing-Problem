1. 编写 Socket API 程序实现，通过多播的形式在局域网内分发一个较大（如：
大于 200MB）的文件，接收方接收并保存。

2. 编写 Socket API 程序实现， TCP 和 UDP 传输一个大文件，在不同环境下对
比有效传输速率。如：本机或本地网络，远程互联网络（不一定要完成）。

---

## 实现落地（已开始）

按要求拆分为两个独立文件夹：

1. 项目1（多播分发模拟）
- 目录：14/project1_multicast
- 文件：
	- multicast_sender.c
	- multicast_receiver.c
	- common.h
	- README.md

2. 项目2（TCP/UDP 速率对比模拟）
- 目录：14/project2_tcp_udp
- 文件：
	- tcp_server.c
	- tcp_client.c
	- udp_server.c
	- udp_client.c
	- common.h
	- README.md

## 说明

- 均采用 C 风格 Socket API（Windows winsock2）。
- 当前版本为教学模拟实现，优先流程跑通与速率统计，不实现复杂重传机制。
- 建议先 50MB 文件联调，再扩展到 200MB 文件测试。