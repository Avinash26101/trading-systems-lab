#include "hft/udp_feed.hpp"

#include <sstream>
#include <stdexcept>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace hft {

namespace {

#if defined(_WIN32)
struct SocketRuntime {
  SocketRuntime() {
    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);
  }
  ~SocketRuntime() { WSACleanup(); }
};

void close_socket(SOCKET socket_handle) {
  closesocket(socket_handle);
}
#else
using SOCKET = int;
const int INVALID_SOCKET = -1;
const int SOCKET_ERROR = -1;

struct SocketRuntime {};

void close_socket(SOCKET socket_handle) {
  close(socket_handle);
}
#endif

} // namespace

std::string encode_update(const MarketUpdate& update) {
  std::ostringstream out;
  out << update.sequence << ','
      << update.bid_price << ','
      << update.bid_quantity << ','
      << update.ask_price << ','
      << update.ask_quantity;
  return out.str();
}

MarketUpdate decode_update(const std::string& wire) {
  std::stringstream ss(wire);
  std::string cell;
  MarketUpdate update;
  std::getline(ss, cell, ',');
  update.sequence = static_cast<std::uint64_t>(std::stoull(cell));
  std::getline(ss, cell, ',');
  update.bid_price = std::stod(cell);
  std::getline(ss, cell, ',');
  update.bid_quantity = std::stoi(cell);
  std::getline(ss, cell, ',');
  update.ask_price = std::stod(cell);
  std::getline(ss, cell, ',');
  update.ask_quantity = std::stoi(cell);
  return update;
}

bool UdpFeedPublisher::publish(const std::string& host, int port, const MarketUpdate& update) const {
  SocketRuntime runtime;
  SOCKET socket_handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (socket_handle == INVALID_SOCKET) return false;

  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(static_cast<unsigned short>(port));
  address.sin_addr.s_addr = inet_addr(host.c_str());

  const std::string payload = encode_update(update);
  const int sent = sendto(socket_handle, payload.c_str(), static_cast<int>(payload.size()), 0,
                          reinterpret_cast<sockaddr*>(&address), sizeof(address));
  close_socket(socket_handle);
  return sent != SOCKET_ERROR;
}

bool UdpFeedListener::receive_once(int port, MarketUpdate& update, int timeout_ms) const {
  SocketRuntime runtime;
  SOCKET socket_handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (socket_handle == INVALID_SOCKET) return false;

  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(static_cast<unsigned short>(port));

  if (bind(socket_handle, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
    close_socket(socket_handle);
    return false;
  }

  fd_set read_set;
  FD_ZERO(&read_set);
  FD_SET(socket_handle, &read_set);
  timeval timeout;
  timeout.tv_sec = timeout_ms / 1000;
  timeout.tv_usec = (timeout_ms % 1000) * 1000;
  const int ready = select(static_cast<int>(socket_handle + 1), &read_set, nullptr, nullptr, &timeout);
  if (ready <= 0) {
    close_socket(socket_handle);
    return false;
  }

  char buffer[256] = {0};
  const int received = recvfrom(socket_handle, buffer, sizeof(buffer) - 1, 0, nullptr, nullptr);
  close_socket(socket_handle);
  if (received <= 0) return false;
  update = decode_update(std::string(buffer, static_cast<std::size_t>(received)));
  return true;
}

} // namespace hft

