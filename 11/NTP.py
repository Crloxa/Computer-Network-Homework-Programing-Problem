import socket
import struct
import time
import sys
from datetime import datetime

def run_ntp_server(target_time_str):
    # NTP 时间戳起点为 1900-01-01 00:00:00
    # Unix 时间戳起点为 1970-01-01 00:00:00
    # 二者相差 2208988800 秒
    NTP_OFFSET = 2208988800

    try:
        # 解析命令行输入的时间字符串
        target_dt = datetime.strptime(target_time_str, "%Y-%m-%d %H:%M:%S")
        # 转换为 NTP 秒数
        target_ntp_seconds = int(target_dt.timestamp()) + NTP_OFFSET
    except ValueError:
        print("错误: 时间格式应为 'YYYY-MM-DD HH:MM:SS'")
        return

    # 创建 UDP 套接字
    addr = ('127.0.0.1', 123)
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind(addr)
    except PermissionError:
        print("错误: 请以管理员权限运行此程序以监听 123 端口。")
        return
    except OSError as e:
        print(f"错误: 端口绑定失败 (可能 W32Time 服务正在运行): {e}")
        return

    print(f"NTP 服务器已启动，模拟时间: {target_time_str}")
    print("正在等待 Windows 客户端请求...")

    while True:
        try:
            # 接收客户端请求 (NTP 包标准长度为 48 字节)
            data, client_addr = sock.recvfrom(1024)
            
            # 提取客户端发出的 Transmit Timestamp (位于包的 40-48 字节)
            # 必须将其填入响应包的 Origin Timestamp 字段
            client_transmit_timestamp = data[40:48]

            # 构造 NTP 响应包
            # LI=0 (无警告), VN=3 (版本3), Mode=4 (服务器) -> 0x1C
            li_vn_mode = 0x1C 
            stratum = 2         # 二级时间服务器
            poll = 4            # 最小轮询间隔
            precision = -20     # 时钟精度
            root_delay = 0
            root_dispersion = 0
            ref_id = b"LOCL"    # 本地参考源

            # 时间戳构造 (均为 8 字节: 4字节整数秒 + 4字节小数部分)
            # 我们直接使用固定的目标时间，忽略微秒部分
            transmit_ts_sec = struct.pack("!I", target_ntp_seconds)
            transmit_ts_fra = struct.pack("!I", 0)
            timestamp_full = transmit_ts_sec + transmit_ts_fra

            # 组装完整的 48 字节 NTP 包
            response = struct.pack(
                "!B B b b I I 4s",
                li_vn_mode,
                stratum,
                poll,
                precision,
                root_delay,
                root_dispersion,
                ref_id
            )
            
            # 补充时间戳部分
            response += timestamp_full          # Reference Timestamp
            response += client_transmit_timestamp # Origin Timestamp 
            response += timestamp_full          # Receive Timestamp
            response += timestamp_full          # Transmit Timestamp

            sock.sendto(response, client_addr)
            print(f"响应发送至 {client_addr}，时间同步完成。")

        except KeyboardInterrupt:
            print("\n服务器正在关闭...")
            break

    sock.close()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        # 如果没有参数，默认使用题目的时间字符串
        time_str = "2019-05-01 10:41:00"
    else:
        time_str = sys.argv[1]
    
    run_ntp_server(time_str)
    
# cmd管理员模式下输入下面命令
# net stop w32time
# python ntp_server.py "2019-05-01 10:41:00"
# 按照题目指示流程做即可
# 复原需要输入下面指令
# net start w32time
# 并把服务器地址改为time.windows.com