#include <algorithm>
#include <utility>
#include <deque>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include "./Solver.h"

// _____________________________________________________________________________

bool Solver::eliminateObvious(
  const std::unordered_multimap<Island*, Island*> &forbiddenConnections,
  std::deque<std::pair<Bridge*, Bridge*>>* steps) {
  size_t iterationsWithoutChange = 0;
  const auto &islands = _game->getIslands();
  size_t islandCount = islands.size();
  while (true) {
    for (const auto &island : islands) {
      size_t stepAmount = steps->size();
      int8_t conn = island->missingConnections();
      if (conn != 0) {
        SmartConnector smart(_game, island, forbiddenConnections);
        if (!smart.connectSmart(conn, steps)) {
          return false;
        }
      }
      // Once we iterated over all islands without a single
      // change, our strategies no longer achieve anything
      if (steps->size() == stepAmount) {
        iterationsWithoutChange++;
      } else {
        iterationsWithoutChange = 0;
      }
      if (iterationsWithoutChange == islandCount) {
        return true;
      }
    }
  }
}

// _____________________________________________________________________________

// Helper function that deallocates the "overridden bridges" from the
// given step deque when a solution has been found
void deleteReplacedBridges(std::deque<std::pair<Bridge*, Bridge*>>* steps) {
  // Clear the steps, so we dont store any references to potentially
  // deallocated memory
  for (const auto &entry : *steps) {
    delete entry.second;
  }
}

// _____________________________________________________________________________

bool Solver::solve() {
  std::deque<std::tuple<Island*, Island*, const Direction*>> all;
  // Gather all connections that can be made on the map
  for (const auto &start : _game->getIslands()) {
    // We only need to check in 2 directions, otherwise we'd have some bridges
    // 2 times in the deque
    Island* down = _game->findAccessibleIsland(*start, Direction::DOWN);
    if (down != nullptr) {
      all.push_back(std::make_tuple(start, down, &Direction::DOWN));
    }
    Island* right = _game->findAccessibleIsland(*start, Direction::RIGHT);
    if (right != nullptr) {
      all.push_back(std::make_tuple(start, right, &Direction::RIGHT));
    }
  }
  std::unordered_multimap<Island*, Island*> forbidden;
  std::deque<std::pair<Bridge*, Bridge*>> steps;
  // Start recursion
  bool solved = solve(all, forbidden, &steps);
  deleteReplacedBridges(&steps);
  return solved;
}

// _____________________________________________________________________________

bool Solver::solve(const std::deque<std::tuple<Island*, Island*,
  const Direction*>> &possibleGaps,
  const std::unordered_multimap<Island*, Island*> &forbidden,
  std::deque<std::pair<Bridge*, Bridge*>>* steps) {
  if (eliminateObvious(forbidden, steps)) {
    if (_game->isSolved()) {
      // Game is solved, hooray
      return true;
    }

    std::unordered_multimap<Island*, Island*> curForbidden(forbidden);
    // Gather gaps that are still valid
    auto newGaps = extractValidGaps(possibleGaps, &curForbidden);
    // Try them exclusively
    return tryGaps(&newGaps, &curForbidden);
  }
  return false;
}

// _____________________________________________________________________________

std::deque<std::tuple<Island*, Island*, const Direction*>> Solver::
  extractValidGaps(const std::deque<std::tuple<Island*, Island*,
  const Direction*>> &possibleGaps,
  std::unordered_multimap<Island*, Island*>* forbidden) const {
  std::deque<std::tuple<Island*, Island*, const Direction*>> newGaps;
  for (const auto &entry : possibleGaps) {
    Island* start = std::get<0>(entry);
    Island* stop = std::get<1>(entry);
    if (_game->findAccessibleIsland(*start, *std::get<2>(entry)) == stop) {
      int8_t maxBandwidth = _game->maxBandwidth(start, stop);
      if (maxBandwidth > 0) {
        if (maxBandwidth != start->missingConnections()
            || maxBandwidth != stop->missingConnections()
            || !_game->wouldCreateDisjunctGroup(start, stop)) {
          newGaps.push_back(entry);
        } else {
          forbidden->insert(std::make_pair(start, stop));
        }
      }
    }
  }
  return newGaps;
}

// _____________________________________________________________________________

bool Solver::tryGaps(std::deque<std::tuple<Island*, Island*,
  const Direction*>>* newGaps,
  std::unordered_multimap<Island*, Island*>* forbidden) {
  // Consume all gaps in the deque
  while (!newGaps->empty()) {
    const auto &entry = newGaps->front();
    Island* start = std::get<0>(entry);
    Island* stop = std::get<1>(entry);
    Bridge* oldBridge = nullptr;
    Bridge* newBridge = _game->connect(start, stop, false, &oldBridge);

    std::deque<std::pair<Bridge*, Bridge*>> steps;
    steps.push_back({newBridge, oldBridge});
    if (solve(*newGaps, *forbidden, &steps)) {
      // Solved, hooray
      deleteReplacedBridges(&steps);
      return true;
    }
    // That didn't work, abort
    revertSteps(&steps);
    newGaps->pop_front();
    forbidden->insert(std::make_pair(start, stop));
  }
  return false;
}

// _____________________________________________________________________________

