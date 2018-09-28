#ifndef GAME_H_
#define GAME_H_

#include <gtest/gtest_prod.h>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <utility>

// _____________________________________________________________________________

// Forward declarations for the Bridge class
class Island;
class Game;

// _____________________________________________________________________________

// A class representing a connection between 2 Islands.
// Bridges are immutable objects, so in order to change a connection
// we need to remove and recreate it
class Bridge {
  FRIEND_TEST(GameTest, connect);
  FRIEND_TEST(SolverTest, revertSteps);
  FRIEND_TEST(SolverTest, eliminateObvious);
  FRIEND_TEST(SmartConnectorTest, reversedFindConnection);
  FRIEND_TEST(SmartConnectorTest, connectSmart);
  FRIEND_TEST(SolverTest, solvePublicSolvable);
  FRIEND_TEST(SolverTest, solvePrivate);
  FRIEND_TEST(SolverTest, tryGaps);
  FRIEND_TEST(BridgeTest, constructor);
  FRIEND_TEST(IslandTest, missingConnections);
  FRIEND_TEST(IslandTest, isConnected);
  FRIEND_TEST(GameTest, registerBridge);
  friend class Game;
  friend class Island;
  friend class PlainPrinter;
  friend class XYPrinter;
  // source island
  Island* const _one;
  // destination island
  Island* const _two;
  // true if the bridge is a double bridge false otherwise
  const bool _doubleBridge;

  // construct a bridge width a fixed "width"
  // a bridge is immutable only the game instance is supposed
  // to create a bridge object
  Bridge(Island* one, Island* two, bool doubleBridge):
    _one(one), _two(two), _doubleBridge(doubleBridge) {}
};

// _____________________________________________________________________________

// Helper class to make direction related functions reusable
class Direction {
  FRIEND_TEST(DirectionTest, constructor);
  // Private constructor, so we only have 4 possible directions
  Direction(int8_t xchange, int8_t ychange):
  _xchange(xchange), _ychange(ychange) {}
 public:
  // Enum-like objects, the only possible directions
  static const Direction UP;
  static const Direction RIGHT;
  static const Direction DOWN;
  static const Direction LEFT;

  // How far a step in x direction goes for this direction
  const int8_t _xchange;
  // How far a step in y direction goes for this direction
  const int8_t _ychange;
};

// _____________________________________________________________________________

// Class representing an Island of the Game
class Island {
  FRIEND_TEST(GameTest, connect);
  FRIEND_TEST(SolverTest, revertSteps);
  FRIEND_TEST(SolverTest, eliminateObvious);
  FRIEND_TEST(SmartConnectorTest, reversedFindConnection);
  FRIEND_TEST(SolverTest, solvePublicSolvable);
  FRIEND_TEST(SolverTest, solvePublicUnsolvable);
  FRIEND_TEST(SolverTest, solvePrivate);
  FRIEND_TEST(SolverTest, tryGaps);
  FRIEND_TEST(IslandTest, missingConnections);
  FRIEND_TEST(IslandTest, isConnected);
  FRIEND_TEST(GameTest, disconnect);
  FRIEND_TEST(GameTest, reconnect);
  friend class Game;
  friend class Bridge;
  friend class PlainPrinter;
  friend class XYPrinter;

  // store all the bridges that come from or to this Island
  std::vector<Bridge*> _bridges;

 public:
  // x coordinate of the island
  const uint32_t _x;
  // y coordinate of the island
  const uint32_t _y;
  // how many bridges should be connected to this island
  const uint32_t _requiredBridges;

  // create an Island x and y are the positions
  // and requiredBridges set the amount of bridges this Island can hold
  explicit Island(const uint32_t x, const uint32_t y,
    const uint32_t requiredBridges):
    _x(x), _y(y), _requiredBridges(requiredBridges) {}

  // Returns how many connections this Island needs in order to be "full"
  int8_t missingConnections() const;

  // Returns the degree of connection between this and the provided Island.
  // If they are connected with a single bridge, this function
  // returns 1, for a double bridge 2 and 0 otherwise
  int8_t isConnected(Island*) const;

  // calculates the direction from this to the other Island and returns
  // a pointer to the appropriate direction object.
  // if the islands can't be connected in a line on the grid,
  // this function returns nullptr
  const Direction* findDirection(const Island*) const;

  // Returns true if this and the given island are relatively aligned to each
  // other so they could technically be connected to each other
  bool isCorrectlyAlignedWith(Island*) const;

  // deletes all bridges connected to this island
  ~Island();
};

// _____________________________________________________________________________

// Convinience class to expose a read only iterator publicly
class IslandView {
  FRIEND_TEST(IslandViewTest, constructor);
  FRIEND_TEST(IslandViewTest, size);
  FRIEND_TEST(IslandViewTest, testIteration);
  friend class Game;

