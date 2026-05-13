ReactOS 中 `nslookup` 的源码位于：

## ReactOS nslookup 源码目录

### Doxygen 文件入口

[ReactOS nslookup 文件列表](https://doxygen.reactos.org/files.html?utm_source=chatgpt.com)

### nslookup.c 源码

[nslookup.c 源码页面](https://doxygen.reactos.org/d8/d89/nslookup_8c_source.html?utm_source=chatgpt.com)

### nslookup.c 函数文档

[nslookup.c 函数说明](https://doxygen.reactos.org/d8/d89/nslookup_8c.html?utm_source=chatgpt.com)

### GitHub 仓库目录（源码目录）

[ReactOS GitHub 仓库 nslookup 目录](https://github.com/reactos/reactos/tree/master/base/applications/network/nslookup?utm_source=chatgpt.com)

---

# Windows 下 nslookup 的整体处理流程（结合 ReactOS 实现分析）

ReactOS 的 nslookup 基本是在模仿 Windows 的 nslookup 行为，因此它非常适合研究 Windows DNS 查询程序的内部流程。

这个程序并不是简单调用一个“系统 DNS API”就结束，而是：

> 自己构造 DNS 报文 → UDP 发送 → 接收 DNS 响应 → 解析 DNS 协议字段 → 输出结果

也就是说：

它本质上是一个“用户态 DNS 客户端”。

---

# 一、程序入口：main()

源码：

```c
int main(int argc, char *argv[])
```

位置：

```text
nslookup.c line 757
```

它做的事情：

---

## 1. 初始化 Winsock

```c
WSAStartup(MAKEWORD(2,2), &wsaData);
```

这是 Windows 网络程序标准初始化。

因为后面要：

* 创建 socket
* 发送 UDP
* 接收 DNS 数据包

---

## 2. 获取系统 DNS 配置

代码：

```c
GetNetworkParams()
```

这个 API 来自：

```c
iphlpapi.dll
```

它会读取：

* 默认 DNS 服务器
* 本地域名
* DNS 搜索列表

例如：

```text
8.8.8.8
114.114.114.114
```

随后保存到：

```c
State.DefaultServerAddress
```

这是整个 nslookup 后面真正发送 DNS 请求的目标。

---

## 3. 初始化默认状态

程序内部维护一个全局状态：

```c
STATE State;
```

里面保存：

* 查询类型
* 默认 DNS 服务器
* 是否递归
* 是否 debug
* 默认 domain
* class
* timeout

类似：

```text
set type=A
set debug
set recurse
```

这些都保存在 State 中。

---

# 二、命令行解析：ParseCommandLine()

源码：

```c
BOOL ParseCommandLine(int argc, char *argv[])
```

([doxygen.reactos.org][1])

---

它负责：

## 1. 解析参数

例如：

```bash
nslookup www.baidu.com
```

或者：

```bash
nslookup -type=MX gmail.com 8.8.8.8
```

---

## 2. 判断 DNS 服务器

这里有个重要逻辑：

```c
if (IsValidIP(Server))
```

意思是：

如果用户输入的是：

```text
8.8.8.8
```

那么直接当作 DNS Server IP。

否则：

```text
dns.google
```

会先解析这个域名对应的 IP。

这里它会调用：

```c
PerformInternalLookup()
```

---

# 三、内部查询：PerformInternalLookup()

这是程序最关键的部分之一。

源码：

```c
BOOL PerformInternalLookup(PCHAR pAddr, PCHAR pResult)
```

([doxygen.reactos.org][1])

---

它的作用：

# “解析 DNS 服务器自身”

例如：

```bash
nslookup www.baidu.com dns.google
```

这里：

```text
dns.google
```

需要先被解析成 IP。

于是：

---

## 1. 判断是不是 IP

```c
if (IsValidIP(pAddr))
    Type = TYPE_PTR;
else
    Type = TYPE_A;
```

意思：

* 输入是域名 → 做 A 查询
* 输入是 IP → 做 PTR 反向解析

---

## 2. PTR 查询的处理

如果输入：

```text
8.8.8.8
```

代码会：

```c
ReverseIP()
```

把它变成：

```text
8.8.8.8.in-addr.arpa
```

这是 DNS PTR 查询标准格式。

---

# 四、真正 DNS 查询：PerformLookup()

核心函数：

```c
void PerformLookup(PCHAR pAddr)
```

这是：

# nslookup 真正干活的地方

---

它的流程几乎就是：

# 手工实现 DNS 协议

而不是调用高级 API。

---

# 五、DNS 报文构造

这里是整个程序最关键的部分。

它会自己构造：

```text
DNS Header
Question Section
```

---

## 1. DNS Header

会填写：

* Transaction ID
* Flags
* Questions 数量
* Answers 数量

例如：

```text
ID = 0x1234
Flags = recursion desired
QDCOUNT = 1
```

---

程序里的：

```c
RequestID
```

就是 DNS Transaction ID。

用于：

# 匹配请求和响应

因为 UDP 是无连接的。

---

# 六、域名编码

DNS 协议中的：

```text
www.baidu.com
```

不能直接发送。

需要变成：

```text
3www5baidu3com0
```

即：

```text
[length][label]
```

格式。

程序内部会：

* 拆分 `.`
* 写入长度字节
* 写入字符串

这是 DNS RFC 标准格式。

---

# 七、发送 DNS 请求

真正发送由：

```c
SendRequest()
```

完成。

它内部：

---

## 1. 创建 UDP Socket

```c
socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)
```

DNS 默认 UDP 53。

---

## 2. 连接 DNS Server

目标：

```text
State.DefaultServerAddress
```

例如：

```text
8.8.8.8:53
```

---

## 3. sendto()

发送刚才构造的 DNS 报文。

---

# 八、接收 DNS 响应

之后：

```c
recvfrom()
```

等待 DNS 返回。

DNS 服务器会返回：

```text
Header
Question
Answer
Authority
Additional
```

完整 DNS 数据包。

---

# 九、解析 DNS 响应

这里是代码量最大的部分。

程序需要：

# 自己解析 DNS 协议

---

## 1. 检查响应码

代码中：

```c
RCODE_NOERROR
RCODE_NXDOMAIN
RCODE_REFUSED
```

对应：

* 成功
* 域名不存在
* 拒绝访问

---

## 2. 解析 Answer 数量

```c
NumAnswers
```

如果是：

```text
A Record
```

则解析 IPv4。

如果：

```text
AAAA
```

则解析 IPv6。

---

## 3. 解析域名压缩

DNS 响应里大量使用：

# Name Compression

例如：

```text
0xC0 0x0C
```

表示：

“跳转到偏移 0x0C 的名字”。

ReactOS 用：

```c
ExtractName()
```

处理。

这是 DNS 协议里最复杂的部分之一。

---

# 十、提取 IP

例如：

```text
www.baidu.com → 110.242.68.4
```

程序会：

```c
ExtractIP()
```

把二进制 IPv4 转成字符串。

---

# 十一、输出结果

最后：

```c
printf()
```

输出：

```text
Name: www.baidu.com
Address: 110.242.68.4
```

---

# 十二、Windows nslookup 的本质

从 ReactOS 实现能看出：

Windows nslookup 本质是：

# 一个用户态 DNS 协议实现器

它：

* 不依赖浏览器
* 不依赖 hosts
* 不依赖高级域名 API

而是：

# 直接和 DNS 服务器通信

---

# 十三、它与 ping 的区别

很多人误以为：

```text
ping 域名
```

和：

```text
nslookup 域名
```

一样。

实际上不同。

---

## ping

通常调用：

```text
getaddrinfo()
```

属于系统 Resolver。

会：

* 查 hosts
* 查 DNS cache
* LLMNR
* NetBIOS
* DNS

---

## nslookup

更接近：

# 原始 DNS 客户端

直接发 DNS 协议包。

因此：

```text
hosts 能解析
但 nslookup 失败
```

是完全可能的。

---

# 十四、整个流程总结（非常重要）

Windows / ReactOS nslookup 的核心流程：

```text
main()
    ↓
读取系统 DNS 配置
    ↓
ParseCommandLine()
    ↓
PerformLookup()
    ↓
构造 DNS Header
    ↓
编码域名
    ↓
SendRequest()
    ↓
UDP 发送到 53 端口
    ↓
接收 DNS 响应
    ↓
解析 DNS Header
    ↓
解析 Answer Record
    ↓
输出结果
```

---

# 十五、你可以重点阅读的源码函数

建议重点看：

| 函数                    | 作用          |
| --------------------- | ----------- |
| main                  | 程序入口        |
| ParseCommandLine      | 参数解析        |
| PerformLookup         | DNS 查询核心    |
| PerformInternalLookup | DNS 服务器自身解析 |
| SendRequest           | UDP 发送      |
| ExtractName           | DNS 名字压缩解析  |
| ExtractIP             | 提取 IP       |
| ReverseIP             | PTR 反向查询    |

这些函数已经基本覆盖：

# Windows DNS 客户端底层实现原理

([doxygen.reactos.org][1])

[1]: https://doxygen.reactos.org/d8/d89/nslookup_8c.html?utm_source=chatgpt.com "ReactOS: base/applications/network/nslookup/nslookup.c File Reference"