void Solver::revertSteps(std::deque<std::pair<Bridge*, Bridge*>>* steps) {
  while (!steps->empty()) {
    auto bridgePair = steps->back();
    steps->pop_back();
    _game->disconnect(bridgePair.first);
    delete bridgePair.first;
    if (bridgePair.second != nullptr) {
      // Old bridge was replaced, reattach it to the Game
      _game->reconnect(bridgePair.second);
    }
  }
}

// _____________________________________________________________________________

int8_t SmartConnector::maxBandwidthRegardingForbidden(Island* one, Island* two,
  const std::unordered_multimap<Island*, Island*> &forbidden) const {
  auto iterators = forbidden.equal_range(one);
  for (auto it = iterators.first; it != iterators.second; it++) {
    if (it->second == two) {
      return 0;
    }
  }
  return _game->maxBandwidth(one, two);
}

// _____________________________________________________________________________

std::pair<Island*, int8_t> SmartConnector::createPair(Island* one, Island* two,
  const std::unordered_multimap<Island*, Island*> &forbidden) const {
  return {_island == one ? two : one,
    maxBandwidthRegardingForbidden(one, two, forbidden)};
}

const std::vector<std::pair<Island*, int8_t>> SmartConnector::createNeighbours(
  const std::unordered_multimap<Island*, Island*> &forbidden) const {
  std::vector<std::pair<Island*, int8_t>> result;
  Island* north = _game->findAccessibleIsland(*_island, Direction::UP);
  Island* east = _game->findAccessibleIsland(*_island, Direction::RIGHT);
  Island* south = _game->findAccessibleIsland(*_island, Direction::DOWN);
  Island* west = _game->findAccessibleIsland(*_island, Direction::LEFT);

  if (north != nullptr) {
    result.push_back(createPair(north, _island, forbidden));
  }
  if (east != nullptr) {
    result.push_back(createPair(_island, east, forbidden));
  }
  if (south != nullptr) {
    result.push_back(createPair(_island, south, forbidden));
  }
  if (west != nullptr) {
    result.push_back(createPair(west, _island, forbidden));
  }
  return result;
}

// _____________________________________________________________________________

SmartConnector::SmartConnector(Game* game, Island* island,
  const std::unordered_multimap<Island*, Island*> &forbidden):
  _island(island), _game(game), _neighbours(createNeighbours(forbidden)) {}

// _____________________________________________________________________________

bool SmartConnector::connectSmart(int8_t conn,
  std::deque<std::pair<Bridge*, Bridge*>>* steps) const {
  int8_t sum = availableConnections();
  int8_t diff = sum - conn;
  if (diff < 0) {
    return false;
  }

  bool somethingChangedCase1 = false;
  switch (diff) {
    case 0:
      // There are exactly the amount of connections left, the Island needs
      // the connections are obvious
      for (const auto &neighbour : _neighbours) {
        if (neighbour.second != 0) {
          Bridge* oldBridge = nullptr;
          Bridge* newBridge = _game->connect(_island, neighbour.first,
            neighbour.second == 2, &oldBridge);
          steps->push_back({newBridge, oldBridge});
        }
      }
      return !_game->isPartOfDisjunctGroup(_island);
    case 1:
      // There is a slight ambiguity where the Islands need to connect
      // But we can at least create a single bridge, on the gaps where
      // a double bridge would be possible
      for (const auto &neighbour : _neighbours) {
        if (neighbour.second == 2) {
          steps->push_back({ _game->connect(_island, neighbour.first, false),
            nullptr });
          somethingChangedCase1 = true;
        }
      }
      // If no bridge has been added, we can safely fall-through and try it
      // the other way
      if (somethingChangedCase1) {
        break;
      }
    case 2:
      reversedFindConnection(conn, steps);
      break;
  }
  return true;
}

// _____________________________________________________________________________

void SmartConnector::reversedFindConnection(int8_t conn,
  std::deque<std::pair<Bridge*, Bridge*>>* steps) const {
  for (const auto &island : findReverseConnectIslands(conn)) {
    Bridge* oldBridge = nullptr;
    Bridge* newBridge = _game->connect(_island, island, false, &oldBridge);
    steps->push_back({ newBridge, oldBridge});
  }
}

// _____________________________________________________________________________

std::vector<Island*> SmartConnector::findReverseConnectIslands(int8_t conn)
  const {
  std::vector<Island*> result;
  for (const auto &neighbour : _neighbours) {
    if (neighbour.second == 0) {
      continue;
    }
    uint8_t sum = 0;
    std::deque<Island*> others;
    for (const auto &neighbourTwo : _neighbours) {
      if (neighbour.first == neighbourTwo.first || neighbourTwo.second == 0) {
        continue;
      }
      if (neighbourTwo.second != neighbourTwo.first->missingConnections()) {
        sum = 0;
        break;
      }
      others.push_back(neighbourTwo.first);
      sum += neighbourTwo.second;
    }
    if (sum == conn) {
      others.push_back(_island);
      if (_game->wouldCreateDisjunctGroup(&others)) {
        result.push_back(neighbour.first);
      }
    }
  }
  return result;
}

// _____________________________________________________________________________

int8_t SmartConnector::availableConnections() const {
  int8_t sum = 0;
  for (const auto &pair : _neighbours) {
    sum += pair.second;
  }
  return sum;
}
