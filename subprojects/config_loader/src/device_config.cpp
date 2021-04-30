/**
 * @file   device_config.cpp
 * @author  <snk@ws1933linux>
 * @date   Thu Apr 22 16:05:14 2021
 *
 * @brief
 *
 *
 */

#include "device_config.hpp"
#include <atomic>
#include <filesystem>
#include <fstream>
#include <list>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/error/error.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <syslog.h>
#include <utility>
extern std::atomic_bool syslog_crit_enable;
extern std::atomic_bool syslog_warn_enable;
extern std::atomic_bool syslog_info_enable;
extern std::atomic_bool sqllog_crit_enable;
extern std::atomic_bool sqllog_warn_enable;
extern std::atomic_bool sqllog_info_enable;
namespace fs = std::filesystem;
using std::list;
shared_ptr<map<string, shared_ptr<device_config>>>
device_config::parse_device_config_files(const string &config_path) {
  if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
      syslog_info_enable.load())
    openlog("telemetry_server", LOG_PID | LOG_PERROR, LOG_USER);

  fs::path path(config_path);
  path /= "configs";
  path /= "devices";
  if (!fs::exists(path)) {
    syslog(LOG_CRIT, "Отсутсвует директория хранения настроек опрашиваемых "
                     "устройств, дальнейшая загрузка приложения не возможно.");
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  list<string> file_list;
  for (auto iter : fs::directory_iterator(path)) {
    if (iter.path().string().find(".json") != string::npos)
      file_list.push_back(iter.path().string());
  }
  if (file_list.empty()) {
    syslog(LOG_CRIT, "Отсутствуют файлы хранения настроек опрашиваемых "
                     "устройств, дальнейшая загрузка приложения не возможно.");
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
      syslog_info_enable.load())
    closelog();
  shared_ptr<map<string, shared_ptr<device_config>>> result{};

  for (auto iter : file_list) {
    auto buffer = parse_device_config_file(iter);
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      openlog("telemetry_server", LOG_PID | LOG_PERROR, LOG_USER);
    if (!buffer) {
      if (syslog_warn_enable)
        syslog(LOG_WARNING, "Не удалось добавить настройки из файла %s",
               iter.c_str());
      continue;
    }
    if (result->contains(buffer->get_id())) {
      if (syslog_warn_enable)
        syslog(LOG_WARNING,
               "Не удалось добавить настройки из файла %s,настройка с таким "
               "id{%s} уже присутствует.",
               iter.c_str(), buffer->get_id().c_str());
      continue;
    }
    result->insert({buffer->get_id(), buffer});
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
  }
  return result;
}

shared_ptr<device_config>
device_config::parse_device_config_file(const string &config_path) {
  shared_ptr<device_config> result{};
  if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
      syslog_info_enable.load())
    openlog("telemetry_server", LOG_PID | LOG_PERROR, LOG_USER);
  fs::path path(config_path);
  if (!fs::exists(path)) {
    if (syslog_crit_enable.load())
      syslog(LOG_CRIT,
             "Отсутствует конфигурационный файл %s описания опрашиваемых "
             "устройств.",config_path.c_str());
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  rapidjson::Document doc;
  std::fstream ifs(path);
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::ParseResult ok = doc.ParseStream(isw);
  if (!ok) {
    if (syslog_crit_enable.load())
      syslog(LOG_CRIT,
             "Синтаксическая ошибка, %s, в файле %s, описание опрашиваемых "
             "устройств.",
             rapidjson::GetParseError_En(ok.Code()),config_path.c_str());
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  if (!doc.HasMember("device-id")) {
    if (syslog_warn_enable.load())
      syslog(LOG_WARNING,
             "Отсутсвует поле device-id, настройка будет проигнорирована.");
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  if (!doc["device-id"].IsString()) {
    if (syslog_warn_enable.load())
      syslog(LOG_WARNING,
             "Тип поля device-id не верен, настройка будет проигнорирована.");
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  string device_id(doc["device-id"].GetString(),
                   doc["device-id"].GetStringLength());
  if (!doc.HasMember("connection-id")) {
    if (syslog_warn_enable.load())
      syslog(LOG_WARNING,
             "Отсутсвует поле type, настройка будет проигнорирована.");
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  if (!doc["connection-id"].IsString()) {
    if (syslog_warn_enable.load())
      syslog(
          LOG_WARNING,
          "Тип поля connection-id не верен, настройка будет проигнорирована.");
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  string device_connection_id(doc["connection-id"].GetString(),
                              doc["connection-id"].GetStringLength());
  if (!doc.HasMember("model-id")) {
    if (syslog_warn_enable.load())
      syslog(LOG_WARNING,
             "Отсутсвует поле model-id, настройка будет проигнорирована.");
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  if (!doc["model-id"].IsString()) {
    if (syslog_warn_enable.load())
      syslog(LOG_WARNING,
             "Тип поля model-id не верен, настройка будет проигнорирована.");
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  string device_model_id(doc["model-id"].GetString(),
                         doc["model-id"].GetStringLength());
  result.reset(
      new device_config(device_id, device_connection_id, device_model_id));
  if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
      syslog_info_enable.load())
    closelog();

  return result;
}
