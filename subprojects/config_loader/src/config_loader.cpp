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
#include <atomic>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/error/error.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <sstream>
#include <syslog.h>
extern std::atomic_bool syslog_crit_enable;
extern std::atomic_bool syslog_warn_enable;
extern std::atomic_bool syslog_info_enable;
extern std::atomic_bool sqllog_crit_enable;
extern std::atomic_bool sqllog_warn_enable;
extern std::atomic_bool sqllog_info_enable;

namespace fs = std::filesystem;

void config_loader::load_configs(const string &general_config) {
  if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
      syslog_info_enable.load())
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
    if (syslog_crit_enable.load())
      syslog(LOG_CRIT, "Отсутствует основной конфигурационный файл, запуск "
                       "приложения не возможно.");
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    exit(EXIT_FAILURE);
  }
  rapidjson::Document doc;
  std::fstream ifs(general_config_path);
  rapidjson::IStreamWrapper isw(ifs);
  rapidjson::ParseResult ok = doc.ParseStream(isw);
  if (!ok) {
    if (syslog_crit_enable.load())
      syslog(LOG_CRIT,
             "Синтаксическая ошибка, %s, в основном конфигурационном файле, "
             "приложение не может быть запущено.",
             rapidjson::GetParseError_En(ok.Code()));
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    exit(EXIT_FAILURE);
  }
  if (doc.HasMember("sql-base-address")) {
    if (doc["sql-base-address"].IsString())
      base_address.assign(doc["sql-base-address"].GetString(),
                          doc["sql-base-address"].GetStringLength());
    else if (syslog_warn_enable.load())
      syslog(LOG_WARNING, "Неверный тип поля адреса подключения к базе "
                          "данных, установлен адрес: 127.0.0.1.");
  } else {
    if (syslog_warn_enable.load())
      syslog(LOG_WARNING,
             "Не указан адрес подключения к базе данных, установлен "
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
        if (syslog_warn_enable.load())
          syslog(LOG_WARNING,
                 "Неверный порт подключения к базе данных, установлен "
                 "порт: 5432.");
      }
    }
  } else {
    if (syslog_warn_enable.load())
      syslog(LOG_WARNING,
             "Не указан порт подключения к базе данных, установлен "
             "порт: 5432.");
  }

  if (doc.HasMember("sql-base-db-name")) {
    if (doc["sql-base-db-name"].IsString())
      base_db_name.assign(doc["sql-base-db-name"].GetString(),
                          doc["sql-base-db-name"].GetStringLength());
    else if (syslog_warn_enable.load())
      syslog(LOG_WARNING, "Неверный тип поля имени базы данных, установлено "
                          "имя: telemetry_server.");
  } else {
    if (syslog_warn_enable.load())
      syslog(LOG_WARNING, "Не указан имя базы данных, установлено "
                          "имя: telemetry_server.");
  }

  if (doc.HasMember("sql-base-login")) {
    if (doc["sql-base-login"].IsString())
      base_login.assign(doc["sql-base-login"].GetString(),
                        doc["sql-base-login"].GetStringLength());
    else if (syslog_warn_enable.load())
      syslog(LOG_WARNING, "Неверный тип поля имени пользователя базы данных, "
                          "установлено имя пользователя: postgres.");
  } else {
    if (syslog_warn_enable.load())
      syslog(LOG_WARNING,
             "Не указано имя пользователя базы данных, установлено "
             "имя пользователя: postgres.");
  }

  if (doc.HasMember("sql-base-password")) {
    if (doc["sql-base-password"].IsString())
      base_password.assign(doc["sql-base-password"].GetString(),
                           doc["sql-base-password"].GetStringLength());
    else if (syslog_warn_enable.load())
      syslog(LOG_WARNING, "Неверный тип поля пароля пользователя базы данных, "
                          "установлен пароль пользователя: пустой.");
  } else {
    if (syslog_warn_enable.load())
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
      if (syslog_crit_enable.load())
        syslog(LOG_CRIT,
               "Неверный тип поля, хранящего путь к директории хранения "
               "настроек опроса устройств, запуск приложения невозможено.");
      if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
          syslog_info_enable.load())
        closelog();
      exit(EXIT_FAILURE);
    }
  } else {
    if (syslog_crit_enable.load())
      syslog(LOG_CRIT, "Не указан путь к директории хранения настроек опроса "
                       "устройств, запуск приложения невозможено.");
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    exit(EXIT_FAILURE);
  }

  if (doc.HasMember("syslog-crit-enable")) {
    if (doc["syslog-crit-enable"].IsBool()) {
      syslog_crit_enable.store(doc["syslog-crit-enable"].GetBool());
      if (syslog_info_enable)
        syslog(LOG_INFO, ((syslog_crit_enable.load())
                              ? "Активировано журналирование критических "
                                "ошибок, подсистемой syslog."
                              : "Дезактивировано журналирование критических "
                                "ошибок, подсистемой syslog."));
    } else {
      if (syslog_warn_enable.load())
        syslog(
            LOG_WARNING,
            "Неверный тип поля syslog-crit-enable, журналирование критических "
            "ошибок, подсистемой syslog, осталось активировано.");
    }
  } else {
    if (syslog_warn_enable.load())
      syslog(LOG_WARNING,
             "Отсутствует поле syslog-crit-enable, журналирование критических "
             "ошибок, подсистемой syslog, осталось активировано.");
  }

  if (doc.HasMember("syslog-warn-enable")) {
    if (doc["syslog-warn-enable"].IsBool()) {
      syslog_warn_enable.store(doc["syslog-warn-enable"].GetBool());
      if (syslog_info_enable)
        syslog(LOG_INFO, ((syslog_warn_enable.load())
                              ? "Активировано журналирование предупреждений, "
                                "подсистемой syslog."
                              : "Дезактивировано журналирование "
                                "предупреждений, подсистемой syslog."));
    } else {
      if (syslog_warn_enable.load())
        syslog(LOG_WARNING,
               "Неверный тип поля syslog-warn-enable, журналирование "
               "предупреждений, подсистемой syslog, осталось активировано.");
    }
  } else {
    if (syslog_warn_enable.load())
      syslog(LOG_WARNING,
             "Отсутствует поле syslog-warn-enable, журналирование "
             "предупреждений, подсистемой syslog, осталось активировано.");
  }

  if (doc.HasMember("syslog-info-enable")) {
    if (doc["syslog-info-enable"].IsBool()) {
      syslog_info_enable.store(doc["syslog-info-enable"].GetBool());
      if (syslog_info_enable)
        syslog(LOG_INFO, ((syslog_info_enable.load())
                              ? "Активировано журналирование информационных "
                                "сообщений, подсистемой syslog"
                              : "Дезактивировано журналирование информационных "
                                "сообщений, подсистемой syslog"));
    } else {
      if (syslog_warn_enable.load())
        syslog(LOG_WARNING, "Неверный тип поля syslog-info-enable, "
                            "журналирование информационных сообщений, "
                            "подсистемой syslog, осталось активировано.");
    }
  } else {
    if (syslog_warn_enable.load())
      syslog(
          LOG_WARNING,
          "Отсутствует поле syslog-info-enable, журналирование информационных "
          "сообщений, подсистемой syslog, осталось активировано.");
  }

  if (doc.HasMember("sqllog-crit-enable")) {
    if (doc["sqllog-crit-enable"].IsBool()) {
      sqllog_crit_enable.store(doc["sqllog-crit-enable"].GetBool());
      if (syslog_info_enable)
        syslog(LOG_INFO, ((sqllog_crit_enable.load())
                              ? "Активировано журналирование критических "
                                "ошибок, подсистемой sql."
                              : "Дезактивировано журналирование критических "
                                "ошибок, подсистемой sql."));
    } else {
      if (syslog_warn_enable.load())
        syslog(
            LOG_WARNING,
            "Неверный тип поля sqllog-crit-enable, журналирование критических "
            "ошибок, подсистемой sqllog, осталось активировано.");
    }
  } else {
    if (syslog_warn_enable.load())
      syslog(LOG_WARNING,
             "Отсутствует поле sqllog-crit-enable, журналирование критических "
             "ошибок, подсистемой sql, осталось активировано.");
  }

  if (doc.HasMember("sqllog-warn-enable")) {
    if (doc["sqllog-warn-enable"].IsBool()) {
      sqllog_warn_enable.store(doc["sqllog-warn-enable"].GetBool());
      if (syslog_info_enable)
        syslog(LOG_INFO, ((sqllog_warn_enable.load())
                              ? "Активировано журналирование предупреждений, "
                                "подсистемой sql."
                              : "Дезактивировано журналирование "
                                "предупреждений, подсистемой sql."));
    } else {
      if (sqllog_warn_enable.load())
        syslog(LOG_WARNING,
               "Неверный тип поля sqllog-warn-enable, журналирование "
               "предупреждений, подсистемой sql, осталось активировано.");
    }
  } else {
    if (syslog_warn_enable.load())
      syslog(LOG_WARNING,
             "Отсутствует поле sqllog-warn-enable, журналирование "
             "предупреждений, подсистемой sql, осталось активировано.");
  }

  if (doc.HasMember("sqllog-info-enable")) {
    if (doc["sqllog-info-enable"].IsBool()) {
      sqllog_info_enable.store(doc["sqllog-info-enable"].GetBool());
      if (syslog_info_enable)
        syslog(LOG_INFO, ((syslog_info_enable.load())
                              ? "Активировано журналирование информационных "
                                "сообщений, подсистемой sql"
                              : "Дезактивировано журналирование информационных "
                                "сообщений, подсистемой sql"));
    } else {
      if (syslog_warn_enable.load())
        syslog(LOG_WARNING, "Неверный тип поля sqllog-crit-enable, "
                            "журналирование информационных сообщений, "
                            "подсистемой sql, осталось активировано.");
    }
  } else {
    if (syslog_warn_enable.load())
      syslog(
          LOG_WARNING,
          "Отсутствует поле sqllog-info-enable, журналирование информационных "
          "сообщений, подсистемой sql, осталось активировано.");
  }

  if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
      syslog_info_enable.load())
    closelog();
  device_map=device_config::parse_device_config_file(general_config_path);
  if(!device_map){
    if(syslog_crit_enable.load()){
      syslog(LOG_CRIT, "Отсутствует информация о подключаемых устройствах.");
    }
    exit(EXIT_FAILURE);
  }
  if(device_map->empty()){
    if(syslog_crit_enable.load()){
      syslog(LOG_CRIT, "Отсутствует информация о подключаемых устройствах.");
    }
    exit(EXIT_FAILURE);
  }
};
