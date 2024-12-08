#include <iostream>
#include "driver/driver.hpp"
#include "support/config.hpp"


using namespace std;

static auto& config = sysy::Config::getInstance();

int main(int argc, char* argv[]) {
  config.parseCmdArgs(argc, argv);
  config.print_info();

  if (config.infile.empty()) {
    cerr << "Error: input file not specified" << endl;
    config.print_help();
    return EXIT_FAILURE;
  }

  compilerPipeline();
  return EXIT_SUCCESS;
}