#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <random>
#include <atomic>

// 模拟参数
const int SLOT_TIME_MS = 50;      // 争用期（时隙）
const int MAX_ATTEMPTS = 16;      // 最大重传次数
const int FRAME_LEN_MS = 200;     // 发送一个完整帧需要的时间

// 信道状态
enum class ChannelState { IDLE, BUSY, COLLISION };

class EthernetBus {
public:
    // 互斥锁
    std::mutex mtx;
    ChannelState state = ChannelState::IDLE;
    int active_stations = 0;

    // 载波侦听
    bool is_idle() {
        // lock_guard 自动锁线程
        std::lock_guard<std::mutex> lock(mtx);
        return state == ChannelState::IDLE;
    }

    // 尝试占用信道
    void start_transmitting() {
        std::lock_guard<std::mutex> lock(mtx);
        // 获取到锁后增加活动站点
        active_stations++;
        if (active_stations > 1) {
            state = ChannelState::COLLISION;
        } else {
            state = ChannelState::BUSY;
        }
    }

    // 释放信道
    void stop_transmitting() {
        std::lock_guard<std::mutex> lock(mtx);
        active_stations--;
        if (active_stations == 0) {
            state = ChannelState::IDLE;
        } else if (active_stations == 1) {
            state = ChannelState::BUSY;
        }
    }

    // 冲突检测：检查当前是不是处于冲突状态
    bool check_collision() {
        std::lock_guard<std::mutex> lock(mtx);
        return state == ChannelState::COLLISION;
    }
};

// 站点类
class Station {
private:
    int id;             // 站点编号
    EthernetBus& bus;   // 总线
    std::mt19937 rng;   // 随机数生成器

public:
    Station(int _id, EthernetBus& _bus) 
        : id(_id), bus(_bus), rng(std::random_device{}()) {}

    // 发送帧
    void send_frame() {
        int attempts = 0;       // 重传次数
        bool success = false;

        while (attempts < MAX_ATTEMPTS && !success) {
            // 1. 载波侦听 (Non-persistent CSMA)
            while (!bus.is_idle()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            // 2. 开始发送
            bus.start_transmitting();
            printf("[站点 %d] 开始尝试发送数据...\n", id);

            // 3. 冲突检测 (在发送过程中持续监听)
            bool collision_detected = false;
            auto start_time = std::chrono::steady_clock::now();
            
            // 模拟发送过程
            while (std::chrono::steady_clock::now() - start_time < std::chrono::milliseconds(FRAME_LEN_MS)) {
                if (bus.check_collision()) {
                    collision_detected = true;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            if (collision_detected) {
                // 4. 监听到冲突：停止发送并发送拥塞信号 (Jam Signal)
                bus.stop_transmitting();
                attempts++;
                printf("[站点 %d] !!! 检测到冲突 (第 %d 次) !!! 发送拥塞信号...\n", id, attempts);

                // 5. 二进制指数退避
                int k = std::min(attempts, 10);
                int range = (1 << k) - 1;
                std::uniform_int_distribution<int> dist(0, range);
                int backoff_slots = dist(rng);
                
                printf("[站点 %d] 进入退避，等待 %d 个时隙...\n", id, backoff_slots);
                std::this_thread::sleep_for(std::chrono::milliseconds(backoff_slots * SLOT_TIME_MS));
            } else {
                // 发送成功
                bus.stop_transmitting();
                success = true;
                printf("[站点 %d] >>> 帧发送成功 <<<\n", id);
            }
        }

        if (!success) {
            printf("[站点 %d] 达到最大重传次数，发送失败。\n", id);
        }
    }
};

int main() {
    EthernetBus shared_bus;
    std::vector<std::thread> stations;

    printf("=== CSMA/CD 多线程模拟开始 ===\n");

    // 创建 5 个站点同时尝试发送
    for (int i = 1; i <= 5; ++i) {
        stations.emplace_back([&shared_bus, i]() {
            Station s(i, shared_bus);
            // 每个站点随机延迟一下再开始，模拟真实突发流量
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 100));
            s.send_frame();
        });
    }

    for (auto& t : stations) {
        t.join();
    }

    printf("=== 模拟结束 ===\n");
    return 0;
}