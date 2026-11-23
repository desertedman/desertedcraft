#include "application.h"
#include <cstdlib>
#include <exception>
#include <iostream>

int main() {
  try {
    Application application;
    application.Run();
  }

  catch (const std::exception &e) {
    std::cerr << "EXCEPTION OCCURRED: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return 0;
}
