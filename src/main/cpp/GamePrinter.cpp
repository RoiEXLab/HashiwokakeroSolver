#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cstdint>
#include "./Game.h"
#include "./GamePrinter.h"

// _____________________________________________________________________________

// Helper function which opens a file for writing
std::ofstream openFile(const std::string &filename) {
  std::ofstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Invalid output File: " << filename << std::endl;
    exit(6);
  }
  return file;
}
// _____________________________________________________________________________

std::vector<std::string> PlainPrinter::generateOutput() const {
  std::vector<std::string> output(_game._height,
    std::string(_game._width, ' '));
  for (const auto &island : _game.getIslands()) {
    // Simple int to ascii conversion
    output[island->_y][island->_x] = '0' + island->_requiredBridges;
    for (const auto &bridge : island->_bridges) {
      if (bridge->_one == island) {
        if (bridge->_one->_x == bridge->_two->_x) {
          auto minmax = std::minmax(bridge->_one->_y, bridge->_two->_y);
          for (uint32_t i = minmax.first + 1; i < minmax.second; i++) {
            output[i][island->_x] = bridge->_doubleBridge ? 'H' : '|';
          }
        } else {
          auto minmax = std::minmax(bridge->_one->_x, bridge->_two->_x);
          for (uint32_t i = minmax.first + 1; i < minmax.second; i++) {
            output[island->_y][i] = bridge->_doubleBridge ? '=' : '-';
          }
        }
      }
    }
  }
  return output;
}

// _____________________________________________________________________________

void PlainPrinter::printToFile(const std::string& filename) const {
  std::ofstream file = openFile(filename);
  for (const auto &line : generateOutput()) {
    file << line << std::endl;
  }
  file.close();
}

// _____________________________________________________________________________

void XYPrinter::printToFile(const std::string& filename) const {
  std::ofstream file = openFile(filename);
  for (const auto &island : _game.getIslands()) {
    for (const auto &bridge : island->_bridges) {
      if (bridge->_one == island) {
        for (int8_t i = 0; i < (bridge->_doubleBridge ? 2 : 1); i++) {
          file << island->_x << ',' << island->_y << ','
          << bridge->_two->_x << ',' << bridge->_two->_y << std::endl;
        }
      }
    }
  }
  file.close();
}
