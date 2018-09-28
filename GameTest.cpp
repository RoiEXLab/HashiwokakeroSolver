#include <gtest/gtest.h>
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <deque>
#include <memory>
#include "./Game.h"

// _____________________________________________________________________________

TEST(GameTest, constructor) {
  // We check if the island is properly copied
  // and if the value is ignored
  Island test(0, 0, 0);
  Game game({
    test,
    Island(123, 456, 8),
    // Invalid bridge value on purpose
    Island(1, 1, 9),
    Island(5, 0, 3),
    Island(7, 8, 1)
  });

  EXPECT_EQ(124, game._width);
  EXPECT_EQ(457, game._height);

  for (uint32_t x = 0; x < game._width; x++) {
    ASSERT_NE(nullptr, game._bridgeOccupation[x]);
    for (uint32_t y = 0; y < game._height; y++) {
      ASSERT_EQ(0, game._bridgeOccupation[x][y]);
    }
  }

  ASSERT_EQ(5, game._islands.size());

  EXPECT_NE(nullptr, game._islands.at(0 + 0 * 124));
  EXPECT_NE(nullptr, game._islands.at(123 + 456 * 124));
  EXPECT_NE(nullptr, game._islands.at(1 + 1 * 124));
  EXPECT_NE(nullptr, game._islands.at(5 + 0 * 124));
  EXPECT_NE(nullptr, game._islands.at(7 + 8 * 124));

  EXPECT_EQ(0, game._islands.at(0 + 0 * 124)->_requiredBridges);
  EXPECT_EQ(8, game._islands.at(123 + 456 * 124)->_requiredBridges);
  EXPECT_EQ(9, game._islands.at(1 + 1 * 124)->_requiredBridges);
  EXPECT_EQ(3, game._islands.at(5 + 0 * 124)->_requiredBridges);
  EXPECT_EQ(1, game._islands.at(7 + 8 * 124)->_requiredBridges);

  EXPECT_NE(&test, game._islands.at(000 + 000 * 124));
}

// _____________________________________________________________________________

TEST(GameTest, testGetIsland) {
  Game game({
    Island(0, 0, 1),
    Island(5, 0, 2),
    Island(0, 1, 2),
    Island(3, 1, 8),
    // even an invalid amount of required bridges should work
    Island(5, 5, 99)
  });

  ASSERT_NE(nullptr, game.getIsland(0, 0));
  ASSERT_NE(nullptr, game.getIsland(5, 0));
  ASSERT_NE(nullptr, game.getIsland(0, 1));
  ASSERT_NE(nullptr, game.getIsland(3, 1));
  ASSERT_NE(nullptr, game.getIsland(5, 5));

  EXPECT_EQ(1, game.getIsland(0, 0)->_requiredBridges);
  EXPECT_EQ(2, game.getIsland(5, 0)->_requiredBridges);
  EXPECT_EQ(2, game.getIsland(0, 1)->_requiredBridges);
  EXPECT_EQ(8, game.getIsland(3, 1)->_requiredBridges);
  EXPECT_EQ(99, game.getIsland(5, 5)->_requiredBridges);
}

