#include <gtest/gtest.h>
#include <deque>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <vector>
#include <tuple>
#include "./Game.h"
#include "./Solver.h"
#include "./GamePrinter.h"

// _____________________________________________________________________________

TEST(SolverTest, constructor) {
  Game game({});
  Solver solver(&game);
  ASSERT_EQ(solver._game, &game);
}

// _____________________________________________________________________________

TEST(SolverTest, revertSteps) {
  Game game({
    Island(0, 0, 1),
    Island(0, 9, 2),
    Island(9, 0, 4),
    Island(9, 9, 8)
  });
  auto island1 = game.getIsland(0, 0);
  auto island2 = game.getIsland(0, 9);
  auto island3 = game.getIsland(9, 0);
  auto island4 = game.getIsland(9, 9);
  Bridge* oldBridge = nullptr;
  game.connect(island1, island2, false);
  auto bridge = game.connect(island1, island2, false, &oldBridge);
  std::deque<std::pair<Bridge*, Bridge*>> steps({
    { bridge, oldBridge },
    { game.connect(island2, island4, false), nullptr },
    { game.connect(island3, island4, true), nullptr }
  });
  Solver solver(&game);
  solver.revertSteps(&steps);

  EXPECT_EQ(0, steps.size());

  ASSERT_EQ(1, island1->_bridges.size());
  ASSERT_EQ(1, island2->_bridges.size());
  EXPECT_EQ(0, island3->_bridges.size());
  EXPECT_EQ(0, island4->_bridges.size());

  EXPECT_EQ(oldBridge, island1->_bridges[0]);
  EXPECT_EQ(oldBridge, island2->_bridges[0]);

  EXPECT_FALSE(island1->_bridges[0]->_doubleBridge);
  EXPECT_FALSE(island2->_bridges[0]->_doubleBridge);
}

// _____________________________________________________________________________

TEST(SolverTest, eliminateObvious) {
  Game game({
    Island(0, 0, 2),
    Island(5, 0, 3),
    Island(7, 0, 2),
    Island(0, 5, 4),
    Island(5, 5, 3),
    Island(7, 5, 3),
    Island(0, 7, 2),
    Island(7, 8, 1)
  });

  auto i00 = game.getIsland(0, 0);
  auto i50 = game.getIsland(5, 0);
  auto i70 = game.getIsland(7, 0);
  auto i05 = game.getIsland(0, 5);
  auto i55 = game.getIsland(5, 5);
  auto i75 = game.getIsland(7, 5);
  auto i07 = game.getIsland(0, 7);
  auto i78 = game.getIsland(7, 8);

  Solver solver(&game);
  std::deque<std::pair<Bridge*, Bridge*>> steps;
  ASSERT_TRUE(solver.eliminateObvious({}, &steps));

  EXPECT_EQ(i00->_bridges.size(), 1);
  EXPECT_EQ(1, i00->isConnected(i50));

  EXPECT_EQ(i50->_bridges.size(), 2);
  EXPECT_EQ(1, i50->isConnected(i00));
  EXPECT_EQ(1, i50->isConnected(i70));

  EXPECT_EQ(i70->_bridges.size(), 1);
  EXPECT_EQ(1, i70->isConnected(i50));

  EXPECT_EQ(i05->_bridges.size(), 2);
  EXPECT_EQ(2, i05->isConnected(i07));
  EXPECT_EQ(1, i05->isConnected(i55));

  EXPECT_EQ(i55->_bridges.size(), 2);
  EXPECT_EQ(1, i55->isConnected(i05));
  EXPECT_EQ(1, i55->isConnected(i75));

  EXPECT_EQ(i75->_bridges.size(), 2);
  EXPECT_EQ(1, i75->isConnected(i78));
  EXPECT_EQ(1, i75->isConnected(i55));

  EXPECT_EQ(i07->_bridges.size(), 1);
  EXPECT_EQ(2, i07->isConnected(i05));

  EXPECT_EQ(i78->_bridges.size(), 1);
  EXPECT_EQ(1, i78->isConnected(i75));

  ASSERT_EQ(steps.size(), 6);
  // Order of the pointers might differ, but this is okay
  std::vector<Bridge*> bridges = {
    i50->_bridges[0],
    i50->_bridges[1],
    i05->_bridges[0],
    i05->_bridges[1],
    i75->_bridges[0],
    i75->_bridges[1]
  };
  std::sort(steps.begin(), steps.end());
  std::sort(bridges.begin(), bridges.end());
  EXPECT_EQ(bridges[0], steps[0].first);
  EXPECT_EQ(bridges[1], steps[1].first);
  EXPECT_EQ(bridges[2], steps[2].first);
  EXPECT_EQ(bridges[3], steps[3].first);
  EXPECT_EQ(bridges[4], steps[4].first);
  EXPECT_EQ(bridges[5], steps[5].first);
  EXPECT_EQ(nullptr, steps[0].second);
  EXPECT_EQ(nullptr, steps[1].second);
  EXPECT_EQ(nullptr, steps[2].second);
  EXPECT_EQ(nullptr, steps[3].second);
  EXPECT_EQ(nullptr, steps[4].second);
  EXPECT_EQ(nullptr, steps[5].second);
}

