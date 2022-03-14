#include <functional>
#include <iostream>

#include <spdlog/spdlog.h>

#include "relay/RtmpRelay.h"


int main(int argc, char *argv[])
{
  if (argc != 3) {
    spdlog::error("example $input $output");
    return -1;
  }

  std::string input_file = argv[1];
  std::string output_url = argv[2];

  RtmpRelay relay;
  try {
    relay.initialize(input_file, output_url);
    relay.play();
    while (true) {
      relay.wait();
    }
  } catch (const std::exception &e){
    spdlog::error(e.what());
  }
  relay.cleanup();

  return 0;
}