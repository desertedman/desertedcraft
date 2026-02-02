#include "application.h"
#include <exception>
#include <iostream>
#include <stdexcept>

int main() {
  try {
    Application application;
    application.Run();
  }

  catch (const std::runtime_error &e) {
    std::cerr << "Runtime error: " << e.what() << std::endl;
    return -1;
  }

  catch (const std::exception &e) {
    std::cerr << "Unknown error: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