// _____________________________________________________________________________

TEST(SolverTest, extractValidGaps) {
  Game game({
    Island(0, 0, 2),
    Island(2, 0, 3),
    Island(4, 0, 2),
    Island(0, 2, 3),
    Island(4, 2, 3),
    Island(0, 4, 2),
    Island(2, 4, 3),
    Island(4, 4, 2)
  });
  auto i02 = game.getIsland(0, 2);
  auto i42 = game.getIsland(4, 2);
  auto i20 = game.getIsland(2, 0);
  auto i24 = game.getIsland(2, 4);
  auto i40 = game.getIsland(4, 0);
  auto i04 = game.getIsland(0, 4);

  Solver solver(&game);
  std::unordered_multimap<Island*, Island*> forbidden;

  game.connect(i02, i42, false);
  auto gap1 = std::make_tuple(i02, i42, &Direction::RIGHT);
  auto result = solver.extractValidGaps({
    gap1,
    std::make_tuple(i20, i24, &Direction::DOWN)
  }, &forbidden);
  ASSERT_EQ(result.size(), 1);
  ASSERT_EQ(result[0], gap1);


  game.connect(i40, i42, false);
  game.connect(i02, i04, true);
  auto result2 = solver.extractValidGaps({
    std::make_tuple(i04, i42, &Direction::DOWN)
  }, &forbidden);
  ASSERT_EQ(result2.size(), 0);

  ASSERT_EQ(0, forbidden.size());
}

// _____________________________________________________________________________

TEST(SolverTest, tryGaps) {
  Game game({
    Island(2, 0, 2),
    Island(0, 2, 2),
    Island(2, 2, 8),
    Island(4, 2, 2),
    Island(2, 4, 2)
  });
  auto middleIsland = game.getIsland(2, 2);
  auto topIsland = game.getIsland(2, 0);
  auto leftIsland = game.getIsland(0, 2);
  auto rightIsland = game.getIsland(4, 2);

  Solver solver(&game);

  std::deque<std::tuple<Island*, Island*, const Direction*>> gaps = {
    std::make_tuple(topIsland, middleIsland, &Direction::DOWN),
    std::make_tuple(leftIsland, middleIsland, &Direction::LEFT)
  };

  std::unordered_multimap<Island*, Island*> forbidden = {
    { leftIsland, middleIsland }
  };

  EXPECT_FALSE(solver.tryGaps(&gaps, &forbidden));

  EXPECT_EQ(0, gaps.size());

  EXPECT_EQ(0, middleIsland->_bridges.size());
  EXPECT_EQ(0, rightIsland->_bridges.size());
  EXPECT_EQ(0, topIsland->_bridges.size());
  EXPECT_EQ(0, rightIsland->_bridges.size());
  EXPECT_EQ(0, leftIsland->_bridges.size());

  ASSERT_EQ(3, forbidden.size());
  std::vector<std::pair<Island*, Island*>> fvector(forbidden.begin(),
    forbidden.end());
  EXPECT_EQ(topIsland, fvector[0].first);
  EXPECT_EQ(middleIsland, fvector[0].second);
  EXPECT_EQ(leftIsland, fvector[1].first);
  EXPECT_EQ(middleIsland, fvector[1].second);
  EXPECT_EQ(leftIsland, fvector[2].first);
  EXPECT_EQ(middleIsland, fvector[2].second);
}