TEST(GameTest, connect) {
  Game game({
    Island(0, 0, 8),
    Island(1, 0, 8),
    Island(0, 1, 8),
    Island(1, 1, 8),
    Island(0, 2, 8),
    Island(2, 0, 8)
  });

  auto island1 = game.getIsland(0, 0);
  auto island2 = game.getIsland(1, 0);
  auto island3 = game.getIsland(0, 1);
  auto island4 = game.getIsland(1, 1);
  auto island5 = game.getIsland(0, 2);
  auto island6 = game.getIsland(2, 0);

  ASSERT_EQ(nullptr, game.connect(island1, island2, false));
  ASSERT_EQ(nullptr, game.connect(island1, island2, true));

  ASSERT_EQ(nullptr, game.connect(island1, island3, false));
  ASSERT_EQ(nullptr, game.connect(island1, island3, true));

  ASSERT_EQ(nullptr, game.connect(island2, island1, false));
  ASSERT_EQ(nullptr, game.connect(island2, island1, true));

  ASSERT_EQ(nullptr, game.connect(island2, island4, false));
  ASSERT_EQ(nullptr, game.connect(island2, island4, true));

  ASSERT_EQ(nullptr, game.connect(island3, island1, false));
  ASSERT_EQ(nullptr, game.connect(island3, island1, true));

  ASSERT_EQ(nullptr, game.connect(island3, island4, false));
  ASSERT_EQ(nullptr, game.connect(island3, island4, true));

  ASSERT_EQ(nullptr, game.connect(island4, island2, false));
  ASSERT_EQ(nullptr, game.connect(island4, island2, true));

  ASSERT_EQ(nullptr, game.connect(island4, island3, false));
  ASSERT_EQ(nullptr, game.connect(island4, island3, true));

  ASSERT_EQ(nullptr, game.connect(island1, island1, false));
  ASSERT_EQ(nullptr, game.connect(island1, island1, true));

  ASSERT_EQ(nullptr, game.connect(island2, island2, false));
  ASSERT_EQ(nullptr, game.connect(island2, island2, true));

  ASSERT_EQ(nullptr, game.connect(island3, island3, false));
  ASSERT_EQ(nullptr, game.connect(island3, island3, true));

  ASSERT_EQ(nullptr, game.connect(island4, island4, false));
  ASSERT_EQ(nullptr, game.connect(island4, island4, true));

  ASSERT_EQ(nullptr, game.connect(island1, island4, false));
  ASSERT_EQ(nullptr, game.connect(island1, island4, false));

  ASSERT_EQ(nullptr, game.connect(island2, island3, false));
  ASSERT_EQ(nullptr, game.connect(island2, island3, false));

  ASSERT_EQ(nullptr, game.connect(island3, island2, false));
  ASSERT_EQ(nullptr, game.connect(island3, island2, false));

  ASSERT_EQ(nullptr, game.connect(island4, island1, false));
  ASSERT_EQ(nullptr, game.connect(island4, island1, false));


  Bridge* bridge15s = game.connect(island1, island5, false);
  ASSERT_NE(nullptr, bridge15s);
  ASSERT_EQ(island1, bridge15s->_one);
  ASSERT_EQ(island5, bridge15s->_two);
  ASSERT_FALSE(bridge15s->_doubleBridge);
  ASSERT_EQ(island1->_bridges[0], bridge15s);
  ASSERT_EQ(1, island1->_bridges.size());
  ASSERT_EQ(island5->_bridges[0], bridge15s);
  ASSERT_EQ(1, island5->_bridges.size());

  EXPECT_TRUE(game._bridgeOccupation[0][1]);

  Bridge* oldBridge = nullptr;

  ASSERT_EQ(nullptr, game.connect(island1, island5, true, &oldBridge));
  ASSERT_EQ(island1->_bridges[0], bridge15s);
  ASSERT_EQ(1, island1->_bridges.size());
  ASSERT_EQ(island5->_bridges[0], bridge15s);
  ASSERT_EQ(1, island5->_bridges.size());

  EXPECT_TRUE(game._bridgeOccupation[0][1]);

  EXPECT_EQ(nullptr, oldBridge);

  Bridge* oldBridge15s = nullptr;

  ASSERT_EQ(nullptr, game.connect(island5, island1, true, &oldBridge15s));
  ASSERT_EQ(island5->_bridges[0], bridge15s);
  ASSERT_EQ(1, island5->_bridges.size());
  ASSERT_EQ(island1->_bridges[0], bridge15s);
  ASSERT_EQ(1, island1->_bridges.size());

  EXPECT_EQ(nullptr, oldBridge15s);

  Bridge* bridge51d = game.connect(island5, island1, false, &oldBridge15s);
  ASSERT_NE(nullptr, bridge51d);
  ASSERT_EQ(island5, bridge51d->_one);
  ASSERT_EQ(island1, bridge51d->_two);
  ASSERT_TRUE(bridge51d->_doubleBridge);
  ASSERT_EQ(island5->_bridges[0], bridge51d);
  ASSERT_EQ(1, island5->_bridges.size());
  ASSERT_EQ(island1->_bridges[0], bridge51d);
  ASSERT_EQ(1, island1->_bridges.size());

  EXPECT_EQ(bridge15s, oldBridge15s);
  delete bridge15s;

  EXPECT_TRUE(game._bridgeOccupation[0][1]);

  Bridge* bridge16s = game.connect(island1, island6, false, &oldBridge);
  ASSERT_NE(nullptr, bridge16s);
  ASSERT_EQ(island1, bridge16s->_one);
  ASSERT_EQ(island6, bridge16s->_two);
  ASSERT_FALSE(bridge16s->_doubleBridge);
  ASSERT_EQ(island1->_bridges[1], bridge16s);
  ASSERT_EQ(2, island1->_bridges.size());
  ASSERT_EQ(island6->_bridges[0], bridge16s);
  ASSERT_EQ(1, island6->_bridges.size());

  EXPECT_TRUE(game._bridgeOccupation[0][1]);
  EXPECT_TRUE(game._bridgeOccupation[1][0]);

  EXPECT_EQ(nullptr, oldBridge);

  Bridge* oldBridge16s = nullptr;


  ASSERT_EQ(nullptr, game.connect(island1, island6, true, &oldBridge16s));
  ASSERT_EQ(island1->_bridges[1], bridge16s);
  ASSERT_EQ(2, island1->_bridges.size());
  ASSERT_EQ(island6->_bridges[0], bridge16s);
  ASSERT_EQ(1, island6->_bridges.size());

  EXPECT_EQ(nullptr, oldBridge16s);

  ASSERT_EQ(nullptr, game.connect(island6, island1, true, &oldBridge16s));
  ASSERT_EQ(island6->_bridges[0], bridge16s);
  ASSERT_EQ(1, island6->_bridges.size());
  ASSERT_EQ(island1->_bridges[1], bridge16s);
  ASSERT_EQ(2, island1->_bridges.size());

  EXPECT_EQ(nullptr, oldBridge16s);

  Bridge* bridge61d = game.connect(island6, island1, false, &oldBridge16s);
  ASSERT_NE(nullptr, bridge61d);
  ASSERT_EQ(island6, bridge61d->_one);
  ASSERT_EQ(island1, bridge61d->_two);
  ASSERT_TRUE(bridge61d->_doubleBridge);
  ASSERT_EQ(island6->_bridges[0], bridge61d);
  ASSERT_EQ(1, island6->_bridges.size());
  ASSERT_EQ(island1->_bridges[1], bridge61d);
  ASSERT_EQ(2, island1->_bridges.size());

  EXPECT_TRUE(game._bridgeOccupation[0][1]);
  EXPECT_TRUE(game._bridgeOccupation[1][0]);

  EXPECT_EQ(bridge16s, oldBridge16s);
  delete bridge16s;
}

