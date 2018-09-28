#ifndef GAMEPRINTER_H_
#define GAMEPRINTER_H_

#include <gtest/gtest_prod.h>
#include <string>
#include <vector>
#include "./Game.h"

// _____________________________________________________________________________

// Abstract class to print a file to a file
class GamePrinter {
 protected:
  // The game instance to be serialized
  const Game &_game;

 public:
  // Constructs a printer instance with the provided game instance
  explicit GamePrinter(const Game &game) : _game(game) {}

  // abstract function to be implemented by subclasses
  virtual void printToFile(const std::string&) const = 0;

  // default destructor
  virtual ~GamePrinter() = default;
};

// _____________________________________________________________________________


// Printer for the xy.solution format
class XYPrinter : public GamePrinter {
 public:
  // public constructor mirroring its superclass constructor
  explicit XYPrinter(const Game &game) : GamePrinter(game) {}

  // Serializes the game instance into the provided file using the xy format
  void printToFile(const std::string&) const override;
};

// _____________________________________________________________________________

// Printer for the plain.solution format
class PlainPrinter : public GamePrinter {
  FRIEND_TEST(PlainPrinterTest, generateOutput);
  // Helper method to generate the output that should
  // get printed
  std::vector<std::string> generateOutput() const;

 public:
  // public constructor mirroring its superclass constructor
  explicit PlainPrinter(const Game &game) : GamePrinter(game) {}

  // Serializes the game instance into the provided file using the plain format
  void printToFile(const std::string&) const override;
};

#endif  // GAMEPRINTER_H_
