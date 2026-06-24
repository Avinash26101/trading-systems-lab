#pragma once

#include "hft/types.hpp"

#include <string>

namespace hft {

std::string encode_update(const MarketUpdate& update);
MarketUpdate decode_update(const std::string& wire);

class UdpFeedPublisher {
 public:
  bool publish(const std::string& host, int port, const MarketUpdate& update) const;
};

class UdpFeedListener {
 public:
  bool receive_once(int port, MarketUpdate& update, int timeout_ms = 1000) const;
};

} // namespace hft

