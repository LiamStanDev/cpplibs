#pragma once

#include "cosmic/log.h"
#include <boost/lexical_cast.hpp>
#include <exception>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <yaml-cpp/yaml.h>

namespace cosmic {

class ConfigVarBase {
public:
  using ptr = std::shared_ptr<ConfigVarBase>;
  ConfigVarBase(const std::string& name, const std::string& desc = "")
      : m_name(name), m_desc(desc) {}
  virtual ~ConfigVarBase() {};

  const std::string& getName() const { return m_name; }
  const std::string& getDescription() const { return m_desc; }

  virtual std::string toString() = 0;
  virtual bool fromString(const std::string& val) = 0;

private:
  std::string m_name;
  std::string m_desc;
};

template <class T> class ConfigVar : public ConfigVarBase {
public:
  using ptr = std::shared_ptr<ConfigVar<T>>;

  ConfigVar(const std::string& name, const T& default_value,
            const std::string& desc = "")
      : ConfigVarBase(name, desc), m_val(default_value) {}

  const T getValue() const { return m_val; }
  void setValue(const T& v) { m_val = v; }

  std::string toString() override {
    try {
      return boost::lexical_cast<std::string>(m_val);
    } catch (std::exception& e) {
      LOG_ERROR(ROOT_LOGGER())
          << "ConfigVar::toString exeception" << e.what()
          << "convert: " << typeid(m_val).name() << " to string";
    }
    return "";
  }

  bool fromString(const std::string& val) override {
    try {
      m_val = boost::lexical_cast<T>(val);
    } catch (std::exception& e) {
      LOG_ERROR(ROOT_LOGGER())
          << "ConfigVar::fromString exeception" << e.what()
          << "convert: " << "string to " << typeid(m_val).name();
    }
    return false;
  }

private:
  T m_val;
};

class Config {
public:
  using ConfigVarMap = std::map<std::string, std::shared_ptr<ConfigVarBase>>;

  template <class T>
  static std::shared_ptr<ConfigVar<T>> Lookup(const std::string& name) {
    auto it = s_vars.find(name);
    if (it == s_vars.end()) {
      return nullptr;
    }

    // dynamic_pointer_cast is for shared_ptr from child to base.
    return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
  }

  template <class T>
  static std::shared_ptr<ConfigVar<T>>
  Lookup(const std::string& name, const T& default_value,
         const std::string& description = "") {
    auto tmp = Lookup<T>(name);
    if (tmp) {
      LOG_INFO(ROOT_LOGGER()) << "Lookup name=" << name << " exists";
      return tmp;
    }

    // validate name
    if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTU"
                               "VWXYZ._0123456789") != std::string::npos) {
      LOG_ERROR(ROOT_LOGGER()) << "Lookup name invalid" << name;
      throw std::invalid_argument(name);
    }

    std::shared_ptr<ConfigVar<T>> var{
        new ConfigVar<T>{name, default_value, description}};
    s_vars[name] = var;

    return var;
  }

private:
  static ConfigVarMap s_vars;
};

} // namespace cosmic
