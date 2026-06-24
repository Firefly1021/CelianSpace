import torch
import torch.nn as nn
import numpy as np
import matplotlib.pyplot as plt

# 设置随机种子以保证数值实验的可复现性
torch.manual_seed(42)

# ==========================================
# 步骤 2: 构建神经网络替代模型 (Surrogate Model)
# ==========================================
class PINN(nn.Module):
    def __init__(self):
        super().__init__()
        # 核心注意：在 PINN 中求解二阶 PDE，绝对不能使用 ReLU！
        # ReLU 的二阶导数恒为 0，会导致物理残差在 AutoDiff 时直接消失。
        # 必须使用具备无限阶平滑导数的激活函数，如 Tanh, Sigmoid 或 GELU。
        self.net = nn.Sequential(
            nn.Linear(1, 32), nn.Tanh(),
            nn.Linear(32, 32), nn.Tanh(),
            nn.Linear(32, 32), nn.Tanh(),
            nn.Linear(32, 1)
        )

    def forward(self, x):
        return self.net(x)

model = PINN()

# ==========================================
# 步骤 3: 数据采样与配点生成 (Collocation Points)
# ==========================================
N_f = 100 # 域内配点数量

# 1. 域内配点 x_f (必须设置 requires_grad=True，这是 AutoDiff 的前提)
x_f = torch.linspace(-1, 1, N_f).view(-1, 1)
x_f.requires_grad_(True) 

# 2. 边界配点 x_bc 与对应的边界值 u_bc
x_bc = torch.tensor([[-1.0], [1.0]])
u_bc = torch.tensor([[0.0], [0.0]])

# ==========================================
# 步骤 4 & 5: AutoDiff 残差计算与损失函数优化
# ==========================================
# 针对这种一维平滑函数逼近，二阶优化器 L-BFGS 通常比 Adam 表现出更高的收敛精度
optimizer = torch.optim.LBFGS(
    model.parameters(),
    lr=1.0,
    max_iter=1000,
    tolerance_grad=1e-7,
    tolerance_change=1e-9,
    history_size=50
)

def closure():
    optimizer.zero_grad()

    # --- 1. 计算边界损失 (Data/BC Loss) ---
    u_bc_pred = model(x_bc)
    loss_bc = nn.functional.mse_loss(u_bc_pred, u_bc)

    # --- 2. 计算物理残差损失 (PDE Loss) ---
    u_f = model(x_f)
    
    # [AutoDiff 核心] 计算一阶导数 du/dx
    # create_graph=True 允许我们对导数再次求导
    du_dx = torch.autograd.grad(
        outputs=u_f, 
        inputs=x_f,
        grad_outputs=torch.ones_like(u_f),
        create_graph=True,
        retain_graph=True
    )[0]

    # [AutoDiff 核心] 计算二阶导数 d^2u/dx^2
    d2u_dx2 = torch.autograd.grad(
        outputs=du_dx, 
        inputs=x_f,
        grad_outputs=torch.ones_like(du_dx),
        create_graph=True,
        retain_graph=True
    )[0]

    # 组装 PDE 残差: r(x) = d^2u/dx^2 + pi^2 * sin(pi * x)
    # 当网络完美逼近解析解时，residual 应当处处为 0
    residual = d2u_dx2 + (torch.pi ** 2) * torch.sin(torch.pi * x_f)
    loss_f = nn.functional.mse_loss(residual, torch.zeros_like(residual))

    # --- 3. 构造总损失 ---
    loss = loss_bc + loss_f
    loss.backward()
    return loss

print("开始 L-BFGS 闭包优化...")
optimizer.step(closure)
print("优化完成。")

# ==========================================
# 步骤 6: 模型评估与推断 (Evaluation)
# ==========================================
# 生成密集测试网格以评估全局误差
x_test = torch.linspace(-1, 1, 500).view(-1, 1)

with torch.no_grad(): # 推理阶段不再需要计算图
    u_pred = model(x_test)
    u_exact = torch.sin(torch.pi * x_test)
    
    # 严格计算相对 L2 误差向量范数
    error_l2 = torch.norm(u_pred - u_exact, p=2) / torch.norm(u_exact, p=2)
    print(f"Relative L2 Error: {error_l2.item():.4e}")

# 切换到测试模式
model.eval()
with torch.no_grad():
    x_plot = torch.linspace(-1, 1, 200).view(-1, 1)
    u_pred_plot = model(x_plot).numpy()
    u_exact_plot = torch.sin(torch.pi * x_plot).numpy()
    x_plot_nodes = x_plot.numpy()

# 创建画布
fig, ax = plt.subplots(1, 2, figsize=(12, 5))

# 图 1: 预测解 vs 解析解
ax[0].plot(x_plot_nodes, u_exact_plot, 'r-', linewidth=2, label='Exact Solution', alpha=0.6)
ax[0].plot(x_plot_nodes, u_pred_plot, 'b--', linewidth=2, label='PINN Prediction')
ax[0].scatter(x_bc.detach().numpy(), u_bc.detach().numpy(), color='black', marker='x', s=100, label='Boundary Points', zorder=5)
ax[0].set_title('Solution Comparison: $u(x)$')
ax[0].set_xlabel('$x$')
ax[0].set_ylabel('$u$')
ax[0].legend()
ax[0].grid(True, linestyle='--', alpha=0.7)

# 图 2: 绝对误差分布
error = np.abs(u_pred_plot - u_exact_plot)
ax[1].fill_between(x_plot_nodes.flatten(), error.flatten(), color='teal', alpha=0.3)
ax[1].plot(x_plot_nodes, error, color='teal', label='Absolute Error')
ax[1].set_title('Absolute Error: $|u_{pred} - u_{exact}|$')
ax[1].set_xlabel('$x$')
ax[1].set_yscale('log') # 误差通常很小，对数坐标更清晰
ax[1].legend()
ax[1].grid(True, linestyle='--', alpha=0.7)

plt.tight_layout()
plt.show()