  // iterator wrapper that maps a map iterator to the value iterator of the map
  struct unordered_map_value_iterator
    : public std::unordered_map<uint32_t, Island*>::const_iterator {
    unordered_map_value_iterator(std::unordered_map<uint32_t,
      Island*>::const_iterator src)
    : std::unordered_map<uint32_t, Island*>::const_iterator(std::move(src)) {}

    // override the indirection operator in order to provide
    // a view of the values of the map
    Island* const & operator*() const;
  };

  // the unordered_map containing the islands of the Game class
  const std::unordered_map<uint32_t, Island*> &_islands;

  // Constructor to construct an iterator over the values of the given map
  explicit IslandView(const std::unordered_map<uint32_t, Island*> &islands)
    : _islands(islands) {}

 public:
  // begin iterator for for-each loops
  unordered_map_value_iterator begin() const;
  // end iterator for for-each loops
  unordered_map_value_iterator end() const;
  // COnvinience size function to query the size of the underlying map
  size_t size() const;
};

// _____________________________________________________________________________

// Game object, the root object of all game related operations
class Game {
  FRIEND_TEST(GameTest, constructor);
  FRIEND_TEST(GameParserTest, parserPlain);
  FRIEND_TEST(GameParserTest, parserXY);
  FRIEND_TEST(GameParserTest, autoParsePlain);
  FRIEND_TEST(GameParserTest, autoParseXY);
  FRIEND_TEST(GameTest, connect);
  FRIEND_TEST(GameTest, islandVectorToMap);
  FRIEND_TEST(GameTest, registerBridge);
  FRIEND_TEST(GameTest, disconnect);
  FRIEND_TEST(GameTest, reconnect);
  friend class PlainPrinter;
  friend class XYPrinter;

  // width of the map
  const size_t _width;
  // height of the map
  const size_t _height;

  // store all islands of the map
  // with average constant access based on x-y coordinates
  const std::unordered_map<uint32_t, Island*> _islands;
  // a dynamic 2 dimensional array holding basic information about all
  // bridges on the field, this is more efficient than calculating
  // every time where the bridges are when this class tries to connect
  // two islands
  bool* const * const _bridgeOccupation;

  // Convinience function to "copy" a given vector of islands
  // into a O(1) access map for faster lookup
  std::unordered_map<uint32_t, Island*> islandVectorToMap(
    const std::vector<Island> &islands) const;

  // Removes (false) or adds (true) a bridge from the collision
  // cache based on the second parameter
  void registerBridge(Bridge*, bool);

 public:
  // Construct a game
  // Stores a given vector of Islands in this game instance
  explicit Game(const std::vector<Island>&);

  // returns an Island pointer pointing to the Island
  // at the specified x y coordinate
  // is nullptr if this island does not exist
  Island* getIsland(uint32_t, uint32_t) const;

  // returns a pointer to a reachable Island in the given direction
  // if no Island is reachable in this direction this function returns nullptr
  Island* findAccessibleIsland(const Island&, const Direction&) const;

  // returns a vector of Islands that are reachable from the given instance
  std::vector<Island*> findAccessibleIslands(const Island&) const;

  // const view of the islands map's values
  IslandView getIslands() const;

  // Checks if the current state is a solution to the game, returns true if so
  bool isSolved() const;

  // calculates how many connections between this and the other Island
  // could in theory be made until eihter of them is completely full
  // or 2 connections have been made in total, whichever happens first
  int8_t maxBandwidth(Island*, Island*) const;

  // Returns true if the provided island is part of a disjunct,
  // closed (i.e. all Islands have all of their bridge capacity maxed out)
  // component within the graph of bridges and islands
  bool isPartOfDisjunctGroup(Island*) const;

  // Checks if when the given islands would be
  // connected they would create a closed group
  // in the game which doesn't represent the complete graph
  bool wouldCreateDisjunctGroup(Island*, Island*) const;

    // Checks if when the given islands in the vector would be
    // connected they would create a closed group
    // in the game which doesn't represent the complete graph
  bool wouldCreateDisjunctGroup(std::deque<Island*>*) const;

  // Checks if the given Island is part of this game instance
  bool containsIsland(Island*);

  // connects two islands and create a bridge
  // does check if the other Island is aligned correctly,
  // does not check if the connection path is clear
  // note that if a bridge is already present, the connection degree will
  // be added to the new bridge, so calling this function 2 times
  // with false as the bool argument, will result in the same bridge
  // as calling one time using true instead
  // If a Bridge** is being passed, any old Bridge won't be deleted
  // but assigned to the Bridge* instead, so keep track of your bridge pointers
  Bridge* connect(Island*, Island*, bool, Bridge** = nullptr);

  // Removes this Bridge from the Game, and deregisters the bridge
  void disconnect(Bridge*);
  // Connects this Bridge to the Game, and registers the bridge
  void reconnect(Bridge*);

  // Destruct a game deletes all stored Islands
  ~Game();
};

#endif  // GAME_H_
