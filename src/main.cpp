/**
 * @file   main.cpp
 * @author  <snk@ws1933linux>
 * @date   Thu Apr 22 09:25:05 2021
 *
 * @brief
 *
 *
 */



#include "config_loader.hpp"
#include <iostream>
#include <memory>
#include <atomic>

std::atomic_bool syslog_crit_enable(true);
std::atomic_bool syslog_warn_enable(true);
std::atomic_bool syslog_info_enable(true);
std::atomic_bool sqllog_crit_enable(true);
std::atomic_bool sqllog_warn_enable(true);
std::atomic_bool sqllog_info_enable(true);
using std::shared_ptr;
int main(int argc, char **argv) {
  auto config_load = shared_ptr<config_loader>(new config_loader);
  if (argc > 1)
    config_load->load_configs(argv[1]);
  else
    config_load->load_configs();
  return 0;
}