// _____________________________________________________________________________

TEST(BridgeTest, constructor) {
  Island island1(0, 0, 1);
  Island island2(0, 0, 8);
  Bridge bridge1(&island1, &island2, true);
  Bridge bridge2(&island2, &island1, false);

  EXPECT_TRUE(bridge1._doubleBridge);
  EXPECT_FALSE(bridge2._doubleBridge);

  EXPECT_EQ(&island1, bridge1._one);
  EXPECT_EQ(&island1, bridge2._two);
  EXPECT_EQ(&island2, bridge1._two);
  EXPECT_EQ(&island2, bridge2._one);
}

// _____________________________________________________________________________

TEST(DirectionTest, constructor) {
  Direction direction1(-100, 0);
  Direction direction2(0, -100);
  Direction direction3(100, 100);

  EXPECT_EQ(-100, direction1._xchange);
  EXPECT_EQ(0, direction1._ychange);

  EXPECT_EQ(0, direction2._xchange);
  EXPECT_EQ(-100, direction2._ychange);

  EXPECT_EQ(100, direction3._xchange);
  EXPECT_EQ(100, direction3._ychange);
}

// _____________________________________________________________________________

TEST(DirectionTest, predefinedValues) {
  EXPECT_EQ(0, Direction::UP._xchange);
  EXPECT_EQ(-1, Direction::UP._ychange);

  EXPECT_EQ(1, Direction::RIGHT._xchange);
  EXPECT_EQ(0, Direction::RIGHT._ychange);

  EXPECT_EQ(0, Direction::DOWN._xchange);
  EXPECT_EQ(1, Direction::DOWN._ychange);

  EXPECT_EQ(-1, Direction::LEFT._xchange);
  EXPECT_EQ(0, Direction::LEFT._ychange);
}

// _____________________________________________________________________________

TEST(IslandTest, constructor) {
  Island island1(-1, 0, 255);
  Island island2(231231412, 2904803980, 3);

  EXPECT_EQ(-1, island1._x);
  EXPECT_EQ(0, island1._y);
  EXPECT_EQ(255, island1._requiredBridges);

  EXPECT_EQ(231231412, island2._x);
  EXPECT_EQ(2904803980, island2._y);
  EXPECT_EQ(3, island2._requiredBridges);
}

// _____________________________________________________________________________

TEST(IslandTest, missingConnections) {
  Island island0(0, 0, 0);
  Island island1(1, 1, 1);
  Island island2(1, 4, 2);
  Island island3(2, 9, 3);
  Island island4(3, 16, 4);
  Island island5(5, 25, 5);
  Island island6(8, 36, 6);
  Island island7(13, 49, 7);
  Island island8(21, 64, 8);
  Island island9(34, 81, 9);

  EXPECT_EQ(0, island0.missingConnections());
  EXPECT_EQ(1, island1.missingConnections());
  EXPECT_EQ(2, island2.missingConnections());
  EXPECT_EQ(3, island3.missingConnections());
  EXPECT_EQ(4, island4.missingConnections());
  EXPECT_EQ(5, island5.missingConnections());
  EXPECT_EQ(6, island6.missingConnections());
  EXPECT_EQ(7, island7.missingConnections());
  EXPECT_EQ(8, island8.missingConnections());
  EXPECT_EQ(9, island9.missingConnections());

  Bridge dummy1(nullptr, nullptr, false);
  Bridge dummy2(nullptr, nullptr, true);

  island0._bridges.push_back(&dummy1);
  EXPECT_EQ(-1, island0.missingConnections());

  island0._bridges.clear();
  island0._bridges.push_back(&dummy2);
  EXPECT_EQ(-2, island0.missingConnections());

  island0._bridges.clear();

  island5._bridges.push_back(&dummy1);
  island5._bridges.push_back(&dummy2);
  island5._bridges.push_back(&dummy2);

  EXPECT_EQ(0, island5.missingConnections());

  island5._bridges.clear();

  island8._bridges.push_back(&dummy2);
  island8._bridges.push_back(&dummy2);
  island8._bridges.push_back(&dummy2);
  island8._bridges.push_back(&dummy2);
  island8._bridges.push_back(&dummy2);

  EXPECT_EQ(-2, island8.missingConnections());

  island8._bridges.clear();
}

// _____________________________________________________________________________

TEST(IslandTest, isConnected) {
  Island islanda(0, 0, 1);
  Island islandb(8, 8, 8);

  EXPECT_EQ(0, islanda.isConnected(&islandb));
  EXPECT_EQ(0, islandb.isConnected(&islanda));

  Bridge bridge(&islanda, &islandb, false);
  islanda._bridges.push_back(&bridge);
  islandb._bridges.push_back(&bridge);

  EXPECT_EQ(1, islanda.isConnected(&islandb));
  EXPECT_EQ(1, islandb.isConnected(&islanda));

  islanda._bridges.push_back(&bridge);
  islandb._bridges.push_back(&bridge);

  EXPECT_EQ(1, islanda.isConnected(&islandb));
  EXPECT_EQ(1, islandb.isConnected(&islanda));

  islanda._bridges.clear();
  islandb._bridges.clear();

  Bridge bridge2(&islanda, &islandb, true);
  islanda._bridges.push_back(&bridge2);
  islandb._bridges.push_back(&bridge2);

  EXPECT_EQ(2, islanda.isConnected(&islandb));
  EXPECT_EQ(2, islandb.isConnected(&islanda));

  islanda._bridges.push_back(&bridge2);
  islandb._bridges.push_back(&bridge2);

  EXPECT_EQ(2, islanda.isConnected(&islandb));
  EXPECT_EQ(2, islandb.isConnected(&islanda));

  islanda._bridges.clear();
  islandb._bridges.clear();
}

