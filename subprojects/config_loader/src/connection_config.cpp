#include "connection_config.hpp"
#include <atomic>
#include <filesystem>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/error/error.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>
#include <sstream>
#include <syslog.h>
#include <utility>
extern std::atomic_bool syslog_crit_enable;
extern std::atomic_bool syslog_warn_enable;
extern std::atomic_bool syslog_info_enable;
extern std::atomic_bool sqllog_crit_enable;
extern std::atomic_bool sqllog_warn_enable;
extern std::atomic_bool sqllog_info_enable;
namespace fs = std::filesystem;

shared_ptr<map<string, shared_ptr<connection_config>>>
connection_config::parse_connection_config_files(
    const string &general_config_path) {
  if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
      syslog_info_enable.load())
    openlog("telemetry_server", LOG_PID | LOG_PERROR, LOG_USER);

  fs::path path(general_config_path);
  path /= "configs";
  path /= "connections";
  if (!fs::exists(path)) {
    syslog(LOG_CRIT, "Отсутсвует директория хранения настроек соединений"
                     ", дальнейшая загрузка приложения не возможно.");
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
    syslog(LOG_CRIT, "Отсутствуют файлы хранения настроек соединений"
                     ", дальнейшая загрузка приложения не возможно.");
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
      syslog_info_enable.load())
    closelog();
  shared_ptr<map<string, shared_ptr<connection_config>>> result{};
  for (auto iter : file_list) {
    auto buffer = parse_connection_config_file(iter);
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

shared_ptr<connection_config>
connection_config::parse_connection_config_file(const string &config_path) {
  if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
      syslog_info_enable.load())
    openlog("telemetry_server", LOG_PID | LOG_PERROR, LOG_USER);
  fs::path path(config_path);
  if (!fs::exists(path)) {
    if (syslog_crit_enable.load())
      syslog(LOG_CRIT,
             "Отсутствует конфигурационный файл описания соединения %s",
             config_path.c_str());
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
             "Синтаксическая ошибка, %s, в файле %s, описывающим соединение"
             ".",
             rapidjson::GetParseError_En(ok.Code()), config_path.c_str());
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  shared_ptr<connection_config> result{};
  if (!doc.HasMember("connection-id")) {
    if (syslog_warn_enable)
      syslog(LOG_WARNING,
             "В файле настроек соединения, %s, отсутствует поле connection-id",
             config_path.c_str());
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  if (!doc["connection-id"].IsString()) {
    if (syslog_warn_enable)
      syslog(
          LOG_WARNING,
          "В файле настроек соединения, %s, поле connection-id неверного типа.",
          config_path.c_str());
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  string id(doc["connection-id"].GetString(),
            doc["connection-id"].GetStringLength());
  if (!doc.HasMember("connection-role")) {
    if (syslog_warn_enable)
      syslog(
          LOG_WARNING,
          "В файле настроек соединения, %s, отсутствует поле connection-role",
          config_path.c_str());
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  if (!doc["connection-role"].IsString()) {
    if (syslog_warn_enable)
      syslog(LOG_WARNING,
             "В файле настроек соединения, %s, поле connection-role неверного "
             "типа.",
             config_path.c_str());
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  string role_string(doc["connection-role"].GetString(),
                     doc["connection-role"].GetStringLength());
  enum_role role{};
  if (role_string.find("master") != string::npos)
    role = enum_role::master;
  else if (role_string.find("slave") != string::npos)
    role = enum_role::slave;
  else {
    if (syslog_warn_enable)
      syslog(LOG_WARNING,
             "В файле настроек соединения,%s, не верно указана роль "
             "соединения, возможные значения master/slave",
             config_path.c_str());
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }

  if (!doc.HasMember("connection-type")) {
    if (syslog_warn_enable)
      syslog(
          LOG_WARNING,
          "В файле настроек соединения, %s, отсутствует поле connection-type",
          config_path.c_str());
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  if (!doc["connection-type"].IsString()) {
    if (syslog_warn_enable)
      syslog(LOG_WARNING,
             "В файле настроек соединения, %s, поле connection-type неверного "
             "типа.",
             config_path.c_str());
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  string type_string(doc["connection-type"].GetString(),
                     doc["connection-type"].GetStringLength());
  enum_type type{};
  if (type_string.find("tcp") != string::npos)
    type = enum_type::tcp;
  else if (type_string.find("udp") != string::npos)
    type = enum_type::udp;
  else if (type_string.find("uart") != string::npos)
    type = enum_type::uart;
  else {
    if (syslog_warn_enable)
      syslog(LOG_WARNING,
             "В файле настроек соединения, %s, не верно указан тип "
             "соединения, возможные значения tcp/udp/uart.",
             config_path.c_str());
    if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
        syslog_info_enable.load())
      closelog();
    return nullptr;
  }
  std::size_t await_time{30};
  if (!doc.HasMember("await-time")) {
    if (syslog_warn_enable)
      syslog(LOG_INFO,
             "В файле настроек соединения, %s, отсутствует поле await-time, "
             "значение await_time осталось неизменным.",
             config_path.c_str());
  } else {
    if (!doc["await-time"].IsUint()) {
      if (syslog_warn_enable)
        syslog(LOG_WARNING,
               "В файле настроек соединения, %s, поле await-time имеет "
               "неверный тип, "
               "значение await-time осталось неизменным.",
               config_path.c_str());
    } else {
      await_time = doc["await-time"].GetUint();
    }
  }
  switch (type) {
  case enum_type::tcp:
  case enum_type::udp:
    switch (role) {
    case enum_role::master: {
      string address{"127.0.0.1"};
      if (!doc.HasMember("address")) {
        if (syslog_warn_enable)
          syslog(LOG_INFO,
                 "В файле настроек соединения, %s, отсутствует поле address, "
                 "значение address осталось неизменным.",
                 config_path.c_str());
      } else {
        if (!doc["address"].IsString()) {
          if (syslog_warn_enable)
            syslog(LOG_WARNING,
                   "В файле настроек соединения, %s, поле address имеет "
                   "неверный тип, "
                   "значение address осталось неизменным.",
                   config_path.c_str());
        } else {
          address.assign(doc["address"].GetString(),
                         doc["address"].GetStringLength());
        }
      }
      string port{"520"};
      if (!doc.HasMember("port")) {
        if (syslog_warn_enable)
          syslog(LOG_INFO,
                 "В файле настроек соединения, %s, отсутствует поле port, "
                 "значение port осталось неизменным.",
                 config_path.c_str());
      } else {
        if (doc["port"].IsString()) {
          port.assign(doc["port"].GetString(), doc["port"].GetStringLength());

        } else {
          if (doc["port"].IsUint()) {
            std::stringstream ss;
            ss << doc["port"].GetUint();
            port = ss.str();
          } else {
            if (syslog_warn_enable)
              syslog(LOG_WARNING,
                     "В файле настроек соединения, %s, поле port имеет "
                     "неверный тип, "
                     "значение port осталось неизменным.",
                     config_path.c_str());
          }
        }
      }
      result.reset(
          new connection_config(id, role, type, address, port, await_time));
    } break;
    case enum_role::slave: {
      string port{"520"};
      if (!doc.HasMember("port")) {
        if (syslog_warn_enable)
          syslog(LOG_INFO,
                 "В файле настроек соединения, %s, отсутствует поле port, "
                 "значение port осталось неизменным.",
                 config_path.c_str());
      } else {
        if (doc["port"].IsString()) {
          port.assign(doc["port"].GetString(), doc["port"].GetStringLength());

        } else {
          if (doc["port"].IsUint()) {
            std::stringstream ss;
            ss << doc["port"].GetUint();
            port = ss.str();
          } else {
            if (syslog_warn_enable)
              syslog(LOG_WARNING,
                     "В файле настроек соединения, %s, поле port имеет "
                     "неверный тип, "
                     "значение port осталось неизменным.",
                     config_path.c_str());
          }
        }
      }
      list<string> addresses{};
      if (doc.HasMember("listen-addresses")) {
        if (doc["listen-addresses"].IsArray()) {
          for (auto &item : doc["listen-addresses"].GetArray()) {
            if (item.IsString()) {
              string buffer;
              buffer.assign(item.GetString(), item.GetStringLength());
              addresses.push_back(buffer);
            }
          }
          result.reset(new connection_config(id, role, type, addresses, port,
                                             await_time));
        } else {
          if (syslog_warn_enable)
            syslog(LOG_WARNING,
                   "В файле настроек соединения, %s, поле listen-addresses "
                   "имеет неверный тип.",
                   config_path.c_str());
        }
      } else {
        if (syslog_warn_enable)
          syslog(LOG_WARNING,
                 "В файле настроек соединения, %s, отсутствует поле "
                 "listen-addresses.",
                 config_path.c_str());
      }
    } break;
    }
    break;
  case enum_type::uart: {
    string port{"/dev/ttyS0"};
    if (!doc.HasMember("port")) {
      if (syslog_warn_enable)
        syslog(LOG_INFO,
               "В файле настроек соединения, %s, отсутствует поле port, "
               "значение port осталось неизменным.",
               config_path.c_str());
    } else {
      if (doc["port"].IsString()) {
        port.assign(doc["port"].GetString(), doc["port"].GetStringLength());

      } else {
        if (doc["port"].IsUint()) {
          std::stringstream ss;
          ss << doc["port"].GetUint();
          port = ss.str();
        } else {
          if (syslog_warn_enable)
            syslog(LOG_WARNING,
                   "В файле настроек соединения, %s, поле port имеет "
                   "неверный тип, "
                   "значение port осталось неизменным.",
                   config_path.c_str());
        }
      }
    }
    std::size_t baud_rate{19200};
    if (!doc.HasMember("baudrate")) {
      if (syslog_warn_enable)
        syslog(LOG_INFO,
               "В файле настроек соединения, %s, отсутствует поле baudrate, "
               "значение port осталось неизменным.",
               config_path.c_str());
    } else {
      if (!doc["baudrate"].IsUint()) {
        syslog(LOG_WARNING,
               "В файле настроек соединения, %s, поле baudrate имеет "
               "неверный тип, "
               "значение baudrate осталось неизменным.",
               config_path.c_str());
      } else {
        baud_rate = doc["baudrate"].GetUint();
      }
    }
    enum_flow_control flow_control{enum_flow_control::none};
    if (!doc.HasMember("flow-control")) {
      if (syslog_warn_enable)
        syslog(LOG_INFO,
               "В файле настроек соединения, %s, отсутствует поле flow-contol, "
               "значение flow-control осталось неизменным.",
               config_path.c_str());
    } else {
      if (!doc["flow-control"].IsString()) {
        if (syslog_warn_enable)
          syslog(LOG_WARNING,
                 "В файле настроек соединения, %s, поле flow-control имеет "
                 "неверный тип, "
                 "значение flow-control осталось неизменным.",
                 config_path.c_str());
      } else {
        string flow_control_string(doc["flow-control"].GetString(),
                                   doc["flow-control"].GetStringLength());
        if (flow_control_string.find("none") != string::npos) {
          flow_control = enum_flow_control::none;
        } else {
          if (flow_control_string.find("hardware") != string::npos) {
            flow_control = enum_flow_control::hardware;
          } else {
            if (flow_control_string.find("software") != string::npos) {
              flow_control = enum_flow_control::software;
            } else {
              if (syslog_warn_enable)
                syslog(LOG_WARNING,
                       "Неверное значение поля flow-control, возможные "
                       "значения none/hardware/software.");
            }
          }
        }
      }
    }

    enum_parity parity{enum_parity::none};
    if (!doc.HasMember("parity")) {
      if (syslog_warn_enable)
        syslog(LOG_INFO,
               "В файле настроек соединения, %s, отсутствует поле parity, "
               "значение parity осталось неизменным.",
               config_path.c_str());
    } else {
      if (!doc["parity"].IsString()) {
        if (syslog_warn_enable)
          syslog(LOG_WARNING,
                 "В файле настроек соединения, %s, поле parity имеет "
                 "неверный тип, "
                 "значение parity осталось неизменным.",
                 config_path.c_str());
      } else {
        string parity_string(doc["parity"].GetString(),
                             doc["parity"].GetStringLength());
        if (parity_string.find("none") != string::npos) {
          parity = enum_parity::none;
        } else {
          if (parity_string.find("even") != string::npos) {
            parity = enum_parity::even;
          } else {
            if (parity_string.find("odd") != string::npos) {
              parity = enum_parity::odd;
            } else {
              if (syslog_warn_enable)
                syslog(LOG_WARNING, "Неверное значение поля parity, возможные "
                                    "значения none/even/odd.");
            }
          }
        }
      }
    }

    enum_stop_bits stop_bits{enum_stop_bits::one};
    if (!doc.HasMember("stop_bits")) {
      if (syslog_warn_enable)
        syslog(LOG_INFO,
               "В файле настроек соединения, %s, отсутствует поле stop_bits, "
               "значение stop_bits осталось неизменным.",
               config_path.c_str());
    } else {
      if (!doc["stop-bits"].IsString()) {
        if (!doc["stop-bit"].IsFloat()) {
          if (syslog_warn_enable)
            syslog(LOG_WARNING,
                   "В файле настроек соединения, %s, поле stop_bits имеет "
                   "неверный тип, "
                   "значение stop_bits осталось неизменным.",
                   config_path.c_str());
        } else {
          float stop_bits_float = doc["stop-bits"].GetFloat();
          switch (static_cast<int>(stop_bits_float * 10)) {
          case 10:
            stop_bits = enum_stop_bits::one;
          case 15:
            stop_bits = enum_stop_bits::one_pont_five;
          case 20:
            stop_bits = enum_stop_bits::two;
          default:
            if (syslog_warn_enable)
              syslog(LOG_WARNING,
                     "Неверное значение поля stop_bits, возможные "
                     "значения one/one-point-five/two или 1/1.5/2.");
            break;
          }
        }
      } else {
        string stop_bits_string(doc["stop-bits"].GetString(),
                                doc["stop-bits"].GetStringLength());
        if (stop_bits_string.find("one") != string::npos) {
          stop_bits = enum_stop_bits::one;
        } else {
          if (stop_bits_string.find("one-point-five") != string::npos) {
            stop_bits = enum_stop_bits::one_pont_five;
          } else {
            if (stop_bits_string.find("two") != string::npos) {
              stop_bits = enum_stop_bits::two;
            } else {
              if (syslog_warn_enable)
                syslog(LOG_WARNING,
                       "Неверное значение поля stop_bits, возможные "
                       "значения one/one-point-five/two или 1/1.5/2.");
            }
          }
        }
      }
    }

    enum_data_bits data_bits{enum_data_bits::eight};
    if (!doc.HasMember("data-bits")) {
      if (syslog_warn_enable)
        syslog(LOG_INFO,
               "В файле настроек соединения, %s, отсутствует поле data-bits, "
               "значение data-bits осталось неизменным.",
               config_path.c_str());
    } else {
      if (!doc["data-bits"].IsString()) {
        if (!doc["data-bit"].IsUint()) {
          if (syslog_warn_enable)
            syslog(LOG_WARNING,
                   "В файле настроек соединения, %s, поле data-bits имеет "
                   "неверный тип, "
                   "значение data-bits осталось неизменным.",
                   config_path.c_str());
        } else {
          std::size_t data_bits_float = doc["data-bits"].GetUint();
          switch (data_bits_float) {
          case 5:
            data_bits = enum_data_bits::five;
          case 6:
            data_bits = enum_data_bits::six;
          case 7:
            data_bits = enum_data_bits::seven;
          case 8:
            data_bits = enum_data_bits::eight;
          case 9:
            data_bits = enum_data_bits::nine;
          default:
            if (syslog_warn_enable)
              syslog(LOG_WARNING,
                     "Неверное значение поля data-bits, возможные "
                     "значения five/six/seven/eight/nine или 5/6/7/8/9.");
            break;
          }
        }
      } else {
        string data_bits_string(doc["data-bits"].GetString(),
                                doc["data-bits"].GetStringLength());
        if (data_bits_string.find("five") != string::npos) {
          data_bits = enum_data_bits::five;
        } else {
          if (data_bits_string.find("six") != string::npos) {
            data_bits = enum_data_bits::six;
          } else {
            if (data_bits_string.find("seven") != string::npos) {
              data_bits = enum_data_bits::seven;
            } else {
              if (data_bits_string.find("eight") != string::npos) {
                data_bits = enum_data_bits::eight;
              } else {
                if (data_bits_string.find("nine") != string::npos) {
                  data_bits = enum_data_bits::nine;
                } else {
                  if (syslog_warn_enable)
                    syslog(LOG_WARNING,
                           "Неверное значение поля data-bits, возможные "
                           "значения five/six/seven/eight/nine или 5/6/7/8/9.");
                }
              }
            }
          }
        }
      }
    }
    result.reset(new connection_config(id, role, type, port, baud_rate,
                                       flow_control, parity, stop_bits,
                                       data_bits, await_time));
  } break;
  }
  if (syslog_crit_enable.load() || syslog_warn_enable.load() ||
      syslog_info_enable.load())
    closelog();
  return result;
}
