#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
using namespace std;

const double PI = 3.141592653589793;

// 原始连续信号
double signal(double t) {
    double f1 = 5.0;
    double f2 = 20.0;
    return sin(2 * PI * f1 * t) + 0.5 * sin(2 * PI * f2 * t);
}

// sinc函数
double sinc(double x) {
    if (fabs(x) < 1e-8) return 1.0;
    return sin(PI * x) / (PI * x);
}

// 采样
vector<double> sampleSignal(double fs, double duration, vector<double>& timeSamples) {
    vector<double> samples;
    double T = 1.0 / fs;

    for (double t = 0; t < duration; t += T) {
        timeSamples.push_back(t);
        samples.push_back(signal(t));
    }
    return samples;
}

// sinc重建
double reconstruct(double t, const vector<double>& samples, const vector<double>& timeSamples, double fs) {
    double sum = 0.0;

    for (size_t n = 0; n < samples.size(); n++) {
        sum += samples[n] * sinc(fs * (t - timeSamples[n]));
    }
    return sum;
}

int main() {
    double duration = 1.0;     // 信号时长
    double highResFs = 1000.0; // 用于“连续信号”的高分辨率

    // 高分辨率时间轴
    vector<double> t_cont;
    vector<double> x_cont;
    for (double t = 0; t < duration; t += 1.0 / highResFs) {
        t_cont.push_back(t);
        x_cont.push_back(signal(t));
    }

    // 两种采样率

    double fs_good = 50.0; // 满足Nyquist
    double fs_bad  = 30.0; // 不满足

    vector<double> t_sample_good, t_sample_bad;
    vector<double> samples_good = sampleSignal(fs_good, duration, t_sample_good);
    vector<double> samples_bad  = sampleSignal(fs_bad, duration, t_sample_bad);

    // 重建
    vector<double> recon_good, recon_bad;

    for (double t : t_cont) {
        recon_good.push_back(reconstruct(t, samples_good, t_sample_good, fs_good));
        recon_bad.push_back(reconstruct(t, samples_bad, t_sample_bad, fs_bad));
    }

    // 输出
    ofstream file("result.csv");
    file << "t,original,good_recon,bad_recon\n";

    for (size_t i = 0; i < t_cont.size(); i++) {
        file << t_cont[i] << ","
             << x_cont[i] << ","
             << recon_good[i] << ","
             << recon_bad[i] << "\n";
    }

    file.close();

    cout << "数据已输出" << endl;
    return 0;
}