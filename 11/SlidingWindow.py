import threading
import time
import random
import queue

# --- 配置参数 ---
WINDOW_SIZE = 5        # 窗口大小
TOTAL_PACKETS = 20     # 总发送包数
LOSS_RATE = 0.15       # 丢包率 (数据包或ACK均可能丢)
TIMEOUT = 2.0          # 超时重传时间
SEND_INTERVAL = 0.5    # 发送间隔 (便于观察)

class SlidingWindowSimulator:
    def __init__(self):
        # 发送方状态
        self.send_base = 0
        self.next_seq_num = 0
        self.sender_lock = threading.Lock()
        self.timer_start_time = None
        
        # 接收方状态
        self.rcv_base = 0
        self.receiver_lock = threading.Lock()
        
        # 通信信道
        self.data_channel = queue.Queue()
        self.ack_channel = queue.Queue()
        
        self.running = True

    def log_status(self):
        """监控线程：每秒输出当前窗口状态"""
        print("\n" + "="*60)
        print(f"{'滑动窗口实时状态监控':^50}")
        print("="*60)
        while self.running:
            with self.sender_lock:
                sb = self.send_base
                nsn = self.next_seq_num
                # 发送端计算
                sent_acked = list(range(0, sb))
                sent_not_acked = list(range(sb, nsn))
                can_send_not_sent = list(range(nsn, sb + WINDOW_SIZE))
                not_allowed = "Sequence > " + str(sb + WINDOW_SIZE - 1)
                available_window = (sb + WINDOW_SIZE) - nsn

            with self.receiver_lock:
                rb = self.rcv_base
                # 接收端计算
                acked_delivered = list(range(0, rb))
                allowed_receive = list(range(rb, rb + WINDOW_SIZE))
                not_allowed_rcv = "Sequence > " + str(rb + WINDOW_SIZE - 1)
                advertised_window = WINDOW_SIZE # 简化模型：缓冲区总是清空

            print(f"\n--- 发送方 (Sender) ---")
            print(f"  [已发送并确认]: {sent_acked[-5:] if sent_acked else []} (仅列出末尾)")
            print(f"  [已发送未确认]: {sent_not_acked}")
            print(f"  [允许发尚未发]: {can_send_not_sent}")
            print(f"  [不可发送部分]: {not_allowed}")
            print(f"  * 可用窗口大小: {available_window}")

            print(f"--- 接收方 (Receiver) ---")
            print(f"  [已交付主机]:   {acked_delivered[-5:] if acked_delivered else []}")
            print(f"  [允许接收范围]: {allowed_receive}")
            print(f"  [不可接收部分]: {not_allowed_rcv}")
            print(f"  * 通知窗口大小: {advertised_window}")
            
            if sb >= TOTAL_PACKETS:
                break
            time.sleep(1.5)

    def sender(self):
        """甲方：发送线程"""
        while self.send_base < TOTAL_PACKETS:
            with self.sender_lock:
                # 只要窗口没满且还有数据，就持续发送
                if self.next_seq_num < self.send_base + WINDOW_SIZE and self.next_seq_num < TOTAL_PACKETS:
                    pkt = self.next_seq_num
                    if random.random() > LOSS_RATE:
                        self.data_channel.put(pkt)
                    
                    if self.send_base == self.next_seq_num:
                        self.timer_start_time = time.time()
                    
                    self.next_seq_num += 1
                    time.sleep(SEND_INTERVAL)

            # 检查超时 (GBN逻辑：一旦超时，重置 next_seq_num 到 send_base 重新开始)
            if self.timer_start_time and (time.time() - self.timer_start_time > TIMEOUT):
                with self.sender_lock:
                    print(f"\n[!] 甲方检测到超时，准备从 Seq {self.send_base} 开始重传...")
                    self.next_seq_num = self.send_base
                    self.timer_start_time = time.time()

            # 处理接收到的 ACK
            try:
                ack = self.ack_channel.get_nowait()
                with self.sender_lock:
                    if ack >= self.send_base:
                        self.send_base = ack + 1
                        self.timer_start_time = time.time() if self.send_base != self.next_seq_num else None
            except queue.Empty:
                pass

    def receiver(self):
        """乙方：接收线程"""
        while self.rcv_base < TOTAL_PACKETS:
            try:
                pkt = self.data_channel.get(timeout=2)
                with self.receiver_lock:
                    if pkt == self.rcv_base:
                        # 收到期待的包
                        if random.random() > LOSS_RATE:
                            self.ack_channel.put(self.rcv_base)
                        self.rcv_base += 1
                    else:
                        # 收到乱序包，GBN 丢弃并重发最后一个正确包的 ACK
                        if self.rcv_base > 0 and random.random() > LOSS_RATE:
                            self.ack_channel.put(self.rcv_base - 1)
            except queue.Empty:
                continue

    def start(self):
        threads = [
            threading.Thread(target=self.sender),
            threading.Thread(target=self.receiver),
            threading.Thread(target=self.log_status)
        ]
        for t in threads: t.start()
        for t in threads: t.join()
        self.running = False
        print("\n同步任务圆满完成！")

if __name__ == "__main__":
    sim = SlidingWindowSimulator()
    sim.start()