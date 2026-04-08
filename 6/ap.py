import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
import math

# ==========================================
# 1. 环境与参数建模 (Environment & Parameters)
# ==========================================
# 建筑尺寸
BLDG_LENGTH = 100  # X轴 100m
BLDG_WIDTH = 80    # Y轴 80m
FLOORS = 3         # 楼层数
ROOM_W, ROOM_H = 15, 10 # 教室尺寸 15m x 10m

# 墙体衰减 (dB)
ATTEN_LOAD_BEARING = 12 # 承重墙
ATTEN_PARTITION = 6     # 普通隔断墙
ATTEN_GLASS = 7         # 玻璃幕墙 (外部边界，不影响内部互通)

# 目标阈值
TARGET_DBM = -65

# 信号衰减基准参数 (基于给定的覆盖半径反推简化的对数距离路径损耗模型)
# Pr(d) = P_tx - 10*n*log10(d) - Wall_Loss
# 假设 AP 发射功率 P_tx = 20 dBm, 1米处信号约 -35 dBm
P_TX = 20
PL_1M_24 = -35 # 2.4Ghz 1米处参考接收功率
PL_1M_5 = -40  # 5Ghz 1米处参考接收功率
PATH_LOSS_EXP = 2.5 # 室内空间衰减指数

# ==========================================
# 2. 信号传播与墙体穿透计算模型 (Signal Model)
# ==========================================
def calculate_wall_crossings(x1, y1, X2, Y2):
    """
    计算 AP (x1, y1) 到网格上所有点 (X2, Y2) 穿过的墙体数量。
    返回: 承重墙穿透数矩阵, 隔断墙穿透数矩阵
    """
    # X方向的墙体 (每隔15米一道隔断墙)
    cross_x_walls = np.abs(np.floor(X2 / ROOM_W) - np.floor(x1 / ROOM_W))
    
    # Y方向的墙体 (假设 0-39 和 41-80 是教室区域，每10米一道隔断墙)
    # y=39 和 y=41 是走廊的承重墙
    cross_y_10m = np.abs(np.floor(Y2 / ROOM_H) - np.floor(y1 / ROOM_H))
    
    # 判断是否穿过 y=39 的承重墙
    cross_lb1 = np.logical_xor(y1 < 39, Y2 < 39).astype(int)
    # 判断是否穿过 y=41 的承重墙
    cross_lb2 = np.logical_xor(y1 < 41, Y2 < 41).astype(int)
    
    load_bearing = cross_lb1 + cross_lb2
    
    # 隔断墙总数 = X方向穿透 + Y方向穿透 (减去算作承重墙的部分)
    partition = cross_x_walls + cross_y_10m
    
    return load_bearing, partition

def get_signal_heatmap(aps, band, resolution=1):
    """
    计算整个楼层的信号热力图
    band: '2.4G' 或 '5G'
    """
    x = np.arange(0, BLDG_LENGTH, resolution)
    y = np.arange(0, BLDG_WIDTH, resolution)
    X, Y = np.meshgrid(x, y)
    
    # 初始化信号矩阵为极小值
    max_signal = np.full(X.shape, -100.0)
    
    base_pl = PL_1M_24 if band == '2.4G' else PL_1M_5
    
    for ap in aps:
        ax, ay = ap['x'], ap['y']
        
        # 计算欧氏距离 (加入 0.1 避免 log(0))
        dist = np.sqrt((X - ax)**2 + (Y - ay)**2) + 0.1
        
        # 计算穿墙次数
        lb_cross, part_cross = calculate_wall_crossings(ax, ay, X, Y)
        
        # 自由空间衰减 + 墙体衰减
        signal = (base_pl 
                  - 10 * PATH_LOSS_EXP * np.log10(dist) 
                  - lb_cross * ATTEN_LOAD_BEARING 
                  - part_cross * ATTEN_PARTITION)
        
        # 取最强信号 (漫游时终端连接信号最强的AP)
        max_signal = np.maximum(max_signal, signal)
        
    return X, Y, max_signal

# ==========================================
# 3. AP 部署与信道分配算法 (Deployment & Channel Allocation)
# ==========================================
def deploy_aps():
    """
    生成 AP 坐标并分配信道。
    策略：走廊承重墙衰减高达 12dB，AP必须部署在走廊两侧的教室内。
    基于教室 15x10 的规格，以网格化错开部署保证覆盖且降低同频干扰。
    """
    aps = []
    
    # Y 轴分为南区(5-35)和北区(45-75)
    # 选定 Y 坐标放置 AP：y=15, 30 (南侧), y=50, 65 (北侧)
    y_coords = [15, 30, 50, 65]
    # X 轴坐标: 每隔 20m 放置一个
    x_coords = [10, 30, 50, 70, 90]
    
    # 2.4GHz 独立信道: 1, 6, 11
    channels_24 = [1, 6, 11]
    # 5GHz 独立信道: 36, 40, 44, 48, 52, 56, 60, 64...
    channels_5 = [36, 40, 44, 48, 52, 56, 60, 64]
    
    for i, y in enumerate(y_coords):
        for j, x in enumerate(x_coords):
            # 交错安排信道 (图着色防重叠思路)
            ch_24 = channels_24[(i + j) % len(channels_24)]
            ch_5 = channels_5[(i * len(x_coords) + j) % len(channels_5)]
            
            aps.append({
                'id': f"AP_{i}_{j}",
                'x': x,
                'y': y,
                'ch_24': ch_24,
                'ch_5': ch_5
            })
    return aps

