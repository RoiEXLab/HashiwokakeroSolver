#ifndef SOLVER_H_
#define SOLVER_H_

#include <gtest/gtest_prod.h>
#include <cstdint>
#include <deque>
#include <vector>
#include <utility>
#include <tuple>
#include <unordered_map>
#include "./Game.h"

// _____________________________________________________________________________

// Main class to solve a game instance
class Solver {
  FRIEND_TEST(SolverTest, constructor);
  FRIEND_TEST(SolverTest, revertSteps);
  FRIEND_TEST(SolverTest, eliminateObvious);
  FRIEND_TEST(SolverTest, extractValidGaps);
  FRIEND_TEST(SolverTest, solvePrivate);
  FRIEND_TEST(SolverTest, tryGaps);
  // store the game instance
  Game* const _game;

  // cheap solving function that tries to use simple
  // tricks to solve the game, stricly ignores connections
  // that are pointed out in the first parameter,
  // stores the individual steps in the second parameter
  bool eliminateObvious(const std::unordered_multimap<Island*, Island*>&,
    std::deque<std::pair<Bridge*, Bridge*>>*);

  // function to revert the provided steps deque in order to try a different
  // step if the last step and following didn't work out so well
  void revertSteps(std::deque<std::pair<Bridge*, Bridge*>>*);

  // internal function to try to solve the given game instance recursively
  // does only try gaps in the given deque in order to reduce the tree
  // of possible combinations
  // The first parameter contains the connections that remain that
  // still need to be tried
  // The second parameter contains the forbidden connections that
  // should be strictly ignored in order to improve performance by a lot
  // by not trying connections that are guaranteed to be wrong twice.
  // All individual steps are stored in the third parameter
  bool solve(
    const std::deque<std::tuple<Island*, Island*, const Direction*>>&,
    const std::unordered_multimap<Island*, Island*>&,
    std::deque<std::pair<Bridge*, Bridge*>>*);

  // Checks if the passed Island Gaps steps can still be made
  // without a logical contradiction i.e. an unsolvable Game
  // if the steps cannot be made, they are not in the returned
  // deque, this is to massively cut down the tree of possible
  // combinations
  // Only processes the connections "in" the first parameter
  // and adds all connections that don't make it into
  // the return value into the second parameter
  std::deque<std::tuple<Island*, Island*, const Direction*>>
    extractValidGaps(const std::deque<std::tuple<Island*, Island*,
    const Direction*>>&, std::unordered_multimap<Island*, Island*>*) const;

  // Tries the gaps between the islands in the tuple of the passed deque
  // recursively, if a solution can be made this way, the function
  // returns true, false if a logical contradiction occurs
  // The first parameter indicates which connections still need to be processed
  // The if any steps are encountered that don't work, they are filled
  // into the second parameter
  bool tryGaps(std::deque<std::tuple<Island*, Island*, const Direction*>>*,
  std::unordered_multimap<Island*, Island*>*);

 public:
  // constructs a Solver object using a given game state
  explicit Solver(Game* game): _game(game) {}
  // public method which modifies the given game state
  // and returns true if a solution could be found
  // and false otherwise calls an internal function with the same name
  // with generated arguments in order to start
  bool solve();
};

// _____________________________________________________________________________

// A Helper class to do the 100% obligatory connections for the current Island
class SmartConnector {
  FRIEND_TEST(SmartConnectorTest, constructor);
  FRIEND_TEST(SmartConnectorTest, maxBandwidthRegardingForbidden);
  FRIEND_TEST(SmartConnectorTest, createPair);
  FRIEND_TEST(SmartConnectorTest, createNeighbours);
  FRIEND_TEST(SmartConnectorTest, reversedFindConnection);
  FRIEND_TEST(SmartConnectorTest, findReverseConnectIslands);
  // The island to make connections to
  Island* const _island;
  // The game to operate on
  Game* const _game;
  // A vector of all reachable neighbours and their maximum
  // amount of connections they can make with the current island
  const std::vector<std::pair<Island*, int8_t>> _neighbours;

  // Returns 0 if this island combination is part of the provided 3rd parameter
  // Returns Game#maxBandwidth(Island*, Island*) otherwise
  int8_t maxBandwidthRegardingForbidden(Island*, Island*,
    const std::unordered_multimap<Island*, Island*>&) const;

  // Creates a pair entry to push to _neighbours based on the Island
  // combination and their value calculated by maxBandwidthRegardingForbidden
  std::pair<Island*, int8_t> createPair(Island*, Island*,
    const std::unordered_multimap<Island*, Island*>&) const;

  // Creates a neighbour vector to initialize the member variable of this class
  // creates a pair for every neighbour Island and pushes it to the resulting
  // vector
  const std::vector<std::pair<Island*, int8_t>> createNeighbours(
    const std::unordered_multimap<Island*, Island*>&) const;

  // A special tactic to find obligatory connections.
  // If all remaining connections were made to all other neighbours
  // and that would create an invalid state, this means at least a imple bridge
  // must be to the current neighbour
  // The first parameter is an optimization, so the current amount
  // of missing connections doesn't need to be calculated multiple times
  // Steps are filled into the second parameter
  void reversedFindConnection(int8_t, std::deque<std::pair<Bridge*, Bridge*>>*)
    const;

  // Helper function that finds all Islands that need to be connected to this
  // Islands for reversedFindConnection
  // The first parameter is an optimization, so the current amount
  // of missing connections doesn't need to be calculated multiple times
  std::vector<Island*> findReverseConnectIslands(int8_t) const;

 public:
  // Constructor, 1st the game instance to be modified, 2nd the Island
  // to be analyzed and made connections to, 3rd the forbidden connections
  SmartConnector(Game*, Island*,
    const std::unordered_multimap<Island*, Island*>&);

  // Connects the current island with neighbour Islands if the connection
  // must be made in any case
  // The first parameter is an optimization, so the current amount
  // of missing connections doesn't need to be calculated multiple times
  // The second parameter is filled with steps that are being made during
  // this function call
  bool connectSmart(int8_t, std::deque<std::pair<Bridge*, Bridge*>>*) const;

  // Returns how many connections the current Island could currently make
  // regardless of how many it should have in the end
  int8_t availableConnections() const;
};

#endif  // SOLVER_H_
