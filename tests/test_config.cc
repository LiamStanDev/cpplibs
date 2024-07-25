#include "cosmic.h"

#include <memory>

std::shared_ptr<cosmic::ConfigVar<int>> g_int_val_cfg =
    cosmic::Config::Lookup("system.port", (int)8080, "system port");

std::shared_ptr<cosmic::ConfigVar<float>> g_float_val_cfg =
    cosmic::Config::Lookup("system.value", (float)10.2f, "system value");

void test_yaml() {
  YAML::Node root = YAML::LoadFile("/home/liam/workspace/cosmic/conf/log.yaml");
  LOG_INFO(ROOT_LOGGER()) << root;
}

int main() {
  LOG_INFO(ROOT_LOGGER()) << g_int_val_cfg->getValue();
  LOG_INFO(ROOT_LOGGER()) << g_int_val_cfg->toString();

  LOG_INFO(ROOT_LOGGER()) << g_float_val_cfg->getValue();
  LOG_INFO(ROOT_LOGGER()) << g_float_val_cfg->toString();

  test_yaml();
  return 0;
}
