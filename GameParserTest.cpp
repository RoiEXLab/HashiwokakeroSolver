#include <gtest/gtest.h>
#include <algorithm>
#include <typeinfo>
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <cstdio>
#include "./GameParser.h"

// _____________________________________________________________________________

TEST(PlainParserTest, parse) {
  PlainParser parser;
  auto islands = parser.parse("GameParserTestFile.plain");
  ASSERT_EQ(4, islands.size());

  ASSERT_EQ(1, islands[0]._requiredBridges);
  ASSERT_EQ(0, islands[0]._x);
  ASSERT_EQ(0, islands[0]._y);

  ASSERT_EQ(2, islands[1]._requiredBridges);
  ASSERT_EQ(5, islands[1]._x);
  ASSERT_EQ(0, islands[1]._y);

  ASSERT_EQ(2, islands[2]._requiredBridges);
  ASSERT_EQ(0, islands[2]._x);
  ASSERT_EQ(1, islands[2]._y);

  ASSERT_EQ(5, islands[3]._requiredBridges);
  ASSERT_EQ(3, islands[3]._x);
  ASSERT_EQ(1, islands[3]._y);
}

// _____________________________________________________________________________

TEST(XYParserTest, parse) {
  XYParser parser;
  auto islands = parser.parse("GameParserTestFile.xy");

  ASSERT_EQ(3, islands.size());

  ASSERT_EQ(2, islands[0]._requiredBridges);
  ASSERT_EQ(0, islands[0]._x);
  ASSERT_EQ(1, islands[0]._y);
  ASSERT_EQ(4, islands[1]._requiredBridges);
  ASSERT_EQ(0, islands[1]._x);
  ASSERT_EQ(3, islands[1]._y);
  ASSERT_EQ(1, islands[2]._requiredBridges);
  ASSERT_EQ(4, islands[2]._x);
  ASSERT_EQ(1, islands[2]._y);
}

// _____________________________________________________________________________

TEST(GameParserTest, parserInvalidFileException) {
  PlainParser plainParser;
  EXPECT_THROW(plainParser.parse("./Non existent file"), int);
  XYParser xyParser;
  EXPECT_THROW(xyParser.parse("./Non existent file"), int);
}

// _____________________________________________________________________________

TEST(PlainParserTest, parseLine) {
  PlainParser parser;
  std::vector<Island> islands;
  EXPECT_FALSE(parser.parseLine("# 1   8", 0, &islands));
  EXPECT_FALSE(parser.parseLine("# abclöajdslkj", 10, &islands));
  ASSERT_EQ(0, islands.size());
  EXPECT_TRUE(parser.parseLine("  1    8   ", 0, &islands));
  ASSERT_EQ(2, islands.size());
  ASSERT_EQ(2, islands[0]._x);
  ASSERT_EQ(0, islands[0]._y);
  ASSERT_EQ(1, islands[0]._requiredBridges);
  ASSERT_EQ(7, islands[1]._x);
  ASSERT_EQ(0, islands[1]._y);
  ASSERT_EQ(8, islands[1]._requiredBridges);
  EXPECT_TRUE(parser.parseLine("5", 1, &islands));
  ASSERT_EQ(3, islands.size());
  ASSERT_EQ(0, islands[2]._x);
  ASSERT_EQ(1, islands[2]._y);
  ASSERT_EQ(5, islands[2]._requiredBridges);

  EXPECT_THROW(parser.parseLine("invalid Syntax 😎", 0, &islands),
    std::invalid_argument);
  ASSERT_EQ(3, islands.size());

  // WARNING: Slow test ahead, this tests the edge case of a MAX_INT size string
  // comments this out for fast testing
  std::string test(2147483648, ' ');
  test[0] = '1';
  test[2147483647] = '8';
  EXPECT_TRUE(parser.parseLine(test, 2147483647, &islands));
  ASSERT_EQ(5, islands.size());
  ASSERT_EQ(0, islands[3]._x);
  ASSERT_EQ(2147483647, islands[3]._y);
  ASSERT_EQ(1, islands[3]._requiredBridges);
  ASSERT_EQ(2147483647, islands[4]._x);
  ASSERT_EQ(2147483647, islands[4]._y);
  ASSERT_EQ(8, islands[4]._requiredBridges);
}

// _____________________________________________________________________________

