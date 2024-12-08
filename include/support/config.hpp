#pragma once
#include <memory>
#include <string>
#include <iostream>
#include <vector>

#include <filesystem>
#include "ir/ir.hpp"
namespace fs = std::filesystem;

namespace sysy {

enum OptLevel : uint32_t { O0 = 0, O1 = 1, O2 = 2, O3 = 3 };
enum LogLevel : uint32_t { SILENT, INFO, DEBUG };

class Config {
protected:
  std::ostream* mos;
  std::ostream* merros;

public:
  std::string infile;
  std::string outfile;

  std::vector<std::string> passes;
  bool genIR = false;
  bool genASM = false;

  OptLevel optLevel = OptLevel::O0;
  LogLevel logLevel = LogLevel::SILENT;

public:
  Config() : mos(&std::cout), merros(&std::cerr) {}
  Config(int argc, char* argv[]) { parseTestArgs(argc, argv); }

  // Delete copy constructor and assignment operator to prevent copies
  Config(const Config&) = delete;
  Config& operator=(const Config&) = delete;

  // Public method to get the instance of the singleton
  static Config& getInstance() {
    static Config instance;
    return instance;
  }
  auto debugDir() const {
    // mkdir ./.debug/xxx/ for debug info
    return fs::path("./.debug") / fs::path(infile).filename().replace_extension("");
  }

  void parseTestArgs(int argc, char* argv[]);
  void parseSubmitArgs(int argc, char* argv[]);

  void parseCmdArgs(int argc, char* argv[]);

  void print_help();
  void print_info();

  std::ostream& os() const { return *mos; }
  void set_ostream(std::ostream& os) { mos = &os; }

  void dumpModule(ir::Module* module, const std::string& filename) const;
};

}  // namespace sysy
