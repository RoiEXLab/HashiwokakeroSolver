#include <iostream>
#include <chrono>
#include <string>
#include "./Solver.h"
#include "./Game.h"
#include "./GameParser.h"
#include "./GamePrinter.h"

// _____________________________________________________________________________

// Main solver function
int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Missing arguments" << std::endl;
    std::cerr << "Usage: " << argv[0] << " /path/to/input /path/to/output"
      << std::endl;
    return -1;
  }
  try {
    Game game = GameParser::autoParse(argv[1]);

    Solver solver(&game);
    PlainPrinter plainPrinter(game);
    XYPrinter xyPrinter(game);

    auto start = std::chrono::high_resolution_clock::now();
    bool solved = solver.solve();
    auto time = std::chrono::high_resolution_clock::now() - start;

    std::string outputTemplate = argv[2];
    std::string fileExtension = solved ? ".solution" : ".error";
    plainPrinter.printToFile(outputTemplate + ".plain" + fileExtension);
    xyPrinter.printToFile(outputTemplate + ".xy" + fileExtension);
    if (solved) {
      std::cout << "Solved in " << std::chrono::
                   duration_cast<std::chrono::nanoseconds>(time).count()
                << "ns" << std::endl;
    } else {
      std::cout << "No solution possible, took " << std::chrono::
                   duration_cast<std::chrono::nanoseconds>(time).count()
                << "ns" << std::endl;
      return 1;
    }
  } catch (int exitCode) {
    // Throwing and catching the exit code is necessary for
    // valgrind to be happy, because this ensures all the destructors
    // are called that clean everything up #stackUnwinding
    return exitCode;
  }
}