// _____________________________________________________________________________

TEST(IslandTest, findDirection) {
  Island island1(0, 0, 1);
  Island island2(2, 0, 3);
  Island island3(0, 2, 5);
  Island island4(2, 2, 7);

  EXPECT_EQ(&Direction::RIGHT, island1.findDirection(&island2));
  EXPECT_EQ(&Direction::LEFT, island2.findDirection(&island1));

  EXPECT_EQ(&Direction::RIGHT, island3.findDirection(&island4));
  EXPECT_EQ(&Direction::LEFT, island4.findDirection(&island3));

  EXPECT_EQ(&Direction::DOWN, island1.findDirection(&island3));
  EXPECT_EQ(&Direction::UP, island3.findDirection(&island1));

  EXPECT_EQ(&Direction::DOWN, island2.findDirection(&island4));
  EXPECT_EQ(&Direction::UP, island4.findDirection(&island2));
}

// _____________________________________________________________________________

TEST(IslandTest, isCorrectlyAlignedWith) {
  Island i00(0, 0, 1);
  Island i10(1, 0, 2);
  Island i20(2, 0, 3);
  Island i01(0, 1, 4);
  Island i11(1, 1, 5);
  Island i21(2, 1, 6);
  Island i02(0, 2, 7);
  Island i12(1, 2, 8);
  Island i22(2, 2, 9);

  EXPECT_FALSE(i00.isCorrectlyAlignedWith(&i00));
  EXPECT_FALSE(i00.isCorrectlyAlignedWith(&i10));
  EXPECT_FALSE(i00.isCorrectlyAlignedWith(&i10));
  EXPECT_TRUE(i00.isCorrectlyAlignedWith(&i20));
  EXPECT_FALSE(i00.isCorrectlyAlignedWith(&i01));
  EXPECT_FALSE(i00.isCorrectlyAlignedWith(&i11));
  EXPECT_FALSE(i00.isCorrectlyAlignedWith(&i21));
  EXPECT_TRUE(i00.isCorrectlyAlignedWith(&i02));
  EXPECT_FALSE(i00.isCorrectlyAlignedWith(&i12));
  EXPECT_FALSE(i00.isCorrectlyAlignedWith(&i22));

  EXPECT_FALSE(i10.isCorrectlyAlignedWith(&i00));
  EXPECT_FALSE(i10.isCorrectlyAlignedWith(&i10));
  EXPECT_FALSE(i10.isCorrectlyAlignedWith(&i10));
  EXPECT_FALSE(i10.isCorrectlyAlignedWith(&i20));
  EXPECT_FALSE(i10.isCorrectlyAlignedWith(&i01));
  EXPECT_FALSE(i10.isCorrectlyAlignedWith(&i11));
  EXPECT_FALSE(i10.isCorrectlyAlignedWith(&i21));
  EXPECT_FALSE(i10.isCorrectlyAlignedWith(&i02));
  EXPECT_TRUE(i10.isCorrectlyAlignedWith(&i12));
  EXPECT_FALSE(i10.isCorrectlyAlignedWith(&i22));

  EXPECT_TRUE(i20.isCorrectlyAlignedWith(&i00));
  EXPECT_FALSE(i20.isCorrectlyAlignedWith(&i10));
  EXPECT_FALSE(i20.isCorrectlyAlignedWith(&i10));
  EXPECT_FALSE(i20.isCorrectlyAlignedWith(&i20));
  EXPECT_FALSE(i20.isCorrectlyAlignedWith(&i01));
  EXPECT_FALSE(i20.isCorrectlyAlignedWith(&i11));
  EXPECT_FALSE(i20.isCorrectlyAlignedWith(&i21));
  EXPECT_FALSE(i20.isCorrectlyAlignedWith(&i02));
  EXPECT_FALSE(i20.isCorrectlyAlignedWith(&i12));
  EXPECT_TRUE(i20.isCorrectlyAlignedWith(&i22));

  EXPECT_FALSE(i01.isCorrectlyAlignedWith(&i00));
  EXPECT_FALSE(i01.isCorrectlyAlignedWith(&i10));
  EXPECT_FALSE(i01.isCorrectlyAlignedWith(&i10));
  EXPECT_FALSE(i01.isCorrectlyAlignedWith(&i20));
  EXPECT_FALSE(i01.isCorrectlyAlignedWith(&i01));
  EXPECT_FALSE(i01.isCorrectlyAlignedWith(&i11));
  EXPECT_TRUE(i01.isCorrectlyAlignedWith(&i21));
  EXPECT_FALSE(i01.isCorrectlyAlignedWith(&i02));
  EXPECT_FALSE(i01.isCorrectlyAlignedWith(&i12));
  EXPECT_FALSE(i01.isCorrectlyAlignedWith(&i22));

  EXPECT_FALSE(i11.isCorrectlyAlignedWith(&i00));
  EXPECT_FALSE(i11.isCorrectlyAlignedWith(&i10));
  EXPECT_FALSE(i11.isCorrectlyAlignedWith(&i10));
  EXPECT_FALSE(i11.isCorrectlyAlignedWith(&i20));
  EXPECT_FALSE(i11.isCorrectlyAlignedWith(&i01));
  EXPECT_FALSE(i11.isCorrectlyAlignedWith(&i11));
  EXPECT_FALSE(i11.isCorrectlyAlignedWith(&i21));
  EXPECT_FALSE(i11.isCorrectlyAlignedWith(&i02));
  EXPECT_FALSE(i11.isCorrectlyAlignedWith(&i12));
  EXPECT_FALSE(i11.isCorrectlyAlignedWith(&i22));

  EXPECT_FALSE(i21.isCorrectlyAlignedWith(&i00));
  EXPECT_FALSE(i21.isCorrectlyAlignedWith(&i10));
  EXPECT_FALSE(i21.isCorrectlyAlignedWith(&i10));
  EXPECT_FALSE(i21.isCorrectlyAlignedWith(&i20));
  EXPECT_TRUE(i21.isCorrectlyAlignedWith(&i01));
  EXPECT_FALSE(i21.isCorrectlyAlignedWith(&i11));
  EXPECT_FALSE(i21.isCorrectlyAlignedWith(&i21));
  EXPECT_FALSE(i21.isCorrectlyAlignedWith(&i02));
  EXPECT_FALSE(i21.isCorrectlyAlignedWith(&i12));
  EXPECT_FALSE(i21.isCorrectlyAlignedWith(&i22));

  EXPECT_TRUE(i02.isCorrectlyAlignedWith(&i00));
  EXPECT_FALSE(i02.isCorrectlyAlignedWith(&i10));
  EXPECT_FALSE(i02.isCorrectlyAlignedWith(&i10));
  EXPECT_FALSE(i02.isCorrectlyAlignedWith(&i20));
  EXPECT_FALSE(i02.isCorrectlyAlignedWith(&i01));
  EXPECT_FALSE(i02.isCorrectlyAlignedWith(&i11));
  EXPECT_FALSE(i02.isCorrectlyAlignedWith(&i21));
  EXPECT_FALSE(i02.isCorrectlyAlignedWith(&i02));
  EXPECT_FALSE(i02.isCorrectlyAlignedWith(&i12));
  EXPECT_TRUE(i02.isCorrectlyAlignedWith(&i22));

  EXPECT_FALSE(i12.isCorrectlyAlignedWith(&i00));
  EXPECT_TRUE(i12.isCorrectlyAlignedWith(&i10));
  EXPECT_TRUE(i12.isCorrectlyAlignedWith(&i10));
  EXPECT_FALSE(i12.isCorrectlyAlignedWith(&i20));
  EXPECT_FALSE(i12.isCorrectlyAlignedWith(&i01));
  EXPECT_FALSE(i12.isCorrectlyAlignedWith(&i11));
  EXPECT_FALSE(i12.isCorrectlyAlignedWith(&i21));
  EXPECT_FALSE(i12.isCorrectlyAlignedWith(&i02));
  EXPECT_FALSE(i12.isCorrectlyAlignedWith(&i12));
  EXPECT_FALSE(i12.isCorrectlyAlignedWith(&i22));

  EXPECT_FALSE(i22.isCorrectlyAlignedWith(&i00));
  EXPECT_FALSE(i22.isCorrectlyAlignedWith(&i10));
  EXPECT_FALSE(i22.isCorrectlyAlignedWith(&i10));
  EXPECT_TRUE(i22.isCorrectlyAlignedWith(&i20));
  EXPECT_FALSE(i22.isCorrectlyAlignedWith(&i01));
  EXPECT_FALSE(i22.isCorrectlyAlignedWith(&i11));
  EXPECT_FALSE(i22.isCorrectlyAlignedWith(&i21));
  EXPECT_TRUE(i22.isCorrectlyAlignedWith(&i02));
  EXPECT_FALSE(i22.isCorrectlyAlignedWith(&i12));
  EXPECT_FALSE(i22.isCorrectlyAlignedWith(&i22));
}

