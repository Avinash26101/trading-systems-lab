#include "hft/udp_feed.hpp"

#include <iostream>

int main() {
  const hft::MarketUpdate update{1, 100.10, 800, 100.12, 200};
  std::cout << "UDP wire sample: " << hft::encode_update(update) << "\n";
  std::cout << "Run a listener and publisher in separate processes to stream updates over UDP.\n";
  return 0;
}

