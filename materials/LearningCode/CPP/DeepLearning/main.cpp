#include <torch/torch.h>
#include <iostream>

int main() {
    std::cout << "PyTorch 版本: " 
              << TORCH_VERSION_MAJOR << "." 
              << TORCH_VERSION_MINOR << "." 
              << TORCH_VERSION_PATCH << std::endl;

    // --- 1. 设备选择逻辑 (适配 M4 和 Windows NVIDIA) ---
    torch::Device device(torch::kCPU);

    if (torch::cuda::is_available()) {
        std::cout << "检测到运行环境: Windows/Linux + NVIDIA GPU (CUDA)" << std::endl;
        device = torch::Device(torch::kCUDA);
    } 
    // 注意：在 2.1.x 版本，使用 torch::mps::is_available() 是标准做法
    else if (torch::mps::is_available()) {
        std::cout << "检测到运行环境: Mac M4 + Apple Silicon GPU (MPS)" << std::endl;
        device = torch::Device(torch::kMPS);
    } 
    else {
        std::cout << "检测到运行环境: 仅 CPU 模式" << std::endl;
    }

    try {
        // --- 2. 创建一个线性模型 ---
        // 输入特征 5，输出特征 2
        torch::nn::Linear model(5, 2);
        
        // 将模型移动到指定设备 (M4 会移动到 GPU 核心)
        model->to(device);

        // --- 3. 构造数据并进行计算 ---
        // 创建一个随机输入 [BatchSize=1, Features=5]
        // 注意：创建张量时直接指定 device 和数据类型
        auto input = torch::randn({1, 5}, torch::TensorOptions().device(device).dtype(torch::kFloat32));

        // 前向传播
        auto output = model->forward(input);

        // --- 4. 打印结果 ---
        std::cout << "\n输入张量 (" << device << "):\n" << input << std::endl;
        std::cout << "模型输出 (" << device << "):\n" << output << std::endl;

        // 计算一个简单的损失（测试反向传播路径是否打通）
        auto target = torch::zeros({1, 2}, device);
        auto loss = torch::mse_loss(output, target);
        std::cout << "初始损失值: " << loss.item<float>() << std::endl;

    } catch (const c10::Error& e) {
        std::cerr << "运行出错: " << e.msg() << std::endl;
        return -1;
    }

    return 0;
}