// _____________________________________________________________________________

TEST(IslandViewTest, constructor) {
  std::unordered_map<uint32_t, Island*> islands;
  IslandView view(islands);

  EXPECT_EQ(&islands, &view._islands);
}

// _____________________________________________________________________________

TEST(IslandViewTest, size) {
  std::unordered_map<uint32_t, Island*> islands;
  IslandView view(islands);

  EXPECT_EQ(0, view.size());

  islands[0] = nullptr;
  EXPECT_EQ(1, view.size());

  islands[1] = nullptr;
  EXPECT_EQ(2, view.size());

  islands[2] = nullptr;
  EXPECT_EQ(3, view.size());

  islands.clear();
  EXPECT_EQ(0, view.size());
}

// _____________________________________________________________________________

TEST(IslandViewTest, testIteration) {
  std::unique_ptr<Island> island1(new Island(0, 0, 0));
  std::unique_ptr<Island> island2(new Island(1, 0, 0));
  std::unique_ptr<Island> island3(new Island(2, 0, 0));
  std::vector<Island*> reference = {
    island1.get(),
    island2.get(),
    island3.get()
  };
  std::sort(reference.begin(), reference.end());
  std::unordered_map<uint32_t, Island*> islands = {
    { 0, island1.get() },
    { 1, island2.get() },
    { 2, island3.get() }
  };

  IslandView view(islands);
  std::vector<Island*> viewVector(view.begin(), view.end());

  std::sort(viewVector.begin(), viewVector.end());

  ASSERT_EQ(3, reference.size());
  EXPECT_EQ(reference[0], viewVector[0]);
  EXPECT_EQ(reference[1], viewVector[1]);
  EXPECT_EQ(reference[2], viewVector[2]);
}

