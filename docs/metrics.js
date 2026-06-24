const metrics = {
  throughput: "see CI benchmark artifact",
  p99: "see CI benchmark artifact",
  pnl: "$-0.42",
  updates: "16",
  orders: "16",
  fills: "16",
  riskRejects: "0",
  position: "0"
};

for (const [key, value] of Object.entries(metrics)) {
  const node = document.getElementById(key);
  if (node) node.textContent = value;
}