// _____________________________________________________________________________

TEST(SolverTest, solvePublicSolvable) {
  Game game({
    Island(0, 0, 3),
    Island(2, 0, 3),
    Island(0, 2, 3),
    Island(2, 2, 3)
  });
  auto i00 = game.getIsland(0, 0);
  auto i20 = game.getIsland(2, 0);
  auto i02 = game.getIsland(0, 2);
  auto i22 = game.getIsland(2, 2);

  Solver solver(&game);
  EXPECT_TRUE(solver.solve());
  EXPECT_TRUE(game.isSolved());

  ASSERT_EQ(2, i00->_bridges.size());
  ASSERT_EQ(2, i20->_bridges.size());
  ASSERT_EQ(2, i02->_bridges.size());
  ASSERT_EQ(2, i22->_bridges.size());

  EXPECT_EQ(i00->_bridges[0], i02->_bridges[0]);
  EXPECT_EQ(i00->_bridges[1], i20->_bridges[1]);
  EXPECT_EQ(i02->_bridges[1], i22->_bridges[1]);
  EXPECT_EQ(i20->_bridges[0], i22->_bridges[0]);

  EXPECT_FALSE(i02->_bridges[0]->_doubleBridge);
  EXPECT_TRUE(i20->_bridges[1]->_doubleBridge);
  EXPECT_TRUE(i22->_bridges[1]->_doubleBridge);
  EXPECT_FALSE(i22->_bridges[0]->_doubleBridge);
}

// _____________________________________________________________________________

TEST(SolverTest, solvePublicUnsolvable) {
  Game game({
    Island(0, 0, 1),
    Island(2, 0, 1),
    Island(0, 2, 1),
    Island(2, 2, 1)
  });
  auto i00 = game.getIsland(0, 0);
  auto i20 = game.getIsland(2, 0);
  auto i02 = game.getIsland(0, 2);
  auto i22 = game.getIsland(2, 2);

  Solver solver(&game);
  EXPECT_FALSE(solver.solve());
  EXPECT_FALSE(game.isSolved());

  EXPECT_EQ(0, i00->_bridges.size());
  EXPECT_EQ(0, i20->_bridges.size());
  EXPECT_EQ(0, i02->_bridges.size());
  EXPECT_EQ(0, i22->_bridges.size());
}


// _____________________________________________________________________________

TEST(SolverTest, solvePrivate) {
  Game game({
    Island(2, 0, 2),
    Island(0, 2, 2),
    Island(2, 2, 8),
    Island(4, 2, 2),
    Island(2, 4, 2)
  });
  auto middleIsland = game.getIsland(2, 2);
  auto topIsland = game.getIsland(2, 0);
  auto leftIsland = game.getIsland(0, 2);
  auto bottomIsland = game.getIsland(2, 4);

  std::deque<std::pair<Bridge*, Bridge*>> steps;
  Solver solver(&game);
  EXPECT_FALSE(solver.solve({
    // Even though we effectively only provide 2 Islands
    // al neighbour Islands should be tested as well.
    std::make_tuple(topIsland, middleIsland, &Direction::DOWN),
    std::make_tuple(leftIsland, middleIsland, &Direction::LEFT)
  }, {
    { leftIsland, middleIsland }
  }, &steps));
  EXPECT_FALSE(game.isSolved());

  ASSERT_EQ(2, steps.size());

  EXPECT_TRUE(steps[0].first->_doubleBridge);
  ASSERT_EQ(1, bottomIsland->_bridges.size());
  EXPECT_EQ(bottomIsland->_bridges[0], steps[0].first);
  EXPECT_EQ(middleIsland->_bridges[0], steps[0].first);
  EXPECT_EQ(nullptr, steps[0].second);

  ASSERT_EQ(0, leftIsland->_bridges.size());
}