# ==========================================
# 4. 绘图与可视化 (Visualization)
# ==========================================
def plot_floor_plan(ax):
    """在图层上绘制建筑基本轮廓(墙体)"""
    # 绘制外墙 (玻璃幕墙)
    ax.add_patch(Rectangle((0, 0), BLDG_LENGTH, BLDG_WIDTH, fill=False, edgecolor='blue', lw=2))
    
    # 绘制走廊承重墙 (y=39 和 y=41)
    ax.plot([0, BLDG_LENGTH], [39, 39], color='black', lw=2.5, label='Load Bearing Wall')
    ax.plot([0, BLDG_LENGTH], [41, 41], color='black', lw=2.5)
    
    # 绘制隔断墙 (X方向每15米)
    for x in range(15, BLDG_LENGTH, 15):
        ax.plot([x, x], [0, 39], color='gray', linestyle='--', lw=1)
        ax.plot([x, x], [41, BLDG_WIDTH], color='gray', linestyle='--', lw=1)
        
    # 绘制隔断墙 (Y方向每10米)
    for y in range(10, BLDG_WIDTH, 10):
        if y not in [40]: # 走廊区域跳过
            ax.plot([0, BLDG_LENGTH], [y, y], color='gray', linestyle='--', lw=1)

def main():
    print("正在计算 AP 部署最佳位置...")
    aps = deploy_aps()
    
    print(f"共部署了 {len(aps)} 个 AP (单层)。正在输出 AP 列表及信道分配：")
    print(f"{'AP ID':<10} | {'X 坐标':<6} | {'Y 坐标':<6} | {'2.4G 信道':<10} | {'5G 信道':<10}")
    print("-" * 55)
    for ap in aps:
        print(f"{ap['id']:<10} | {ap['x']:<8} | {ap['y']:<8} | {ap['ch_24']:<12} | {ap['ch_5']:<10}")

    print("\n正在生成信号热力图模型 (请稍候)....")
    
    # 获取热力图数据
    X, Y, sig_24 = get_signal_heatmap(aps, band='2.4G', resolution=1)
    X, Y, sig_5 = get_signal_heatmap(aps, band='5G', resolution=1)
    
    # 开始绘图
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 14))
    
    # 颜色映射设置：低于 -65dBm 显示偏冷/透明色，大于 -65dBm 显示绿色->红色
    levels = np.linspace(-85, -30, 56)
    cmap = plt.get_cmap('RdYlBu_r')

    # ---- 绘制 2.4GHz 热力图 ----
    plot_floor_plan(ax1)
    cf1 = ax1.contourf(X, Y, sig_24, levels=levels, cmap=cmap, alpha=0.7)
    fig.colorbar(cf1, ax=ax1, label='Signal Strength (dBm)')
    
    # 标记 AP 及 2.4G 信道
    for ap in aps:
        ax1.plot(ap['x'], ap['y'], 'k^', markersize=8)
        ax1.text(ap['x']+1, ap['y']+1, f"CH:{ap['ch_24']}", fontsize=8, weight='bold')
        
    ax1.set_title('2.4GHz Band Signal Heatmap & Channel Allocation')
    ax1.set_xlim(0, BLDG_LENGTH)
    ax1.set_ylim(0, BLDG_WIDTH)
    
    # ---- 绘制 5GHz 热力图 ----
    plot_floor_plan(ax2)
    cf2 = ax2.contourf(X, Y, sig_5, levels=levels, cmap=cmap, alpha=0.7)
    fig.colorbar(cf2, ax=ax2, label='Signal Strength (dBm)')
    
    # 标记 AP 及 5G 信道
    for ap in aps:
        ax2.plot(ap['x'], ap['y'], 'k^', markersize=8)
        ax2.text(ap['x']+1, ap['y']+1, f"CH:{ap['ch_5']}", fontsize=8, weight='bold')
        
    ax2.set_title('5GHz Band Signal Heatmap & Channel Allocation')
    ax2.set_xlim(0, BLDG_LENGTH)
    ax2.set_ylim(0, BLDG_WIDTH)
    
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()