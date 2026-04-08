import pandas as pd
import matplotlib.pyplot as plt

# 读取数据
data = pd.read_csv("result.csv")

t = data["t"]
original = data["original"]
good = data["good_recon"]
bad = data["bad_recon"]

# 绘图
plt.figure(figsize=(10, 6))

plt.plot(t, original, label="Original Signal", linewidth=2)
plt.plot(t, good, '--', label="Reconstructed (Good, fs > 2fmax)")
plt.plot(t, bad, ':', label="Reconstructed (Bad, fs < 2fmax)")

plt.xlabel("Time (s)")
plt.ylabel("Amplitude")
plt.title("Nyquist Sampling Theorem Verification")

plt.legend()
plt.grid()

plt.show()