// _____________________________________________________________________________

TEST(SmartConnectorTest, constructor) {
  Game game({
    Island(2, 0, 1),
    Island(0, 2, 2),
    Island(2, 2, 6),
    Island(4, 2, 2),
    Island(2, 4, 1)
  });
  auto middleIsland = game.getIsland(2, 2);
  auto topIsland = game.getIsland(2, 0);
  auto leftIsland = game.getIsland(0, 2);
  auto rightIsland = game.getIsland(4, 2);
  auto bottomIsland = game.getIsland(2, 4);

  // The constructor expects a specific scheme from
  // the forbidden multimap in order to avoid duplicates
  // therefore only 2 of those forbidden connections should
  // actually be ignored
  SmartConnector smartConnector(&game, middleIsland, {
    { middleIsland, topIsland },
    { middleIsland, leftIsland },
    { middleIsland, rightIsland },
    { middleIsland, bottomIsland }
  });
  EXPECT_EQ(&game, smartConnector._game);
  EXPECT_EQ(middleIsland, smartConnector._island);

  ASSERT_EQ(4, smartConnector._neighbours.size());
  EXPECT_EQ(topIsland, smartConnector._neighbours[0].first);
  EXPECT_EQ(1, smartConnector._neighbours[0].second);
  EXPECT_EQ(rightIsland, smartConnector._neighbours[1].first);
  EXPECT_EQ(0, smartConnector._neighbours[1].second);
  EXPECT_EQ(bottomIsland, smartConnector._neighbours[2].first);
  EXPECT_EQ(0, smartConnector._neighbours[2].second);
  EXPECT_EQ(leftIsland, smartConnector._neighbours[3].first);
  EXPECT_EQ(2, smartConnector._neighbours[3].second);
}

// _____________________________________________________________________________

TEST(SmartConnectorTest, maxBandwidthRegardingForbidden) {
  Game game({
    Island(2, 0, 1),
    Island(0, 2, 3),
    Island(2, 2, 2)
  });
  auto middleIsland = game.getIsland(2, 2);
  auto topIsland = game.getIsland(2, 0);
  auto leftIsland = game.getIsland(0, 2);

  SmartConnector smartConnector(&game, middleIsland, {});
  EXPECT_EQ(0, smartConnector.maxBandwidthRegardingForbidden(
    topIsland, middleIsland, { { topIsland, middleIsland } }));
  EXPECT_EQ(1, smartConnector.maxBandwidthRegardingForbidden(
    topIsland, middleIsland, { { middleIsland, topIsland } }));

  EXPECT_EQ(0, smartConnector.maxBandwidthRegardingForbidden(
    leftIsland, middleIsland, { { leftIsland, middleIsland } }));
  EXPECT_EQ(2, smartConnector.maxBandwidthRegardingForbidden(
    leftIsland, middleIsland, { { middleIsland, leftIsland } }));
}

// _____________________________________________________________________________

