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
#include <filesystem>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/error/error.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <syslog.h>
#include <utility>

namespace fs = std::filesystem;

shared_ptr<map<string, shared_ptr<device_config>>>
device_config::parse_device_config_file(const string &config_path) {
  openlog("telemetry_server", LOG_PID | LOG_PERROR, LOG_USER);
  fs::path path(config_path);
  path /= "device_config.json";
  if (!fs::exists(path)) {
    syslog(LOG_CRIT, "Отсутствует конфигурационный файл описания опрашиваемых "
                     "устройств, запуск приложения не возможно.");
    closelog();
    return nullptr;
  }
  rapidjson::Document doc;
  std::fstream ifs(path);
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::ParseResult ok = doc.ParseStream(isw);
  if (!ok) {
    syslog(LOG_CRIT,
           "Синтаксическая ошибка, %s, в файле, описание опрашиваемых "
           "устройств, приложение не может быть запущено.",
           rapidjson::GetParseError_En(ok.Code()));
    closelog();
    return nullptr;
  }
  if (!doc.HasMember("devices")) {
    syslog(LOG_CRIT,
           "В файле, описания настроек опрашиваемых устройств, отсутствует "
           "описание устройств, приложение не может быть запущенно");
    closelog();
    return nullptr;
  }
  auto buffer_map = shared_ptr<map<string, shared_ptr<device_config>>>(
      new map<string, shared_ptr<device_config>>);
  int i = 0;
  for (auto &device : doc["devices"].GetArray()) {
    ++i;
    if (!device.HasMember("device-id")) {
      syslog(LOG_WARNING,
             "Отсутсвует поле device-id, %d настройка будет проигнорирована.",
             i);
      continue;
    }
    if (!device["device-id"].IsString()) {
      syslog(LOG_WARNING,
             "Тип поля device-id не верен, %d настройка будет проигнорирована.",
             i);
      continue;
    }
    string device_id(device["device-id"].GetString(),
                     device["device-id"].GetStringLength());
    if (!device.HasMember("type")) {
      syslog(LOG_WARNING,
             "Отсутсвует поле type, %d настройка будет проигнорирована.", i);
      continue;
    }
    if (!device["connection"].IsString()) {
      syslog(
          LOG_WARNING,
          "Тип поля connection не верен, %d настройка будет проигнорирована.",
          i);
      continue;
    }
    string device_connection(device["connection"].GetString(),
                             device["connection"].GetStringLength());
    if (!device.HasMember("model")) {
      syslog(LOG_WARNING,
             "Отсутсвует поле model, %d настройка будет проигнорирована.", i);
      continue;
    }
    if (!device["model"].IsString()) {
      syslog(LOG_WARNING,
             "Тип поля model не верен, %d настройка будет проигнорирована.", i);
      continue;
    }
    string device_model(device["model"].GetString(),
                        device["model"].GetStringLength());
    if (buffer_map->contains(device_id)) {
      syslog(LOG_WARNING,
             "Настройка %s уже добавлена, наcтрока будет проигнорирована.",
             device_id.c_str());
      continue;
    }
    buffer_map->emplace(std::make_pair(
        device_id, new shared_ptr<device_config>(new device_config(
                       device_id, device_connection, device_model))));
    syslog(LOG_INFO,"Добавлена настройка устройства id = %s",device_id.c_str());
  }
  closelog();
  return buffer_map;
}
