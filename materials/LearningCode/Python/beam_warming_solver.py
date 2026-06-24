import numpy as np
import matplotlib.pyplot as plt

def beam_warming_solver():
    a = 1.0
    t_end = 0.5
    x_min, x_max = -1.0, 1.0
    Nx = 200
    dx = (x_max - x_min) / Nx
    initial_cfl = 0.8
    cfl = initial_cfl
    dt = cfl * dx / a

    x = np.linspace(x_min + dx/2, x_max - dx/2, Nx)
    u = np.where(x <= 0.0, 1.0, 0.0)
    
    t = 0.0
    while t < t_end:
        if t + dt > t_end:
            dt = t_end - t
            cfl = a * dt / dx
        u_new = np.zeros_like(u)
        u_jm1 = np.concatenate(([1.0], u[:-1]))
        u_jm2 = np.concatenate(([1.0, 1.0], u[:-2]))
        u_new = u - 0.5 * cfl * (3*u - 4*u_jm1 + u_jm2) \
                  + 0.5 * cfl**2 * (u - 2*u_jm1 + u_jm2)
        u = u_new
        t += dt

    plt.figure(figsize=(8, 5))
    plt.plot(x, u, 'b-', label=f'Numerical (Beam-Warming, CFL={initial_cfl:.1f})')
    u_exact = np.where(x <= 0.5, 1.0, 0.0)
    plt.plot(x, u_exact, 'r--', label='Exact Solution')
    plt.title(f'Beam-Warming Method solving Linear Advection at t={t_end}')
    plt.xlabel('x')
    plt.ylabel('u(x,t)')
    plt.xlim(-1, 1)
    plt.ylim(-0.2, 1.4)
    plt.grid(True, linestyle=':')
    plt.legend()
    plt.tight_layout()
    plt.savefig("beam_warming_result.png", dpi=300)

if __name__ == "__main__":
    beam_warming_solver()