// _____________________________________________________________________________

TEST(GameTest, islandVectorToMap) {
  Game game({ Island(4, 4, 0) });
  auto map = game.islandVectorToMap({
    Island(0, 0, 0),
    Island(1, 1, 0),
    Island(2, 3, 0)
  });

  ASSERT_EQ(3, map.size());
  EXPECT_EQ(0, map[0]->_x);
  EXPECT_EQ(0, map[0]->_y);
  EXPECT_EQ(1, map[1 + 1 * 5]->_x);
  EXPECT_EQ(1, map[1 + 1 * 5]->_y);
  EXPECT_EQ(2, map[2 + 3 * 5]->_x);
  EXPECT_EQ(3, map[2 + 3 * 5]->_y);

  for (const auto &entry : map) {
    delete entry.second;
  }
}

// _____________________________________________________________________________

TEST(GameTest, registerBridge) {
  Game game({
    Island(0, 0, 8),
    Island(3, 0, 1),
    Island(0, 3, 2)
  });
  auto island1 = game.getIsland(0, 0);
  auto island2 = game.getIsland(3, 0);
  auto island3 = game.getIsland(0, 3);

  Bridge bridge1(island1, island2, false);
  Bridge bridge2(island1, island3, true);

  game.registerBridge(&bridge1, true);
  EXPECT_FALSE(game._bridgeOccupation[0][0]);
  EXPECT_TRUE(game._bridgeOccupation[1][0]);
  EXPECT_TRUE(game._bridgeOccupation[2][0]);
  EXPECT_FALSE(game._bridgeOccupation[3][0]);
  EXPECT_FALSE(game._bridgeOccupation[0][1]);
  EXPECT_FALSE(game._bridgeOccupation[0][2]);
  EXPECT_FALSE(game._bridgeOccupation[0][3]);

  game.registerBridge(&bridge2, true);
  EXPECT_FALSE(game._bridgeOccupation[0][0]);
  EXPECT_TRUE(game._bridgeOccupation[1][0]);
  EXPECT_TRUE(game._bridgeOccupation[2][0]);
  EXPECT_FALSE(game._bridgeOccupation[3][0]);
  EXPECT_TRUE(game._bridgeOccupation[0][1]);
  EXPECT_TRUE(game._bridgeOccupation[0][2]);
  EXPECT_FALSE(game._bridgeOccupation[0][3]);

  game.registerBridge(&bridge1, false);
  EXPECT_FALSE(game._bridgeOccupation[0][0]);
  EXPECT_FALSE(game._bridgeOccupation[1][0]);
  EXPECT_FALSE(game._bridgeOccupation[2][0]);
  EXPECT_FALSE(game._bridgeOccupation[3][0]);
  EXPECT_TRUE(game._bridgeOccupation[0][1]);
  EXPECT_TRUE(game._bridgeOccupation[0][2]);
  EXPECT_FALSE(game._bridgeOccupation[0][3]);

  game.registerBridge(&bridge2, false);
  EXPECT_FALSE(game._bridgeOccupation[0][0]);
  EXPECT_FALSE(game._bridgeOccupation[1][0]);
  EXPECT_FALSE(game._bridgeOccupation[2][0]);
  EXPECT_FALSE(game._bridgeOccupation[3][0]);
  EXPECT_FALSE(game._bridgeOccupation[0][1]);
  EXPECT_FALSE(game._bridgeOccupation[0][2]);
  EXPECT_FALSE(game._bridgeOccupation[0][3]);
}

// _____________________________________________________________________________

TEST(GameTest, findAccessibleIsland) {
  Game game({
    Island(1, 1, 4),
    Island(3, 1, 2),
    Island(1, 3, 2),
    Island(2, 0, 2),
    Island(2, 4, 2)
  });
  auto island1 = game.getIsland(1, 1);
  auto island2 = game.getIsland(3, 1);
  auto island3 = game.getIsland(1, 3);
  auto island4 = game.getIsland(2, 0);
  auto island5 = game.getIsland(2, 4);

  EXPECT_EQ(island2, game.findAccessibleIsland(*island1, Direction::RIGHT));
  EXPECT_EQ(island3, game.findAccessibleIsland(*island1, Direction::DOWN));

  EXPECT_EQ(island1, game.findAccessibleIsland(*island2, Direction::LEFT));
  EXPECT_EQ(island1, game.findAccessibleIsland(*island3, Direction::UP));

  auto bridge1 = game.connect(island1, island2, false);

  EXPECT_EQ(island2, game.findAccessibleIsland(*island1, Direction::RIGHT));
  EXPECT_EQ(island3, game.findAccessibleIsland(*island1, Direction::DOWN));

  EXPECT_EQ(island1, game.findAccessibleIsland(*island2, Direction::LEFT));
  EXPECT_EQ(island1, game.findAccessibleIsland(*island3, Direction::UP));

  bridge1 = game.connect(island1, island2, false);

  EXPECT_EQ(island2, game.findAccessibleIsland(*island1, Direction::RIGHT));
  EXPECT_EQ(island3, game.findAccessibleIsland(*island1, Direction::DOWN));

  EXPECT_EQ(island1, game.findAccessibleIsland(*island2, Direction::LEFT));
  EXPECT_EQ(island1, game.findAccessibleIsland(*island3, Direction::UP));

  auto bridge2 = game.connect(island1, island3, false);

  EXPECT_EQ(island2, game.findAccessibleIsland(*island1, Direction::RIGHT));
  EXPECT_EQ(island3, game.findAccessibleIsland(*island1, Direction::DOWN));

  EXPECT_EQ(island1, game.findAccessibleIsland(*island2, Direction::LEFT));
  EXPECT_EQ(island1, game.findAccessibleIsland(*island3, Direction::UP));

  bridge2 = game.connect(island1, island3, false);

  EXPECT_EQ(island2, game.findAccessibleIsland(*island1, Direction::RIGHT));
  EXPECT_EQ(island3, game.findAccessibleIsland(*island1, Direction::DOWN));

  EXPECT_EQ(island1, game.findAccessibleIsland(*island2, Direction::LEFT));
  EXPECT_EQ(island1, game.findAccessibleIsland(*island3, Direction::UP));

  game.disconnect(bridge1);
  game.disconnect(bridge2);
  delete bridge1;
  delete bridge2;
  // Block connection
  game.connect(island4, island5, false);

  EXPECT_EQ(nullptr, game.findAccessibleIsland(*island1, Direction::RIGHT));
  EXPECT_EQ(island3, game.findAccessibleIsland(*island1, Direction::DOWN));

  EXPECT_EQ(nullptr, game.findAccessibleIsland(*island2, Direction::LEFT));
  EXPECT_EQ(island1, game.findAccessibleIsland(*island3, Direction::UP));
}