TEST(SmartConnectorTest, createPair) {
  Game game({
    Island(2, 0, 1),
    Island(0, 2, 3),
    Island(2, 2, 2)
  });
  auto middleIsland = game.getIsland(2, 2);
  auto topIsland = game.getIsland(2, 0);
  auto leftIsland = game.getIsland(0, 2);

  SmartConnector smartConnector(&game, middleIsland, {});
  auto result1 = smartConnector.createPair(
    topIsland, middleIsland, { { topIsland, middleIsland } });
  EXPECT_EQ(topIsland, result1.first);
  EXPECT_EQ(0, result1.second);
  auto result2 = smartConnector.createPair(
    middleIsland, topIsland, { { topIsland, middleIsland } });
  EXPECT_EQ(topIsland, result2.first);
  EXPECT_EQ(1, result2.second);

  auto result3 = smartConnector.createPair(
    leftIsland, middleIsland, { { leftIsland, middleIsland } });
  EXPECT_EQ(leftIsland, result3.first);
  EXPECT_EQ(0, result3.second);
  auto result4 = smartConnector.createPair(
    middleIsland, leftIsland, { { leftIsland, middleIsland } });
  EXPECT_EQ(leftIsland, result4.first);
  EXPECT_EQ(2, result4.second);
}

// _____________________________________________________________________________

TEST(SmartConnectorTest, createNeighbours) {
  Game game({
    Island(2, 0, 2),
    Island(0, 2, 1),
    Island(2, 2, 6),
    Island(4, 2, 1),
    Island(2, 4, 2)
  });
  auto middleIsland = game.getIsland(2, 2);
  auto topIsland = game.getIsland(2, 0);
  auto leftIsland = game.getIsland(0, 2);
  auto rightIsland = game.getIsland(4, 2);
  auto bottomIsland = game.getIsland(2, 4);

  SmartConnector smartConnector(&game, middleIsland, {});

  auto result = smartConnector.createNeighbours({
    { topIsland, middleIsland },
    { leftIsland, middleIsland },
    { rightIsland, middleIsland },
    { bottomIsland, middleIsland }
  });

  ASSERT_EQ(4, result.size());
  EXPECT_EQ(topIsland, result[0].first);
  EXPECT_EQ(0, result[0].second);
  EXPECT_EQ(rightIsland, result[1].first);
  EXPECT_EQ(1, result[1].second);
  EXPECT_EQ(bottomIsland, result[2].first);
  EXPECT_EQ(2, result[2].second);
  EXPECT_EQ(leftIsland, result[3].first);
  EXPECT_EQ(0, result[3].second);
}

// _____________________________________________________________________________

TEST(SmartConnectorTest, reversedFindConnection) {
  Game game({
    Island(0, 0, 1),
    Island(2, 0, 2),
    Island(0, 2, 2),
    Island(2, 2, 5),
    Island(4, 2, 1),
    Island(2, 4, 2)
  });
  auto topLeftIsland = game.getIsland(0, 0);
  auto middleIsland = game.getIsland(2, 2);
  auto topIsland = game.getIsland(2, 0);
  auto leftIsland = game.getIsland(0, 2);
  auto rightIsland = game.getIsland(4, 2);
  auto bottomIsland = game.getIsland(2, 4);

  game.connect(topLeftIsland, topIsland, false);

  SmartConnector smartConnector(&game, middleIsland, {});

  std::deque<std::pair<Bridge*, Bridge*>> steps;
  smartConnector.reversedFindConnection(5, &steps);

  EXPECT_EQ(1, topLeftIsland->_bridges.size());
  EXPECT_EQ(2, topIsland->_bridges.size());
  EXPECT_EQ(2, middleIsland->_bridges.size());
  EXPECT_EQ(0, leftIsland->_bridges.size());
  EXPECT_EQ(1, rightIsland->_bridges.size());
  EXPECT_EQ(0, bottomIsland->_bridges.size());

  ASSERT_EQ(2, steps.size());
  EXPECT_EQ(middleIsland, steps[0].first->_one);
  EXPECT_EQ(topIsland, steps[0].first->_two);
  EXPECT_FALSE(steps[0].first->_doubleBridge);
  EXPECT_EQ(nullptr, steps[0].second);
  EXPECT_EQ(middleIsland, steps[1].first->_one);
  EXPECT_EQ(rightIsland, steps[1].first->_two);
  EXPECT_FALSE(steps[1].first->_doubleBridge);
  EXPECT_EQ(nullptr, steps[1].second);
}

