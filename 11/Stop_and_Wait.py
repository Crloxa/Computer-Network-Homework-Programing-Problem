import threading
import time
import random
import queue

# 配置参数
LOSS_PROBABILITY = 0.2  # 丢包概率
TIMEOUT = 1.0           # 超时时间 (秒)
TOTAL_PACKETS = 10       # 总共发送的数据包数量

class Packet:
    """数据包类"""
    def __init__(self, seq, data):
        self.seq = seq
        self.data = data
        self.timestamp = time.time()

    def __repr__(self):
        return f"[Seq:{self.seq}, Data:{self.data}]"

class StopAndWaitSimulator:
    def __init__(self):
        # 甲方到乙方的信道
        self.forward_channel = queue.Queue()
        # 乙方到甲方的信道 (ACK)
        self.backward_channel = queue.Queue()

    def network_send(self, q, packet, label):
        """模拟网络传输，附带随机丢包"""
        if random.random() < LOSS_PROBABILITY:
            print(f"！丢失信号：{label} {packet} 在传输中丢失了...")
        else:
            q.put(packet)

    def sender_thread(self):
        """甲方：发送方"""
        curr_seq = 0
        packets_sent = 0

        while packets_sent < TOTAL_PACKETS:
            data = f"Message-{packets_sent}"
            pkt = Packet(curr_seq, data)
            
            print(f"甲方: 准备发送 {pkt}，启动定时器...")
            
            # 发送数据
            self.network_send(self.forward_channel, pkt, "数据包")
            
            # 等待确认 (ACK)
            try:
                # 等待超时
                ack_pkt = self.backward_channel.get(timeout=TIMEOUT)
                
                if ack_pkt.seq == curr_seq:
                    print(f"甲方: 收到正确的 ACK {ack_pkt.seq}。耗时: {time.time() - pkt.timestamp:.2f}s")
                    curr_seq = 1 - curr_seq  # 切换序列号 (0 or 1)
                    packets_sent += 1
                else:
                    print(f"甲方: 收到错误的序列号 ACK，忽略。")
            except queue.Empty:
                print(f"甲方: 超时了！未收到 ACK {curr_seq}，准备重传...")
                # 循环继续，但不增加 packets_sent，从而实现重传

        print("--- 甲方发送完毕 ---")

    def receiver_thread(self):
        """乙方：接收方"""
        expected_seq = 0
        
        while True:
            try:
                # 乙方一直监听信道
                pkt = self.forward_channel.get(timeout=10)
                
                if pkt.seq == expected_seq:
                    print(f"乙方: 成功接收 {pkt}，正在发送 ACK...")
                    # 发送确认
                    ack = Packet(pkt.seq, "ACK")
                    self.network_send(self.backward_channel, ack, "确认包")
                    expected_seq = 1 - expected_seq
                else:
                    print(f"乙方: 收到重复包 {pkt.seq}，重发之前的 ACK...")
                    # 即使是重复包，也要重发 ACK，防止甲方一直超时
                    ack = Packet(pkt.seq, "ACK")
                    self.network_send(self.backward_channel, ack, "确认包")
            except queue.Empty:
                break
        
        print("--- 乙方停止监听 ---")

    def start(self):
        t1 = threading.Thread(target=self.sender_thread)
        t2 = threading.Thread(target=self.receiver_thread)
        
        t1.start()
        t2.start()
        
        t1.join()
        t2.join()

if __name__ == "__main__":
    sim = StopAndWaitSimulator()
    sim.start()