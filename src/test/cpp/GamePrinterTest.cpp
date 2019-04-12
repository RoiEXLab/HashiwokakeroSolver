#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "./Game.h"
#include "./GamePrinter.h"

// _____________________________________________________________________________

// Helper function which creates a populated Game instance
Game createTestGame() {
  Game game({
    Island(0, 0, 1),
    Island(3, 0, 2),
    Island(4, 0, 3),
    Island(0, 3, 4),
    Island(3, 3, 5),
    Island(5, 3, 6),
    Island(1, 5, 7),
    Island(3, 5, 8)
  });
  game.connect(game.getIsland(0, 0), game.getIsland(3, 0), false);
  game.connect(game.getIsland(0, 0), game.getIsland(0, 3), true);
  game.connect(game.getIsland(3, 0), game.getIsland(3, 3), false);
  game.connect(game.getIsland(0, 3), game.getIsland(3, 3), false);
  game.connect(game.getIsland(3, 3), game.getIsland(5, 3), true);
  game.connect(game.getIsland(1, 5), game.getIsland(3, 5), false);
  return game;
}

// _____________________________________________________________________________

TEST(PlainPrinterTest, generateOutput) {
  const Game &game = createTestGame();
  PlainPrinter printer(game);
  const auto &output = printer.generateOutput();

  ASSERT_EQ(6, output.size());
  EXPECT_EQ("1--23 ", output[0]);
  EXPECT_EQ("H  |  ", output[1]);
  EXPECT_EQ("H  |  ", output[2]);
  EXPECT_EQ("4--5=6", output[3]);
  EXPECT_EQ("      ", output[4]);
  EXPECT_EQ(" 7-8  ", output[5]);
}

// _____________________________________________________________________________

TEST(PlainPrinterTest, printToFile) {
  const Game &game = createTestGame();
  PlainPrinter printer(game);
  printer.printToFile("./temporaryTestFile");

  std::vector<std::string> fileContent;
  std::ifstream file("./temporaryTestFile");
  std::string line;
  while (std::getline(file, line)) {
    fileContent.push_back(line);
  }
  file.close();
  std::remove("./temporaryTestFile");

  ASSERT_EQ(6, fileContent.size());
  EXPECT_EQ("1--23 ", fileContent[0]);
  EXPECT_EQ("H  |  ", fileContent[1]);
  EXPECT_EQ("H  |  ", fileContent[2]);
  EXPECT_EQ("4--5=6", fileContent[3]);
  EXPECT_EQ("      ", fileContent[4]);
  EXPECT_EQ(" 7-8  ", fileContent[5]);
}

// _____________________________________________________________________________

TEST(XYPrinterTest, printToFile) {
  const Game &game = createTestGame();
  XYPrinter printer(game);
  printer.printToFile("./temporaryTestFile");

  std::vector<std::string> fileContent;
  std::ifstream file("./temporaryTestFile");
  std::string line;
  while (std::getline(file, line)) {
    fileContent.push_back(line);
  }
  file.close();
  std::remove("./temporaryTestFile");

  std::sort(fileContent.begin(), fileContent.end());

  ASSERT_EQ(8, fileContent.size());
  EXPECT_EQ("0,0,0,3", fileContent[0]);
  EXPECT_EQ("0,0,0,3", fileContent[1]);
  EXPECT_EQ("0,0,3,0", fileContent[2]);
  EXPECT_EQ("0,3,3,3", fileContent[3]);
  EXPECT_EQ("1,5,3,5", fileContent[4]);
  EXPECT_EQ("3,0,3,3", fileContent[5]);
  EXPECT_EQ("3,3,5,3", fileContent[6]);
  EXPECT_EQ("3,3,5,3", fileContent[7]);
}

// _____________________________________________________________________________


TEST(GamePrinterTest, print) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  Game game({});
  PlainPrinter plainPrinter(game);
  EXPECT_EXIT(plainPrinter.printToFile("//////&!?'"),
    ::testing::ExitedWithCode(6),
    "Invalid.+//////&!\\?'.*");
  XYPrinter xyPrinter(game);
  EXPECT_EXIT(xyPrinter.printToFile("//////&!?'"),
    ::testing::ExitedWithCode(6),
    "Invalid.+//////&!\\?'.*");
}
