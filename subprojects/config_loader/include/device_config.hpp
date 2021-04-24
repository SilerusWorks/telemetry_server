/**
 * @file   device_config.hpp
 * @author  <snk@ws1933linux>
 * @date   Thu Apr 22 16:05:19 2021
 *
 * @brief
 *
 *
 */

#ifndef DEVICE_CONFIG_HPP
#define DEVICE_CONFIG_HPP
#include <map>
#include <memory>
#include <string>
#include <utility>

using std::map;
using std::shared_ptr;
using std::string;
using std::pair;
class device_config {
public:
  device_config(const string &device_id, const string &device_connection,
                const string &device_model)
      : id(device_id), connection(device_connection), model(device_model){};
  device_config(){};
  ~device_config(){};
  void set_id(const string &value) { id = value; }
  const string &get_id() const { return id; }
  void set_connection(const string &value) { connection = value; }
  const string &get_connection() const { return connection; }
  void set_model(const string &value) { model = value; }
  const string &get_model() const { return model; }
  static shared_ptr<map<string, shared_ptr<device_config>>>
  parse_device_config_files(const string &config_path);

private:
  string id{};
  string connection{};
  string model{};
  static pair<string,shared_ptr<device_config>> parse_device_config_file(const string &config_path);
};
#endif // DEVICE_CONFIG_HPP
