#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 假设全局参数：采样频率和基础频率
const double SAMPLE_RATE = 1000.0; // 1秒1000个点
const double CARRIER_FREQ = 50.0;  // 载波频率 50Hz
const double ANALOG_MSG_FREQ = 5.0; // 模拟调制信号频率 5Hz

// --- 1. 基础信号生成 ---

// 生成标准正弦载波
int generate_cover_signal(double *cover, const int size) {
    for (int i = 0; i < size; ++i) {
        double t = (double)i / SAMPLE_RATE;
        cover[i] = sin(2 * M_PI * CARRIER_FREQ * t);
    }
    return 1;
}

// 生成随机数字消息序列 (0 或 1)
int simulate_digital_modulation_signal(unsigned char *message, const int size) {
    for (int i = 0; i < size; ++i) {
        message[i] = rand() % 2;
    }
    return 1;
}

// 生成低频正弦模拟消息信号
int simulate_analog_modulation_signal(double *message, const int size) {
    for (int i = 0; i < size; ++i) {
        double t = (double)i / SAMPLE_RATE;
        message[i] = sin(2 * M_PI * ANALOG_MSG_FREQ * t);
    }
    return 1;
}

// --- 2. 调幅 (Amplitude Modulation) ---

// 数字调幅 (ASK): 有信号时为载波，无信号时为0
int modulate_digital_amplitude(double *cover, const int cover_len, const unsigned char *message, const int msg_len) {
    int pts_per_bit = cover_len / msg_len;
    for (int i = 0; i < msg_len; ++i) {
        for (int j = 0; j < pts_per_bit; ++j) {
            int idx = i * pts_per_bit + j;
            if (idx < cover_len) {
                cover[idx] *= (message[i] ? 1.0 : 0.0);
            }
        }
    }
    return 1;
}

// 模拟调幅 (AM): 载波振幅随消息信号线性变化
int modulate_analog_amplitude(double *cover, const int cover_len, const double *message, const int msg_len) {
    for (int i = 0; i < cover_len && i < msg_len; ++i) {
        // 标准AM公式: (1 + m*cos(wm*t)) * cos(wc*t)
        cover[i] = (1.0 + 0.5 * message[i]) * cover[i];
    }
    return 1;
}

// --- 3. 调频 (Frequency Modulation) ---

// 数字调频 (FSK): 0 和 1 对应不同的频率
int modulate_digital_frequency(double *cover, const int cover_len, const unsigned char *message, const int msg_len) {
    int pts_per_bit = cover_len / msg_len;
    double phase = 0;
    for (int i = 0; i < msg_len; ++i) {
        double freq = message[i] ? CARRIER_FREQ * 1.5 : CARRIER_FREQ * 0.5;
        for (int j = 0; j < pts_per_bit; ++j) {
            int idx = i * pts_per_bit + j;
            if (idx < cover_len) {
                phase += 2 * M_PI * freq / SAMPLE_RATE;
                cover[idx] = sin(phase);
            }
        }
    }
    return 1;
}

// 模拟调频 (FM): 频率随消息信号幅度变化
int modulate_analog_frequency(double *cover, const int cover_len, const double *message, const int msg_len) {
    double phase = 0;
    double kf = 20.0; // 调频指数
    for (int i = 0; i < cover_len && i < msg_len; ++i) {
        double inst_freq = CARRIER_FREQ + kf * message[i];
        phase += 2 * M_PI * inst_freq / SAMPLE_RATE;
        cover[i] = sin(phase);
    }
    return 1;
}

// --- 4. 调相 (Phase Modulation) ---

// 数字调相 (PSK): 0 和 1 对应相位翻转 (0 和 Pi)
int modulate_digital_phase(double *cover, const int cover_len, const unsigned char *message, const int msg_len) {
    int pts_per_bit = cover_len / msg_len;
    for (int i = 0; i < msg_len; ++i) {
        double phase_offset = message[i] ? M_PI : 0.0;
        for (int j = 0; j < pts_per_bit; ++j) {
            int idx = i * pts_per_bit + j;
            if (idx < cover_len) {
                double t = (double)idx / SAMPLE_RATE;
                cover[idx] = sin(2 * M_PI * CARRIER_FREQ * t + phase_offset);
            }
        }
    }
    return 1;
}

// 模拟调相 (PM): 相位随消息信号幅度变化
int modulate_analog_phase(double *cover, const int cover_len, const double *message, const int msg_len) {
    double kp = M_PI / 2.0; // 调相增益
    for (int i = 0; i < cover_len && i < msg_len; ++i) {
        double t = (double)i / SAMPLE_RATE;
        cover[i] = sin(2 * M_PI * CARRIER_FREQ * t + kp * message[i]);
    }
    return 1;
}

// --- 测试与展示 ---
void print_signal(const std::string& name, double* sig, int len) {
    std::cout << name << " (前10个采样点): ";
    for(int i=0; i<10; ++i) std::cout << std::fixed << std::setprecision(2) << sig[i] << " ";
    std::cout << "..." << std::endl;
}

int main() {
    const int SIZE = 1000;
    const int MSG_SIZE = 10; // 10个比特或段
    double cover[SIZE];
    unsigned char d_msg[MSG_SIZE];
    double a_msg[SIZE];

    // 1. 生成基础信号
    generate_cover_signal(cover, SIZE);
    simulate_digital_modulation_signal(d_msg, MSG_SIZE);
    simulate_analog_modulation_signal(a_msg, SIZE);

    // 2. 测试调幅 (以数字为例)
    std::cout << "数字消息: "; 
    for(int i=0; i<MSG_SIZE; ++i) std::cout << (int)d_msg[i] << " ";
    std::cout << std::endl;

    modulate_digital_amplitude(cover, SIZE, d_msg, MSG_SIZE);
    print_signal("数字调幅信号 (ASK)", cover, SIZE);

    // 3. 测试调频 (以模拟为例)
    generate_cover_signal(cover, SIZE); // 重置载波
    modulate_analog_frequency(cover, SIZE, a_msg, SIZE);
    print_signal("模拟调频信号 (FM)", cover, SIZE);

    return 0;
}