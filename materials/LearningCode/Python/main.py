import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torchvision import datasets, transforms
import matplotlib.pyplot as plt
import time
from datetime import datetime
import os

# 1. 确保在 M4 上使用 MPS 加速
device = torch.device("mps" if torch.backends.mps.is_available() else "cpu")
print(f"[{datetime.now()}] 运行环境已初始化，使用设备: {device}")

# 2. 定义简单的测试网络
class Net(nn.Module):
    def __init__(self):
        super(Net, self).__init__()
        self.conv1 = nn.Conv2d(1, 10, kernel_size=5)
        self.conv2 = nn.Conv2d(10, 20, kernel_size=5)
        self.conv2_drop = nn.Dropout2d()
        self.fc1 = nn.Linear(320, 50)
        self.fc2 = nn.Linear(50, 10)

    def forward(self, x):
        x = F.relu(F.max_pool2d(self.conv1(x), 2))
        x = F.relu(F.max_pool2d(self.conv2_drop(self.conv2(x)), 2))
        x = x.view(-1, 320)
        x = F.relu(self.fc1(x))
        x = F.dropout(x, training=self.training)
        x = self.fc2(x)
        return F.log_softmax(x, dim=1)

# 3. FGSM 算法实现
def fgsm_attack(image, epsilon, data_grad):
    sign_data_grad = data_grad.sign()
    perturbed_image = image + epsilon * sign_data_grad
    perturbed_image = torch.clamp(perturbed_image, 0, 1)
    return perturbed_image

# 4. PGD 算法实现
def pgd_attack(model, images, labels, epsilon, alpha=0.01, iters=10):
    images = images.clone().detach().to(device)
    labels = labels.to(device)
    loss = nn.CrossEntropyLoss()
    ori_images = images.data
    
    for i in range(iters):
        images.requires_grad = True
        outputs = model(images)
        model.zero_grad()
        cost = loss(outputs, labels).to(device)
        cost.backward()
        
        adv_images = images + alpha * images.grad.sign()
        eta = torch.clamp(adv_images - ori_images, min=-epsilon, max=epsilon)
        images = torch.clamp(ori_images + eta, min=0, max=1).detach_()
            
    return images

# 5. 测试循环
def test_model(model, test_loader, epsilon, attack_type="FGSM"):
    correct = 0
    start_time = time.time()
    
    for data, target in test_loader:
        data, target = data.to(device), target.to(device)
        data.requires_grad = True
        output = model(data)
        init_pred = output.max(1, keepdim=True)[1]
        
        if init_pred[0].item() != target.item():
            continue
            
        loss = F.nll_loss(output, target)
        model.zero_grad()
        loss.backward()
        
        if attack_type == "FGSM":
            data_grad = data.grad.data
            perturbed_data = fgsm_attack(data, epsilon, data_grad)
        else:
            perturbed_data = pgd_attack(model, data, target, epsilon)
            
        output = model(perturbed_data)
        final_pred = output.max(1, keepdim=True)[1]
        if final_pred.item() == target.item():
            correct += 1

    end_time = time.time()
    acc = correct / float(len(test_loader))
    time_cost = end_time - start_time
    print(f"[{datetime.now()}] {attack_type} | Epsilon: {epsilon} | 准确率: {acc:.4f} | 耗时: {time_cost:.2f}s")
    return acc, time_cost

if __name__ == '__main__':
    # 数据加载
    test_loader = torch.utils.data.DataLoader(
        datasets.MNIST('../data', train=False, download=True, transform=transforms.Compose([
            transforms.ToTensor(),
        ])), batch_size=1, shuffle=True) # Batch=1 方便单样本梯度计算

    model = Net().to(device)
    # 为了演示，直接加载预训练模型或快速训练一轮 (这里为确保完整运行，快速训练 1000 个 batch)
    print(f"[{datetime.now()}] 快速训练模型作为靶标...")
    optimizer = optim.SGD(model.parameters(), lr=0.01, momentum=0.5)
    train_loader = torch.utils.data.DataLoader(datasets.MNIST('../data', train=True, download=True, transform=transforms.ToTensor()), batch_size=64, shuffle=True)
    model.train()
    for batch_idx, (data, target) in enumerate(train_loader):
        if batch_idx > 100: break
        data, target = data.to(device), target.to(device)
        optimizer.zero_grad()
        output = model(data)
        loss = F.nll_loss(output, target)
        loss.backward()
        optimizer.step()
    print(f"[{datetime.now()}] 模型准备完毕。")

    model.eval()
    epsilons = [0, .05, .1, .15, .2]
    
    fgsm_accs, pgd_accs = [], []
    fgsm_times, pgd_times = 0, 0
    
    print(f"[{datetime.now()}] 开始对抗攻击测试...")
    # 为了测试速度，只取测试集前 500 个样本
    subset_indices = list(range(500))
    subset_test_loader = torch.utils.data.DataLoader(
        torch.utils.data.Subset(test_loader.dataset, subset_indices), batch_size=1, shuffle=False)

    for eps in epsilons:
        f_acc, f_time = test_model(model, subset_test_loader, eps, "FGSM")
        p_acc, p_time = test_model(model, subset_test_loader, eps, "PGD")
        fgsm_accs.append(f_acc)
        pgd_accs.append(p_acc)
        if eps == 0.1: # 记录特征点的耗时用于画柱状图
            fgsm_times = f_time
            pgd_times = p_time

    # 6. 画图：折线图 (准确率变化)
    plt.figure(figsize=(12, 5))
    plt.subplot(1, 2, 1)
    plt.plot(epsilons, fgsm_accs, "*-", label="FGSM")
    plt.plot(epsilons, pgd_accs, "o-", label="PGD")
    plt.title("Accuracy vs Epsilon (M4 Chip)")
    plt.xlabel("Epsilon")
    plt.ylabel("Accuracy")
    plt.legend()
    plt.grid(True)

    # 画图：柱状图 (耗时对比)
    plt.subplot(1, 2, 2)
    plt.bar(["FGSM", "PGD"], [fgsm_times, pgd_times], color=['#1f77b4', '#ff7f0e'])
    plt.title("Time Cost Comparison (Eps=0.1)")
    plt.ylabel("Time (seconds)")
    
    plt.tight_layout()
    plt.savefig("attack_results.png")
    print(f"[{datetime.now()}] 测试完成，结果已保存为 attack_results.png")