// _____________________________________________________________________________

TEST(GameTest, findAccessibleIslands) {
  Game game({
    Island(2, 0, 1),
    Island(3, 1, 2),
    Island(2, 2, 6),
    Island(4, 2, 2),
    Island(3, 3, 2),
    Island(2, 4, 0)
  });
  auto island1 = game.getIsland(2, 0);
  auto island2 = game.getIsland(3, 1);
  auto island3 = game.getIsland(2, 2);
  auto island5 = game.getIsland(3, 3);
  auto island6 = game.getIsland(2, 4);

  game.connect(island2, island5, false);

  auto result = game.findAccessibleIslands(*island3);

  ASSERT_EQ(2, result.size());
  EXPECT_EQ(island1, result[0]);
  EXPECT_EQ(island6, result[1]);
}

// _____________________________________________________________________________

TEST(GameTest, getIslands) {
  Game game({
    Island(0, 0, 0),
    Island(1, 0, 0),
    Island(2, 0, 0)
  });
  std::vector<Island*> reference = {
    game.getIsland(0, 0),
    game.getIsland(1, 0),
    game.getIsland(2, 0)
  };
  std::sort(reference.begin(), reference.end());

  auto view = game.getIslands();
  std::vector<Island*> viewVector(view.begin(), view.end());
  std::sort(viewVector.begin(), viewVector.end());

  ASSERT_EQ(3, reference.size());
  EXPECT_EQ(reference[0], viewVector[0]);
  EXPECT_EQ(reference[1], viewVector[1]);
  EXPECT_EQ(reference[2], viewVector[2]);
}

// _____________________________________________________________________________

TEST(GameTest, isSolved) {
  Game game({
    Island(0, 0, 2),
    Island(2, 0, 2),
    Island(0, 2, 2),
    Island(2, 2, 2)
  });
  auto island1 = game.getIsland(0, 0);
  auto island2 = game.getIsland(2, 0);
  auto island3 = game.getIsland(0, 2);
  auto island4 = game.getIsland(2, 2);

  EXPECT_FALSE(game.isSolved());

  game.connect(island1, island2, false);
  EXPECT_FALSE(game.isSolved());

  game.connect(island3, island4, false);
  EXPECT_FALSE(game.isSolved());

  auto bridge1 = game.connect(island1, island2, false);
  EXPECT_FALSE(game.isSolved());

  auto bridge2 = game.connect(island3, island4, false);
  EXPECT_FALSE(game.isSolved());

  game.disconnect(bridge1);
  game.disconnect(bridge2);
  auto bridge3 = game.connect(island1, island2, false);
  auto bridge4 = game.connect(island1, island3, false);
  auto bridge5 = game.connect(island2, island4, false);
  EXPECT_FALSE(game.isSolved());

  auto bridge6 = game.connect(island3, island4, false);
  EXPECT_TRUE(game.isSolved());

  game.disconnect(bridge3);
  game.disconnect(bridge4);
  game.disconnect(bridge5);
  game.disconnect(bridge6);
  EXPECT_FALSE(game.isSolved());
  delete bridge1;
  delete bridge2;
  delete bridge3;
  delete bridge4;
  delete bridge5;
  delete bridge6;
}

// _____________________________________________________________________________

TEST(GameTest, maxBandwidth) {
  Game game({
    Island(0, 0, 1),
    Island(2, 0, 3),
    Island(4, 0, 2),
    Island(6, 0, 2)
  });

  auto island1 = game.getIsland(0, 0);
  auto island2 = game.getIsland(2, 0);
  auto island3 = game.getIsland(4, 0);
  auto island4 = game.getIsland(6, 0);

  EXPECT_EQ(1, game.maxBandwidth(island2, island1));
  EXPECT_EQ(1, game.maxBandwidth(island1, island2));

  EXPECT_EQ(1, game.maxBandwidth(island3, island4));
  EXPECT_EQ(1, game.maxBandwidth(island4, island3));

  EXPECT_EQ(2, game.maxBandwidth(island2, island3));
  EXPECT_EQ(2, game.maxBandwidth(island3, island2));

  game.connect(island1, island2, false);

  EXPECT_EQ(1, game.maxBandwidth(island2, island3));
  EXPECT_EQ(1, game.maxBandwidth(island3, island2));

  Game game2({
    Island(0, 0, 1),
    Island(1, 0, 1)
  });

  auto island5 = game2.getIsland(0, 0);
  auto island6 = game2.getIsland(1, 0);

  EXPECT_EQ(1, game2.maxBandwidth(island5, island6));
  EXPECT_EQ(1, game2.maxBandwidth(island6, island5));

  Game game3({
    Island(0, 0, 2),
    Island(1, 0, 2)
  });

  auto island7 = game3.getIsland(0, 0);
  auto island8 = game3.getIsland(1, 0);

  EXPECT_EQ(2, game3.maxBandwidth(island7, island8));
  EXPECT_EQ(2, game3.maxBandwidth(island8, island7));
}

