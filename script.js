const progress = document.querySelector(".progress");
const filterButtons = [...document.querySelectorAll(".filter-button")];
const projectCards = [...document.querySelectorAll(".project-card")];
const canvas = document.querySelector("#lab-canvas");
const pauseButton = document.querySelector("#pause-lab");

function updateProgress() {
  const scrollable = document.documentElement.scrollHeight - window.innerHeight;
  const value = scrollable > 0 ? (window.scrollY / scrollable) * 100 : 0;
  progress.style.width = `${Math.min(100, Math.max(0, value))}%`;
}

filterButtons.forEach((button) => {
  button.addEventListener("click", () => {
    const filter = button.dataset.filter;

    filterButtons.forEach((item) => item.classList.toggle("active", item === button));
    projectCards.forEach((card) => {
      const kinds = card.dataset.kind.split(" ");
      card.classList.toggle("hidden", filter !== "all" && !kinds.includes(filter));
    });
  });
});

window.addEventListener("scroll", updateProgress, { passive: true });
updateProgress();

const context = canvas.getContext("2d");
let paused = false;
let frame = 0;

function resizeCanvas() {
  const ratio = Math.min(window.devicePixelRatio || 1, 2);
  const rect = canvas.getBoundingClientRect();
  canvas.width = Math.floor(rect.width * ratio);
  canvas.height = Math.floor(rect.height * ratio);
  context.setTransform(ratio, 0, 0, ratio, 0, 0);
}

function drawField() {
  if (!paused) frame += 0.012;

  const width = canvas.clientWidth;
  const height = canvas.clientHeight;
  context.clearRect(0, 0, width, height);

  const gradient = context.createRadialGradient(width * 0.5, height * 0.5, 0, width * 0.5, height * 0.5, width * 0.7);
  gradient.addColorStop(0, "rgba(118, 240, 223, 0.12)");
  gradient.addColorStop(1, "rgba(3, 8, 11, 0.96)");
  context.fillStyle = gradient;
  context.fillRect(0, 0, width, height);

  context.strokeStyle = "rgba(174, 230, 221, 0.09)";
  context.lineWidth = 1;
  for (let x = 0; x < width; x += 42) {
    context.beginPath();
    context.moveTo(x, 0);
    context.lineTo(x, height);
    context.stroke();
  }
  for (let y = 0; y < height; y += 42) {
    context.beginPath();
    context.moveTo(0, y);
    context.lineTo(width, y);
    context.stroke();
  }

  const points = [];
  const count = 78;
  for (let index = 0; index < count; index += 1) {
    const t = index / (count - 1);
    const angle = t * Math.PI * 7 + frame;
    const radius = 42 + t * Math.min(width, height) * 0.42;
    const x = width / 2 + Math.cos(angle) * radius * Math.cos(frame * 0.7 + t);
    const y = height / 2 + Math.sin(angle * 0.82) * radius * 0.58;
    points.push({ x, y });
  }

  context.strokeStyle = "rgba(118, 240, 223, 0.62)";
  context.lineWidth = 1.4;
  context.beginPath();
  points.forEach((point, index) => {
    if (index === 0) context.moveTo(point.x, point.y);
    else context.lineTo(point.x, point.y);
  });
  context.stroke();

  points.forEach((point, index) => {
    const glow = index % 9 === 0;
    context.fillStyle = glow ? "rgba(226, 139, 196, 0.9)" : "rgba(215, 202, 169, 0.76)";
    context.beginPath();
    context.arc(point.x, point.y, glow ? 2.7 : 1.8, 0, Math.PI * 2);
    context.fill();
  });

  context.fillStyle = "rgba(240, 247, 244, 0.72)";
  context.font = "12px Cascadia Code, Consolas, monospace";
  context.fillText("x(t) = cos(7t + phi) * r", 24, 34);
  context.fillText("y(t) = sin(0.82t) * r", 24, 56);

  requestAnimationFrame(drawField);
}

pauseButton.addEventListener("click", () => {
  paused = !paused;
  pauseButton.textContent = paused ? "Resume Motion" : "Pause Motion";
});

window.addEventListener("resize", resizeCanvas);
resizeCanvas();
drawField();
