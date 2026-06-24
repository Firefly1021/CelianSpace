#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <string>

// 物理参数常量
const double PI = 3.14159265358979323846;
const double A_WAVE = 1.0;      // 波速 a
const double T_END = 0.5;       // 目标时间
const double X_L = 0.0;         // 空间左边界
const double X_R = 1.0;         // 空间右边界

// 绝对光滑的精确解：行波 u(x,t) = sin(2*pi*(x - a*t))
double exact_solution(double x, double t) {
    return std::sin(2.0 * PI * (x - A_WAVE * t));
}

int main() {
    // 每次翻倍的网格节点数
    std::vector<int> N_values = {40, 80, 160, 320, 640};
    double nu = 0.5; // 固定库朗数 (确保在 0 <= nu <= 2 的稳定域内)

    // 用于保存上一个网格层的误差，以计算收敛阶
    double prev_L2 = 0.0;
    double prev_Linf = 0.0;

    // 打印表头
    std::cout << std::string(80, '-') << "\n";
    std::cout << std::setw(5) << "N" 
              << std::setw(15) << "L2 Error" 
              << std::setw(15) << "L2 Order" 
              << std::setw(15) << "Linf Error" 
              << std::setw(15) << "Linf Order\n";
    std::cout << std::string(80, '-') << "\n";

    for (int N : N_values) {
        double dx = (X_R - X_L) / N;
        double dt = nu * dx / A_WAVE;
        int num_steps = std::round(T_END / dt);

        // 状态数组 (大小 N+1 涵盖边界)
        std::vector<double> u(N + 1, 0.0);
        std::vector<double> u_new(N + 1, 0.0);

        // 1. 初始化 t=0
        for (int j = 0; j <= N; ++j) {
            u[j] = exact_solution(X_L + j * dx, 0.0);
        }

        // 预计算差分系数
        double c_0  = 0.5 * (nu - 1.0) * (nu - 2.0);
        double c_m1 = nu * (2.0 - nu);
        double c_m2 = 0.5 * nu * (nu - 1.0);

        // 2. 时间推进循环
        for (int n = 0; n < num_steps; ++n) {
            double t_old = n * dt;

            // 提取左侧的虚拟节点 (Ghost Nodes)
            // 在精确的收敛测试中，虚拟节点的值由精确解在域外的值直接提供
            double u_ghost_m1 = exact_solution(X_L - dx, t_old);
            double u_ghost_m2 = exact_solution(X_L - 2.0 * dx, t_old);

            // 空间节点循环 (这里可以通过 OpenMP 并行)
            for (int j = 0; j <= N; ++j) {
                // 安全获取 j-1 和 j-2 的值
                double val_m1 = (j >= 1) ? u[j-1] : u_ghost_m1;
                double val_m2 = (j >= 2) ? u[j-2] : ((j == 1) ? u_ghost_m1 : u_ghost_m2);

                u_new[j] = c_0 * u[j] + c_m1 * val_m1 + c_m2 * val_m2;
            }
            
            // 数组交换 (避免拷贝)
            std::swap(u, u_new);
        }

        // 3. 计算误差
        double L2_error = 0.0;
        double Linf_error = 0.0;

        for (int j = 0; j <= N; ++j) {
            double exact_val = exact_solution(X_L + j * dx, T_END);
            double diff = std::abs(u[j] - exact_val);
            
            L2_error += diff * diff * dx;        // L2 积分求和
            Linf_error = std::max(Linf_error, diff); // 最大值范数
        }
        L2_error = std::sqrt(L2_error);

        // 4. 计算收敛阶 EOC = log2(E_old / E_new)
        double order_L2 = 0.0;
        double order_Linf = 0.0;
        if (prev_L2 > 0.0) {
            order_L2 = std::log2(prev_L2 / L2_error);
            order_Linf = std::log2(prev_Linf / Linf_error);
        }

        // 5. 格式化输出
        std::cout << std::setw(5) << N 
                  << std::scientific << std::setprecision(4) 
                  << std::setw(15) << L2_error 
                  << std::fixed << std::setprecision(4) 
                  << std::setw(15) << (prev_L2 > 0 ? std::to_string(order_L2) : "-") 
                  << std::scientific << std::setprecision(4) 
                  << std::setw(15) << Linf_error 
                  << std::fixed << std::setprecision(4) 
                  << std::setw(15) << (prev_Linf > 0 ? std::to_string(order_Linf) : "-") 
                  << "\n";

        // 更新历史误差记录
        prev_L2 = L2_error;
        prev_Linf = Linf_error;
    }
    std::cout << std::string(80, '-') << "\n";

    return 0;
}