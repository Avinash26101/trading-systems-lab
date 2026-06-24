const metrics = {
  throughput: "generated in CI",
  p99: "generated in CI",
  pnl: "generated in CI"
};

for (const [key, value] of Object.entries(metrics)) {
  const node = document.getElementById(key);
  if (node) node.textContent = value;
}

