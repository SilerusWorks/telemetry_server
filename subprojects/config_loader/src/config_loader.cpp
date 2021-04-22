/**
 * @file   config_loader.cpp
 * @author  <snk@ws1933linux>
 * @date   Thu Apr 22 09:24:48 2021
 *
 * @brief
 *
 *
 */

#include "config_loader.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/error/error.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <sstream>
#include <syslog.h>
#include <iostream>

namespace fs = std::filesystem;

void config_loader::load_configs(const string &general_config) {
  openlog("telemetry_server", LOG_PID | LOG_PERROR, LOG_USER);
  if (!general_config.empty())
    if (fs::exists(general_config))
      general_config_path = general_config;
  if (general_config_path.empty()) {
    auto user_home_dir = std::getenv("HOME");
    if (user_home_dir) {
      fs::path config_path(user_home_dir);
      config_path /= ".config";
      config_path /= "telemetry_server";
      config_path /= "general_config.json";
      if (fs::exists(config_path))
        general_config_path = config_path.string();
    }
  }
  if (general_config_path.empty()) {
    if (fs::exists("/etc/telemetry_server/general_config.json"))
      general_config_path = "/etc/telemetry_server/general_config.json";
  }
  if (general_config_path.empty()) {
    syslog(LOG_CRIT, "Отсутствует основной конфигурационный файл, запуск "
                     "приложения не возможно.");
    closelog();
    exit(EXIT_FAILURE);
  }
  rapidjson::Document doc;
  std::fstream ifs(general_config_path);
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::ParseResult ok = doc.ParseStream(isw);
  if (!ok) {
    syslog(LOG_CRIT,
           "Синтаксическая ошибка, %s, в основном конфигурационном файле, "
           "приложение не может быть запущено.",
           rapidjson::GetParseError_En(ok.Code()));
    closelog();
    exit(EXIT_FAILURE);
  }
  if (doc.HasMember("sql-base-address")) {
    if (doc["sql-base-address"].IsString())
      base_address.assign(doc["sql-base-address"].GetString(),
                          doc["sql-base-address"].GetStringLength());
    else
      syslog(LOG_WARNING, "Не верный тип поля адреса подключения к базе "
                          "данных, установлен адрес: 127.0.0.1.");
  } else {
    syslog(LOG_WARNING, "Не указан адрес подключения к базе данных, установлен "
                        "адрес: 127.0.0.1.");
  }
  if (doc.HasMember("sql-base-port")) {
    if (doc["sql-base-port"].IsString()) {
      base_port.assign(doc["sql-base-port"].GetString(),
                       doc["sql-base-port"].GetStringLength());
    } else {
      if (doc["sql-base-port"].IsUint()) {
        std::stringstream ss;
        ss << doc["sql-base-port"].GetUint();
        base_port.assign(ss.str());
      } else {
        syslog(LOG_WARNING,
               "Не верный порт подключения к базе данных, установлен "
               "порт: 5432.");
      }
    }
  } else {
    syslog(LOG_WARNING, "Не указан порт подключения к базе данных, установлен "
                        "порт: 5432.");
  }

  if (doc.HasMember("sql-base-db-name")) {
    if (doc["sql-base-db-name"].IsString())
      base_db_name.assign(doc["sql-base-db-name"].GetString(),
                          doc["sql-base-db-name"].GetStringLength());
    else
      syslog(LOG_WARNING, "Не верный тип поля имени базы данных, установлено "
                          "имя: telemetry_server.");
  } else {
    syslog(LOG_WARNING, "Не указан имя базы данных, установлено "
                        "имя: telemetry_server.");
  }

  if (doc.HasMember("sql-base-login")) {
    if (doc["sql-base-login"].IsString())
      base_login.assign(doc["sql-base-login"].GetString(),
                        doc["sql-base-login"].GetStringLength());
    else
      syslog(LOG_WARNING, "Не верный тип поля имени пользователя базы данных, "
                          "установлено имя пользователя: postgres.");
  } else {
    syslog(LOG_WARNING, "Не указано имя пользователя базы данных, установлено "
                        "имя пользователя: postgres.");
  }

  if (doc.HasMember("sql-base-password")) {
    if (doc["sql-base-password"].IsString())
      base_password.assign(doc["sql-base-password"].GetString(),
                           doc["sql-base-password"].GetStringLength());
    else
      syslog(LOG_WARNING, "Не верный тип поля пароля пользователя базы данных, "
                          "установлен пароль пользователя: пустой.");
  } else {
    syslog(LOG_WARNING,
           "Не указано пароль пользователя базы данных, установлен "
           "пароль пользователя: пустой.");
  }

  if (doc.HasMember("polled-device-config-path")) {
    if (doc["polled-device-config-path"].IsString())
      polled_devices_config_path.assign(
          doc["polled-device-config-path"].GetString(),
          doc["polled-device-config-path"].GetStringLength());
    else {
      syslog(LOG_CRIT,
             "Не верный тип поля, хранящего путь к директории хранения "
             "настроек опроса устройств, запуск приложения невозможено.");
      exit(EXIT_FAILURE);
    }
  } else {
    syslog(LOG_WARNING,
           "Не указан путь к директории хранения настроек опроса устройств, запуск приложения невозможено.");
    exit(EXIT_FAILURE);
  }
  std::cout<<base_address<<" "<<base_port<<" "<<base_db_name<<" "<<base_login<<" "<<base_password<<" "<<polled_devices_config_path<<std::endl;

  closelog();
};