// _____________________________________________________________________________

TEST(SmartConnectorTest, findReverseConnectIslands) {
  Game game({
    Island(0, 0, 1),
    Island(2, 0, 3),
    Island(0, 2, 2),
    Island(2, 2, 4),
    Island(4, 2, 2)
  });
  auto topLeftIsland = game.getIsland(0, 0);
  auto middleIsland = game.getIsland(2, 2);
  auto topIsland = game.getIsland(2, 0);
  auto leftIsland = game.getIsland(0, 2);
  auto rightIsland = game.getIsland(4, 2);

  game.connect(topLeftIsland, topIsland, false);

  SmartConnector smartConnector(&game, middleIsland, {});

  auto result = smartConnector.findReverseConnectIslands(4);

  ASSERT_EQ(3, result.size());
  EXPECT_EQ(topIsland, result[0]);
  EXPECT_EQ(rightIsland, result[1]);
  EXPECT_EQ(leftIsland, result[2]);
}

// _____________________________________________________________________________

TEST(SmartConnectorTest, connectSmart) {
  Game game({
    Island(2, 0, 3),
    Island(5, 0, 2),
    Island(0, 2, 2),
    Island(2, 2, 4),
    Island(4, 2, 2),
    Island(2, 4, 2)
  });
  auto middleIsland = game.getIsland(2, 2);
  auto i20 = game.getIsland(2, 0);
  auto i02 = game.getIsland(0, 2);
  auto i50 = game.getIsland(5, 0);
  auto i42 = game.getIsland(4, 2);
  auto i24 = game.getIsland(2, 4);

  game.connect(i20, i50, true);

  SmartConnector smartConnector(&game, middleIsland, {
    { middleIsland, i42 }
  });

  std::deque<std::pair<Bridge*, Bridge*>> steps;
  EXPECT_TRUE(smartConnector.connectSmart(4, &steps));

  ASSERT_EQ(2, steps.size());
  EXPECT_FALSE(steps[0].first->_doubleBridge);
  EXPECT_EQ(middleIsland, steps[0].first->_one);
  EXPECT_EQ(i24, steps[0].first->_two);
  EXPECT_EQ(nullptr, steps[0].second);
  EXPECT_FALSE(steps[1].first->_doubleBridge);
  EXPECT_EQ(middleIsland, steps[1].first->_one);
  EXPECT_EQ(i02, steps[1].first->_two);
  EXPECT_EQ(nullptr, steps[1].second);

  SmartConnector smartConnector2(&game, middleIsland, {
    { middleIsland, i42 }
  });

  std::deque<std::pair<Bridge*, Bridge*>> steps2;
  EXPECT_TRUE(smartConnector2.connectSmart(2, &steps2));

  ASSERT_EQ(1, steps2.size());
  EXPECT_FALSE(steps2[0].first->_doubleBridge);
  EXPECT_EQ(middleIsland, steps2[0].first->_one);
  EXPECT_EQ(i20, steps2[0].first->_two);
  EXPECT_EQ(nullptr, steps2[0].second);
}

// _____________________________________________________________________________

TEST(SmartConnectorTest, availableConnections) {
  Game game({
    Island(1, 0, 1),
    Island(2, 0, 3),
    Island(5, 0, 2),
    Island(0, 2, 2),
    Island(2, 2, 3),
    Island(4, 2, 1),
    Island(1, 3, 1),
    Island(2, 4, 1)
  });
  auto middleIsland = game.getIsland(2, 2);
  auto i10 = game.getIsland(1, 0);
  auto i20 = game.getIsland(2, 0);
  auto i50 = game.getIsland(5, 0);
  auto i42 = game.getIsland(4, 2);
  auto i13 = game.getIsland(1, 3);

  game.connect(i10, i13, false);
  game.connect(i20, i50, true);

  SmartConnector smartConnector(&game, middleIsland, {
    { middleIsland, i42 }
  });

  ASSERT_EQ(2, smartConnector.availableConnections());
}
