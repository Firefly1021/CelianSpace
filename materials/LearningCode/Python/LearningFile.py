import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.tri as mtri

conv = pd.read_csv("convergence.csv")
print(conv)

plt.figure()
plt.loglog(conv["h"], conv["L2_error"], marker="o", label=r"$L^2$ error")
plt.loglog(conv["h"], conv["curl_error"], marker="s", label=r"curl error")
plt.loglog(conv["h"], conv["Hcurl_error"], marker="^", label=r"$H(\mathrm{curl})$ error")
plt.gca().invert_xaxis()
plt.xlabel("mesh size h")
plt.ylabel("error")
plt.title("Lowest-order Nedelec FEM convergence")
plt.grid(True, which="both")
plt.legend()
plt.tight_layout()
plt.savefig("convergence.png", dpi=300)

plt.figure()
plt.plot(conv["h"], conv["L2_rate"], marker="o", label=r"$L^2$ rate")
plt.plot(conv["h"], conv["curl_rate"], marker="s", label="curl rate")
plt.plot(conv["h"], conv["Hcurl_rate"], marker="^", label=r"$H(\mathrm{curl})$ rate")
plt.gca().invert_xaxis()
plt.xlabel("mesh size h")
plt.ylabel("observed order")
plt.title("Observed convergence order")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("rates.png", dpi=300)

try:
    sol = pd.read_csv("nedelec2d_solution.csv")
    tri = mtri.Triangulation(sol["x"], sol["y"])

    plt.figure()
    plt.tricontourf(tri, sol["Ex"], levels=20)
    plt.colorbar(label="computed Ex")
    plt.xlabel("x")
    plt.ylabel("y")
    plt.title("Computed Ex at element centroids")
    plt.tight_layout()
    plt.savefig("solution_Ex.png", dpi=300)

    plt.figure()
    plt.quiver(sol["x"], sol["y"], sol["Ex"], sol["Ey"], scale=25)
    plt.xlabel("x")
    plt.ylabel("y")
    plt.title("Computed electric field samples")
    plt.axis("equal")
    plt.tight_layout()
    plt.savefig("solution_quiver.png", dpi=300)
except FileNotFoundError:
    print("nedelec2d_solution.csv not found; skipped solution plots.")

print("Saved: convergence.png, rates.png, solution_Ex.png, solution_quiver.png")
