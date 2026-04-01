import numpy as np
import matplotlib.pyplot as plt
import math

# --- 参数配置 ---
# 建筑尺寸
WIDTH = 100      # X轴长度 (m)
HEIGHT = 80      # Y轴长度 (m)
RESOLUTION = 1   # 网格精度 1m

# 墙体衰减 (dB)
LOSS_LOAD_BEARING = 12
LOSS_PARTITION = 6
LOSS_GLASS = 7

# 信号参数 (这里以 5GHz 为主力覆盖频段进行模拟计算)
TX_POWER = 20        # AP 发射功率 (dBm)
FREQ_5G = 5000       # 频率 (MHz)
TARGET_RSSI = -65    # 目标信号强度 (dBm)

# 频段信道池 (用于着色算法分配)
CHANNELS_2_4G = [1, 6, 11]
CHANNELS_5G = [36, 40, 44, 48, 52, 56, 60, 64]

def calculate_free_space_path_loss(distance, freq_mhz):
    """计算自由空间损耗 (FSPL)"""
    if distance < 1:
        return 0
    # FSPL = 20*log10(d) + 20*log10(f) - 27.55
    return 20 * math.log10(distance) + 20 * math.log10(freq_mhz) - 27.55

def check_wall_intersections(x1, y1, x2, y2):
    """
    简化的墙体穿透检测：
    假设走廊在 Y=39 到 Y=41，两侧是承重墙。
    这里仅做一个简化的几何判定，如果连线跨越了走廊边界，则认为穿透了承重墙。
    """
    loss = 0
    # 走廊上下墙体 Y 坐标
    wall_y1, wall_y2 = 39, 41
    
    # 检查是否跨越了 Y=39 的墙
    if (y1 < wall_y1 and y2 > wall_y1) or (y1 > wall_y1 and y2 < wall_y1):
        loss += LOSS_LOAD_BEARING
    # 检查是否跨越了 Y=41 的墙
    if (y1 < wall_y2 and y2 > wall_y2) or (y1 > wall_y2 and y2 < wall_y2):
        loss += LOSS_LOAD_BEARING
        
    return loss

def heuristic_ap_placement():
    """
    启发式 AP 部署：
    根据教室尺寸 (15x10) 和 5GHz 穿墙衰减大的特性，
    最佳实践是在走廊两侧的教室内交错部署，或者在走廊高密度部署。
    这里采用：每隔 20 米在走廊两侧各放置一个 AP。
    """
    aps = []
    # 走廊上侧的教室区域 (Y=20)
    for x in range(10, WIDTH, 20):
        aps.append((x, 20))
    # 走廊下侧的教室区域 (Y=60)
    for x in range(20, WIDTH, 20): # 错开 10m
        aps.append((x, 60))
    return aps

def assign_channels(aps):
    """简单的信道分配：基于取模的交错分配避免相邻同频"""
    ap_configs = []
    for i, (x, y) in enumerate(aps):
        ch_24 = CHANNELS_2_4G[i % len(CHANNELS_2_4G)]
        ch_5 = CHANNELS_5G[i % len(CHANNELS_5G)]
        ap_configs.append({'id': i+1, 'x': x, 'y': y, 'ch_2.4G': ch_24, 'ch_5G': ch_5})
    return ap_configs

def generate_heatmap(ap_configs):
    """生成信号热力图"""
    grid = np.full((HEIGHT, WIDTH), -100.0) # 初始化为底噪 (-100 dBm)
    
    for y in range(HEIGHT):
        for x in range(WIDTH):
            max_rssi = -100
            for ap in ap_configs:
                ap_x, ap_y = ap['x'], ap['y']
                dist = math.hypot(x - ap_x, y - ap_y)
                
                fspl = calculate_free_space_path_loss(dist, FREQ_5G)
                wall_loss = check_wall_intersections(x, y, ap_x, ap_y)
                
                # 接收信号强度公式
                rssi = TX_POWER - fspl - wall_loss
                if rssi > max_rssi:
                    max_rssi = rssi
            
            grid[y, x] = max_rssi
            
    return grid

# --- 主流程 ---
# 1. 计算 AP 位置
ap_positions = heuristic_ap_placement()

# 2. 分配信道
ap_configs = assign_channels(ap_positions)

# 打印配置
print("--- 教学楼单层 AP 部署方案 ---")
for ap in ap_configs:
    print(f"AP {ap['id']:02d}: 坐标(X:{ap['x']:3d}, Y:{ap['y']:2d}) | 2.4G信道: {ap['ch_2.4G']:2d} | 5G信道: {ap['ch_5G']:2d}")

# 3. 生成热力图矩阵
heatmap = generate_heatmap(ap_configs)

# 4. 可视化
plt.figure(figsize=(12, 8))
# 使用 cmap='jet' 来符合常见的无线信号热力图直觉 (红强蓝弱)
im = plt.imshow(heatmap, cmap='jet', origin='lower', extent=[0, WIDTH, 0, HEIGHT], vmin=-85, vmax=-35)

# 绘制走廊边界
plt.hlines([39, 41], 0, WIDTH, colors='white', linestyles='dashed', linewidth=2, label='Load-bearing Wall (Corridor)')

# 标记 AP 位置
for ap in ap_configs:
    plt.scatter(ap['x'], ap['y'], c='black', marker='^', s=100)
    plt.text(ap['x']+1, ap['y']+1, f"AP{ap['id']}", color='black', fontsize=9, fontweight='bold')

plt.colorbar(im, label='Signal Strength (dBm)')
plt.title('5GHz Wi-Fi Signal Heatmap (Floor 1)')
plt.xlabel('Width (m)')
plt.ylabel('Height (m)')
plt.legend()
plt.grid(False)
plt.tight_layout()
plt.show()