// _____________________________________________________________________________

TEST(GameTest, isPartOfDisjunctGroup) {
  Game game({
    Island(0, 0, 1),
    Island(2, 0, 1)
  });
  auto island1 = game.getIsland(0, 0);
  auto island2 = game.getIsland(2, 0);
  game.connect(island1, island2, false);

  EXPECT_FALSE(game.isPartOfDisjunctGroup(island1));
  EXPECT_FALSE(game.isPartOfDisjunctGroup(island2));

  Game game2({
    Island(0, 0, 1),
    Island(2, 0, 1),
    Island(4, 0, 1)
  });
  auto island3 = game2.getIsland(0, 0);
  auto island4 = game2.getIsland(2, 0);
  game2.connect(island3, island4, false);

  EXPECT_TRUE(game2.isPartOfDisjunctGroup(island3));
  EXPECT_TRUE(game2.isPartOfDisjunctGroup(island4));
}

// _____________________________________________________________________________

TEST(GameTest, wouldCreateDisjunctGroup) {
  Game game({
    Island(0, 0, 1),
    Island(2, 0, 1)
  });
  auto island1 = game.getIsland(0, 0);
  auto island2 = game.getIsland(2, 0);

  EXPECT_FALSE(game.wouldCreateDisjunctGroup(island1, island2));

  Game game2({
    Island(0, 0, 1),
    Island(2, 0, 1),
    Island(4, 0, 1)
  });
  auto island3 = game2.getIsland(0, 0);
  auto island4 = game2.getIsland(2, 0);

  EXPECT_TRUE(game2.wouldCreateDisjunctGroup(island3, island4));
}

// _____________________________________________________________________________

TEST(GameTest, wouldCreateDisjunctGroupDeque) {
  Game game({
    Island(0, 0, 1),
    Island(2, 0, 2),
    Island(4, 0, 1)
  });
  auto island1 = game.getIsland(0, 0);
  auto island2 = game.getIsland(2, 0);
  auto island3 = game.getIsland(4, 0);

  std::deque<Island*> deque = {
    island1,
    island2,
    island3
  };

  EXPECT_FALSE(game.wouldCreateDisjunctGroup(&deque));

  Game game2({
    Island(0, 0, 1),
    Island(2, 0, 2),
    Island(4, 0, 1),
    Island(6, 0, 1)
  });
  auto island4 = game2.getIsland(0, 0);
  auto island5 = game2.getIsland(2, 0);
  auto island6 = game2.getIsland(4, 0);

  std::deque<Island*> deque2 = {
    island4,
    island5,
    island6
  };

  EXPECT_TRUE(game2.wouldCreateDisjunctGroup(&deque2));
}

// _____________________________________________________________________________

TEST(GameTest, containsIsland) {
  Island island1(1, 1, 2);
  Island island2(1, 1, 0);
  Island island3(2, 2, 2);
  Game game({ island1 });

  EXPECT_TRUE(game.containsIsland(game.getIsland(1, 1)));

  EXPECT_FALSE(game.containsIsland(&island1));
  EXPECT_FALSE(game.containsIsland(&island2));
  EXPECT_FALSE(game.containsIsland(&island3));
}

// _____________________________________________________________________________

TEST(GameTest, disconnect) {
  Game game({
    Island(0, 0, 2),
    Island(2, 0, 2)
  });
  auto island1 = game.getIsland(0, 0);
  auto island2 = game.getIsland(2, 0);

  auto bridge = game.connect(island1, island2, true);

  game.disconnect(bridge);

  delete bridge;

  EXPECT_EQ(0, island1->_bridges.size());
  EXPECT_EQ(0, island2->_bridges.size());

  EXPECT_FALSE(game._bridgeOccupation[0][0]);
  EXPECT_FALSE(game._bridgeOccupation[1][0]);
  EXPECT_FALSE(game._bridgeOccupation[2][0]);
}

// _____________________________________________________________________________

TEST(GameTest, reconnect) {
  Game game({
    Island(0, 0, 2),
    Island(2, 0, 2)
  });
  auto island1 = game.getIsland(0, 0);
  auto island2 = game.getIsland(2, 0);

  auto bridge = game.connect(island1, island2, true);
  game.disconnect(bridge);
  game.reconnect(bridge);

  ASSERT_EQ(1, island1->_bridges.size());
  ASSERT_EQ(1, island2->_bridges.size());

  ASSERT_EQ(bridge, island1->_bridges[0]);
  ASSERT_EQ(bridge, island2->_bridges[0]);

  EXPECT_FALSE(game._bridgeOccupation[0][0]);
  EXPECT_TRUE(game._bridgeOccupation[1][0]);
  EXPECT_FALSE(game._bridgeOccupation[2][0]);
}
