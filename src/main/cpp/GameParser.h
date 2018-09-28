#ifndef GAMEPARSER_H_
#define GAMEPARSER_H_

#include <vector>
#include <string>
#include <cstdint>
#include "./Game.h"

// _____________________________________________________________________________

// Helper class to allow the variety of Game formats to scale better
class GameParser {
  FRIEND_TEST(GameParserTest, getParser);
  // Abstract function to be implemented by a subclass
  // if returns true the passed line number will be incremented by one
  // The first parameter represents the current line
  // The second parameter is the current line number when ignoring
  // lines consisting of a comment
  // The vector provided as third argument will be filled with
  // discovered islands
  virtual bool parseLine(const std::string&, uint32_t,
    std::vector<Island>*) const = 0;

  // This function deduces the required parser from the provided filename
  // based on its extension and dynamically instanciates an instance
  // and returns the pointer to it
  static GameParser* getParser(const std::string&);

 public:
  // public function which is shared across all Parsers
  // to be called by "the user"
  // It parsers the provided file with the rulez of the subclass
  std::vector<Island> parse(const std::string&) const;
  // default destructor
  virtual ~GameParser() = default;

  // Convinience function to parse a game in one line
  // The file format is deducted from the file extension
  static Game autoParse(const std::string&);
};

// _____________________________________________________________________________

// Parser class for the plain format
// A Little bit more liberal, not strictly based on the specification
class PlainParser: public GameParser {
  FRIEND_TEST(PlainParserTest, parseLine);
  // parse a line based on the plain format
  // See GameParser#parseLine for more information
  bool parseLine(const std::string&, uint32_t,
    std::vector<Island>*) const override;
};

// _____________________________________________________________________________

// Parser class for the xy format
class XYParser: public GameParser {
FRIEND_TEST(XYParserTest, parseLine);
  // parse a line based on the XY format
  // See GameParser#parseLine for more information
  bool parseLine(const std::string&, uint32_t,
    std::vector<Island>*) const override;
};

#endif  // GAMEPARSER_H_
