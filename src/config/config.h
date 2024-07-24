#pragma once

#include <boost/lexical_cast.hpp>
#include <memory>
#include <sstream>
#include <string>

namespace cpplibs {

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

  std::string toString() override {
    try {
      return boost::lexical_cast<std::string>(m_val);
    } catch (...) {
    }
    return "";
  }

  bool fromString(const std::string& val) override {}

private:
  T m_val;
};

} // namespace cpplibs