TEST(XYParserTest, parseLine) {
  XYParser parser;
  std::vector<Island> islands;
  EXPECT_FALSE(parser.parseLine("# 1,2,3", 0, &islands));
  EXPECT_FALSE(parser.parseLine("# abclöajdslkj", 10, &islands));
  ASSERT_EQ(0, islands.size());
  EXPECT_TRUE(parser.parseLine("1,2,3", 0, &islands));
  ASSERT_EQ(1, islands.size());
  ASSERT_EQ(1, islands[0]._x);
  ASSERT_EQ(2, islands[0]._y);
  ASSERT_EQ(3, islands[0]._requiredBridges);
  EXPECT_TRUE(parser.parseLine("0,0,1", 0, &islands));
  ASSERT_EQ(2, islands.size());
  ASSERT_EQ(0, islands[1]._x);
  ASSERT_EQ(0, islands[1]._y);
  ASSERT_EQ(1, islands[1]._requiredBridges);
  EXPECT_TRUE(parser.parseLine("2147483647,2147483647,8", 0, &islands));
  ASSERT_EQ(3, islands.size());
  ASSERT_EQ(2147483647, islands[2]._x);
  ASSERT_EQ(2147483647, islands[2]._y);
  ASSERT_EQ(8, islands[2]._requiredBridges);
  EXPECT_THROW(parser.parseLine("invalid Syntax 😎", 0, &islands),
    std::invalid_argument);
  ASSERT_EQ(3, islands.size());
}

// _____________________________________________________________________________

TEST(GameParserTest, autoParsePlain) {
  const Game &game = GameParser::autoParse("GameParserTestFile.plain");
  ASSERT_EQ(6, game._width);
  ASSERT_EQ(2, game._height);

  ASSERT_EQ(4, game._islands.size());

  ASSERT_NE(nullptr, game._islands.at(0 + 0 * 6));
  ASSERT_NE(nullptr, game._islands.at(5 + 0 * 6));
  ASSERT_NE(nullptr, game._islands.at(0 + 1 * 6));
  ASSERT_NE(nullptr, game._islands.at(3 + 1 * 6));

  ASSERT_EQ(1, game._islands.at(0 + 0 * 6)->_requiredBridges);
  ASSERT_EQ(2, game._islands.at(5 + 0 * 6)->_requiredBridges);
  ASSERT_EQ(2, game._islands.at(0 + 1 * 6)->_requiredBridges);
  ASSERT_EQ(5, game._islands.at(3 + 1 * 6)->_requiredBridges);
}

// _____________________________________________________________________________

TEST(GameParserTest, autoParseXY) {
  const Game &game = GameParser::autoParse("GameParserTestFile.xy");
  ASSERT_EQ(5, game._width);
  ASSERT_EQ(4, game._height);

  ASSERT_EQ(3, game._islands.size());

  ASSERT_NE(nullptr, game._islands.at(0 + 1 * 5));
  ASSERT_NE(nullptr, game._islands.at(0 + 3 * 5));
  ASSERT_NE(nullptr, game._islands.at(4 + 1 * 5));

  ASSERT_EQ(2, game._islands.at(0 + 1 * 5)->_requiredBridges);
  ASSERT_EQ(4, game._islands.at(0 + 3 * 5)->_requiredBridges);
  ASSERT_EQ(1, game._islands.at(4 + 1 * 5)->_requiredBridges);
}

// _____________________________________________________________________________

TEST(GameParserTest, getParser) {
  // Tests if the returned parser has the correct type
  std::unique_ptr<GameParser> plainParser(GameParser::getParser("🌉🌉.plain"));
  std::unique_ptr<GameParser> xyParser(GameParser::getParser("️🏝️🏝️.xy"));
  EXPECT_EQ(typeid(PlainParser), typeid(*plainParser));
  EXPECT_EQ(typeid(XYParser), typeid(*xyParser));

  EXPECT_THROW(GameParser::getParser("./No 🐱‍💻 File Extension"), int);
  EXPECT_THROW(GameParser::getParser("./Invalid File Extension.🐱‍💻"), int);
}

// _____________________________________________________________________________

TEST(GameParserTest, testCorrectErrorLoggingPlain) {
  std::ofstream file("TestFile.plain");
  file << "1    8" << std::endl;
  file << "# some random comment lol " << std::endl;
  file << "0    1" << std::endl;
  file.close();
  PlainParser parser;
  EXPECT_THROW(parser.parse("TestFile.plain"), int);
  std::remove("TestFile.plain");
}

// _____________________________________________________________________________

TEST(GameParserTest, testCorrectErrorLoggingXy) {
  std::ofstream file("TestFile.xy");
  file << "0,1,1" << std::endl;
  file << "0,0,5" << std::endl;
  file << "# damn those nasty comments" << std::endl;
  file << "lol" << std::endl;
  file.close();
  XYParser parser;
  EXPECT_THROW(parser.parse("TestFile.xy"), int);
  std::remove("TestFile.xy");
}
