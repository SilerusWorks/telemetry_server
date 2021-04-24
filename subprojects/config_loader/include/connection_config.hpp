/**
 * @file   connection_config.hpp
 * @author  <snk@ws1933linux>
 * @date   Sat Apr 24 11:27:44 2021
 *
 * @brief
 *
 *
 */

#ifndef CONNECTION_CONFIG_HPP
#define CONNECTION_CONFIG_HPP

#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>

using std::list;
using std::map;
using std::shared_ptr;
using std::string;
using std::pair;

enum class enum_role { master, slave };
enum class enum_type { tcp, udp, uart };
enum class enum_flow_control { none, software, hardware };
enum class enum_parity { none, odd, even };
enum class enum_stop_bits { one, one_pont_five, two };
enum class enum_data_bits { five, six, seven, eight, nine };
class connection_config {
public:
  connection_config(const string &conn_id, const enum_role &conn_role,
                    const enum_type &conn_type, const string &conn_address,
                    const string &conn_port,
                    const std::size_t &conn_await_time = 0)
      : id(conn_id), role(conn_role), type(conn_type),
        tcp_udp_address(conn_address), tcp_udp_port(conn_port),
        await_time(conn_await_time) {}
  connection_config(const string &conn_id, const enum_role &conn_role,
                    const enum_type &conn_type,
                    const list<string> &conn_listen_addresses,
                    const string &conn_port,
                    const std::size_t &conn_await_time = 0)
      : id(conn_id), role(conn_role), type(conn_type), tcp_udp_port(conn_port),
        tcp_udp_listen_addresses(conn_listen_addresses),
        await_time(conn_await_time) {}
  connection_config(
      const string &conn_id, const enum_role &conn_role,
      const enum_type &conn_type, const string &conn_port,
      const std::size_t conn_baud_rate = 19200,
      const enum_flow_control &conn_flow_control = enum_flow_control::none,
      const enum_parity &conn_parity = enum_parity::none,
      const enum_stop_bits &conn_stop_bits = enum_stop_bits::one,
      const enum_data_bits &conn_data_bits = enum_data_bits::eight,
      const std::size_t &conn_await_time = 0)
      : id(conn_id), role(conn_role), type(conn_type),
        baud_rate(conn_baud_rate), flow_control(conn_flow_control),
        parity(conn_parity), stop_bits(conn_stop_bits),
        data_bits(conn_data_bits), uart_port(conn_port),
        await_time(conn_await_time){};
  ~connection_config() {}

  void set_id(const string &value) { id = value; }
  string get_id() const { return id; }
  void set_role(const enum_role &value = enum_role::master) { role = value; }
  enum_role get_role() const { return role; };
  void set_type(const enum_type &value = enum_type::tcp) { type = value; }
  enum_type get_type() const { return type; }
  void set_baud_rate(const std::size_t &value) { baud_rate = value; }
  std::size_t get_baud_rate() const { return baud_rate; };
  void set_flow_control(const enum_flow_control &value) {
    flow_control = value;
  }
  enum_flow_control get_flow_control() const { return flow_control; }
  void set_parity(const enum_parity &value = enum_parity::none) {
    parity = value;
  }
  void set_stop_bits(const enum_stop_bits &value = enum_stop_bits::one) {
    stop_bits = value;
  }
  enum_stop_bits get_stop_bits() const { return stop_bits; }
  void set_data_bits(const enum_data_bits &value) { data_bits = value; }
  enum_data_bits get_data_bits() const { return data_bits; }
  void set_tcp_udp_address(const string &value = "127.0.0.1") {
    tcp_udp_address = value;
  }
  string get_tcp_udp_address() const { return tcp_udp_address; }
  void set_tcp_udp_port(const string &value = "520") { tcp_udp_port = value; }
  string get_tcp_udp_port() const { return tcp_udp_port; }
  void set_tcp_udp_listen_addresses(const list<string> &value = {"127.0.0.1"}) {
    tcp_udp_listen_addresses = value;
  }
  list<string> get_tcp_udp_listen_addresses() const {
    return tcp_udp_listen_addresses;
  }
  void set_uart_port(const string &value = "/dev/ttyS0") { uart_port = value; }
  string get_uart_port() const { return uart_port; }
  void set_await_time(const std::size_t &value = 30) { await_time = value; }
  std::size_t get_await_time() const { return await_time; };

  static shared_ptr<map<string,shared_ptr<connection_config>>> parse_connection_config_files(const string &general_config_path);

private:
  string id{};
  enum_role role{enum_role::master};
  enum_type type{enum_type::tcp};
  std::size_t baud_rate{19200};
  enum_flow_control flow_control{enum_flow_control::none};
  enum_parity parity{enum_parity::none};
  enum_stop_bits stop_bits{enum_stop_bits::one};
  enum_data_bits data_bits{enum_data_bits::eight};
  string tcp_udp_address{"127.0.0.1"};
  string tcp_udp_port{"520"};
  list<string> tcp_udp_listen_addresses;
  string uart_port{};
  std::size_t await_time{30};
  static pair<string,shared_ptr<connection_config>> parse_connection_config_file(const string &config_path);
};
#endif // CONNECTION_CONFIG_HPP
