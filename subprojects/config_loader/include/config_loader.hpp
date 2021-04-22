/**
 * @file   config_loader.hpp
 * @author  <snk@ws1933linux>
 * @date   Thu Apr 22 09:24:39 2021
 * 
 * @brief  
 * 
 * 
 */

#ifndef CONFIG_LOADER_HPP
#define CONFIG_LOADER_HPP
#include <string>
#include <map>

using std::string;
using std::map;   
class config_loader{
public:
  config_loader(){};
  ~config_loader(){};
  void load_configs(const string &general_string=string());
private:
  string general_config_path{};
  string base_address{"127.0.0.1"};
  string base_port{"5432"};
  string base_login{"postgres"};
  string base_password{};
  string base_db_name{"telemetry_server"};
  string polled_devices_config_path{};
  
};

#endif //CONFIG_LOADER_HPP
