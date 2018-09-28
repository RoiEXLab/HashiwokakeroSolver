#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cstdint>

#include "./GameParser.h"
#include "./Game.h"

// _____________________________________________________________________________

std::vector<Island> GameParser::parse(const std::string &filename) const {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << filename << std::endl;
    throw 4;
  }
  std::string line;
  // counter to keep track for plain parser, ingores comments
  uint32_t lineCount = 0;
  // counter for error messages
  uint32_t realLineCount = 0;
  std::vector<Island> data;
  try {
    while (std::getline(file, line)) {
      if (parseLine(line, lineCount, &data)) {
        lineCount++;
      }
      realLineCount++;
    }
  } catch (const std::invalid_argument &error) {
    std::cerr << "Parser error in line " << realLineCount << std::endl;
    std::cerr << "Error while performing " << error.what() << std::endl;
    std::cerr << "Content: " << line << std::endl;
    throw 5;
  }
  file.close();
  return data;
}

// _____________________________________________________________________________

GameParser* GameParser::getParser(const std::string &filename) {
  size_t index = filename.find_last_of('.');
  if (index == std::string::npos
    || (filename.length() - 1 != index && filename[index + 1] == '/')) {
    std::cerr << "Invalid filename '" << filename << "' has no extension."
              << std::endl << "Unable to detect format" << std::endl;
    throw 2;
  }
  std::string ext = filename.substr(index + 1);
  // Deduct required parser
  bool isPlain = ext == "plain";
  if (!isPlain && ext != "xy") {
    std::cerr << "Invalid File '" << filename << "' has an unknown extension."
              << std::endl;
    throw 3;
  }
  // WHY DO I NEED TO EXPLICITLY CAST THIS?!?!?!?!?!?!?
  return isPlain ? static_cast<GameParser*>(new PlainParser()) : new XYParser();
}

// _____________________________________________________________________________

Game GameParser::autoParse(const std::string &filename) {
  std::unique_ptr<GameParser> parser(getParser(filename));
  return Game(parser->parse(filename));
}

// _____________________________________________________________________________

bool PlainParser::parseLine(const std::string &line, uint32_t y,
  std::vector<Island>* data) const {
  if (line.length() == 0 || line[0] != '#') {
    for (size_t x = 0; x < line.length(); x++) {
      if (line[x] != ' ') {
        auto requiredBridges = std::stoi(line.substr(x, 1));
        if (requiredBridges <= 0 || requiredBridges > 8) {
          throw std::invalid_argument("check that 0 < requiredBridges <= 8");
        }
        data->push_back(Island(x, y, requiredBridges));
      }
    }
    return true;
  }
  return false;
}

// _____________________________________________________________________________

bool XYParser::parseLine(const std::string& line, uint32_t lineNumber,
  std::vector<Island>* data) const {
  if (line.length() != 0 && line[0] == '#') {
    return false;
  }
  size_t first = line.find(',');
  size_t second = line.find(',', first + 1);
  if (first == std::string::npos || second == std::string::npos) {
    throw std::invalid_argument("delimiter search");
  }
  uint32_t x = std::stoi(line.substr(0, first));
  uint32_t y = std::stoi(line.substr(first + 1, second - (first + 1)));
  auto bridges = std::stoi(line.substr(second + 1,
    line.length() - (second + 1)));
  if (bridges > 8 || bridges <= 0) {
    throw std::invalid_argument("check that 0 < requiredBridges <= 8");
  }
  data->push_back(Island(x, y, bridges));
  return true;
}
