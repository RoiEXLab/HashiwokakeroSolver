#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <deque>
#include <algorithm>
#include <functional>
#include <cstdint>
#include "./Game.h"

// _____________________________________________________________________________

std::unordered_map<uint32_t, Island*> Game::islandVectorToMap(
  const std::vector<Island> &islands) const {
  std::unordered_map<uint32_t, Island*> map;
  for (const auto &entry : islands) {
    uint32_t key = entry._x + entry._y * _width;
    // Island deoesn't have a copy(-assignment) constructor
    // to make it explicit that that the Island will not
    // copy its bridges which would create undefined behaviour
    map[key] = new Island(entry._x, entry._y, entry._requiredBridges);
  }
  return map;
}

// _____________________________________________________________________________

// This is a little helper function that creates a 2 dimensional
// boolean array to be used by the Game class
bool** createBoolMatrix(uint32_t width, uint32_t height) {
  bool** matrix = new bool*[width];
  for (uint32_t x = 0; x < width; x++) {
    matrix[x] = new bool[height]();
  }
  return matrix;
}


// _____________________________________________________________________________

// Small helper function that gets the highest value (or 0 if no values) + 1
// based on the provided function which extracts a key from the given Island
// Used for const members by the Game class
uint32_t getSize(const std::vector<Island> &islands,
  std::function<uint32_t(Island)> func) {
  uint32_t max = 0;
  for (const auto &island : islands) {
    max = std::max(max, func(island));
  }
  return max + 1;
}


// _____________________________________________________________________________

Game::Game(const std::vector<Island> &islands) :
  _width(getSize(islands, [](const Island &island){ return island._x; })),
  _height(getSize(islands, [](const Island &island){ return island._y; })),
  _islands(islandVectorToMap(islands)),
  _bridgeOccupation(createBoolMatrix(_width, _height)) {}

// _____________________________________________________________________________

Island* Game::getIsland(uint32_t x, uint32_t y) const {
  if (x >= _width || y >= _height) {
    return nullptr;
  }
  // This ensures a unique key
  uint32_t key = x + y * _width;
  if (_islands.count(key)) {
    return _islands.at(key);
  }
  return nullptr;
}

// _____________________________________________________________________________

Island* const & IslandView::unordered_map_value_iterator::operator*() const {
  return std::unordered_map<uint32_t, Island*>::const_iterator::operator*()
    .second;
}

// _____________________________________________________________________________

IslandView::unordered_map_value_iterator IslandView::begin() const {
  return unordered_map_value_iterator(_islands.begin());
}

// _____________________________________________________________________________

IslandView::unordered_map_value_iterator IslandView::end() const {
  return unordered_map_value_iterator(_islands.end());
}

// _____________________________________________________________________________

size_t IslandView::size() const {
  return _islands.size();
}

// _____________________________________________________________________________

IslandView Game::getIslands() const {
  return IslandView(_islands);
}

// _____________________________________________________________________________


Island* Game::findAccessibleIsland(const Island &origin,
  const Direction &dir) const {
  // If we are already connected we can skip
  // the search process
  for (const auto &bridge : origin._bridges) {
    Island* other = bridge->_one == &origin ? bridge->_two : bridge->_one;
    if (&dir == origin.findDirection(other)) {
      return other;
    }
  }
  uint32_t x = origin._x + dir._xchange;
  uint32_t y = origin._y + dir._ychange;
  // Because we're underflowing, this is ok
  while (x < _width && y < _height) {
    if (_bridgeOccupation[x][y]) {
      return nullptr;
    }
    Island* island = getIsland(x, y);
    if (island != nullptr) {
      return island;
    }
    x += dir._xchange;
    y += dir._ychange;
  }
  return nullptr;
}

// _____________________________________________________________________________

std::vector<Island*> Game::findAccessibleIslands(
  const Island &origin) const {
  std::vector<Island*> result;

  Island* north = findAccessibleIsland(origin, Direction::UP);
  Island* east = findAccessibleIsland(origin, Direction::RIGHT);
  Island* south = findAccessibleIsland(origin, Direction::DOWN);
  Island* west = findAccessibleIsland(origin, Direction::LEFT);

  if (north != nullptr) { result.push_back(north); }
  if (east != nullptr) { result.push_back(east); }
  if (south != nullptr) { result.push_back(south); }
  if (west != nullptr) { result.push_back(west); }

  return result;
}

// _____________________________________________________________________________

bool Game::isSolved() const {
  // Depth first search to check if all Islands are connected to each other
  std::unordered_set<Island*> visited;
  std::deque<Island*> toVisit;
  toVisit.push_back(_islands.cbegin()->second);
  while (!toVisit.empty()) {
    Island* current = toVisit.back();
    toVisit.pop_back();
    visited.insert(current);
    uint8_t missingConnections = current->_requiredBridges;
    for (const auto &bridge : current->_bridges) {
      Island* other = bridge->_one == current ? bridge->_two : bridge->_one;
      if (!visited.count(other)) {
        toVisit.push_back(other);
      }
      missingConnections -= bridge->_doubleBridge ? 2 : 1;
    }
    if (missingConnections != 0) {
      // Some connection is still missing, no solution
      return false;
    }
  }
  // If we haven't visited all Islands, the graph is not complete
  return _islands.size() == visited.size();
}

// _____________________________________________________________________________

bool Game::wouldCreateDisjunctGroup(Island* island, Island* other) const {
  std::deque<Island*> toVisit;
  toVisit.push_back(island);
  toVisit.push_back(other);
  return wouldCreateDisjunctGroup(&toVisit);
}

// _____________________________________________________________________________

bool Game::wouldCreateDisjunctGroup(std::deque<Island*>* toVisit) const {
  std::unordered_set<Island*> visited;
  visited.insert(toVisit->begin(), toVisit->end());
  while (!toVisit->empty()) {
    Island* current = toVisit->front();
    toVisit->pop_front();

    // enqueue all unvisited, connected Islands
    for (const auto &bridge : current->_bridges) {
      Island* other = bridge->_one == current ? bridge->_two : bridge->_one;
      if (!visited.count(other)) {
        if (other->missingConnections() != 0) {
          // this function could be a little bit smarter,
          // but ain't nobody got time for that
          return false;
        }
        toVisit->push_back(other);
      }
    }
    visited.insert(current);
  }
  // We iterated through a closed group, if not all Islands
  // are in this group, some of them are disjunct
  return visited.size() != _islands.size();
}

// _____________________________________________________________________________

bool Game::isPartOfDisjunctGroup(Island* island) const {
  // Modified Depth-first search
  std::unordered_set<Island*> visited;
  std::deque<Island*> toVisit;
  toVisit.push_back(island);
  while (!toVisit.empty()) {
    Island* current = toVisit.front();
    toVisit.pop_front();
    if (current->missingConnections() != 0) {
      return false;
    }
    visited.insert(current);

    // enqueue all unvisited, connected Islands
    for (const auto &bridge : current->_bridges) {
      Island* other = bridge->_one == current ? bridge->_two : bridge->_one;
      if (!visited.count(other)) {
        toVisit.push_back(other);
      }
    }
  }
  // We iterated through a closed group, if not all Islands
  // are in this group, some of them are disjunct
  return visited.size() != _islands.size();
}

// _____________________________________________________________________________

void Game::registerBridge(Bridge* bridge, bool add) {
  bool horizontal = bridge->_one->_y == bridge->_two->_y;
  auto minmax = horizontal
    ? std::minmax(bridge->_one->_x, bridge->_two->_x)
    : std::minmax(bridge->_one->_y, bridge->_two->_y);
  for (auto i = minmax.first + 1; i < minmax.second; i++) {
    if (horizontal) {
      _bridgeOccupation[i][bridge->_one->_y] = add;
    } else {
      _bridgeOccupation[bridge->_one->_x][i] = add;
    }
  }
}

// _____________________________________________________________________________

int8_t Game::maxBandwidth(Island* current, Island* other) const {
  int8_t curMissConn = current->missingConnections();
  int8_t othMissConn = other->missingConnections();
  int8_t allowAddConn = 2 - current->isConnected(other);
  int8_t min = std::min({curMissConn, allowAddConn, othMissConn});
  if (curMissConn == othMissConn && curMissConn != 0
      && curMissConn <= allowAddConn) {
    if (wouldCreateDisjunctGroup(current, other)) {
      return min - 1;
    }
  }
  return min;
}

// _____________________________________________________________________________

bool Game::containsIsland(Island* island) {
  return getIsland(island->_x, island->_y) == island;
}

// _____________________________________________________________________________

Bridge* Game::connect(Island* origin, Island* other, bool doubleBridge,
  Bridge** oldBridge) {
  if (origin->isCorrectlyAlignedWith(other) && containsIsland(origin)
    && containsIsland(other)) {
    Bridge* existingBridge = nullptr;
    for (const auto &bridge : origin->_bridges) {
      // Does bridge connect origin and other?
      if (bridge->_one == other || bridge->_two == other) {
        if (!doubleBridge && !bridge->_doubleBridge) {
          // Requested bridge does not break the rules, proceed
          existingBridge = bridge;
          break;
        }
        // The requested bridge does brek the rules, abort
        return nullptr;
      }
    }
    if (existingBridge != nullptr) {
      disconnect(existingBridge);
      // Don't deallocate bridge, is we can pass the ownership to the caller
      if (oldBridge != nullptr) {
        *oldBridge = existingBridge;
      } else {
        delete existingBridge;
      }
    }
    bool bridgeStrength = existingBridge != nullptr || doubleBridge;
    auto bridge = new Bridge(origin, other, bridgeStrength);
    reconnect(bridge);
    return bridge;
  }
  return nullptr;
}

// _____________________________________________________________________________

void Game::disconnect(Bridge* bridge) {
  if (!containsIsland(bridge->_one) || !containsIsland(bridge->_two)) {
    return;
  }
  bridge->_one->_bridges.erase(std::remove(bridge->_one->_bridges.begin(),
    bridge->_one->_bridges.end(), bridge), bridge->_one->_bridges.end());
  bridge->_two->_bridges.erase(std::remove(bridge->_two->_bridges.begin(),
    bridge->_two->_bridges.end(), bridge), bridge->_two->_bridges.end());
  registerBridge(bridge, false);
}

// _____________________________________________________________________________

void Game::reconnect(Bridge* bridge) {
  if (!containsIsland(bridge->_one) || !containsIsland(bridge->_two)) {
    return;
  }
  bridge->_one->_bridges.push_back(bridge);
  bridge->_two->_bridges.push_back(bridge);
  registerBridge(bridge, true);
}

// _____________________________________________________________________________

Game::~Game() {
  for (const auto &island : getIslands()) {
    delete island;
  }
  for (size_t i = 0; i < _width; i++) {
    delete[] _bridgeOccupation[i];
  }
  delete[] _bridgeOccupation;
}

// _____________________________________________________________________________

bool Island::isCorrectlyAlignedWith(Island* other) const {
  return (_x == other->_x && _y != other->_y - 1
      && _y != other->_y && _y != other->_y + 1)
      || (_y == other->_y && _x != other->_x - 1
      && _x != other->_x && _x != other->_x + 1);
}

// _____________________________________________________________________________

int8_t Island::missingConnections() const {
  int8_t sum = _requiredBridges;
  for (const auto &bridge : _bridges) {
    sum -= bridge->_doubleBridge ? 2 : 1;
  }
  return sum;
}

// _____________________________________________________________________________

int8_t Island::isConnected(Island* other) const {
  for (const auto &bridge : _bridges) {
    if (bridge->_one == other || bridge->_two == other) {
      return bridge->_doubleBridge ? 2 : 1;
    }
  }
  return 0;
}


// _____________________________________________________________________________

const Direction* Island::findDirection(const Island* other) const {
  if (this == other) {
    return nullptr;
  }
  if (_x == other->_x) {
    if (_y > other->_y) {
      return &Direction::UP;
    } else if (_y < other->_y) {
      return &Direction::DOWN;
    }
  } else if (_y == other->_y) {
    if (_x > other->_x) {
      return &Direction::LEFT;
    } else if (_x < other->_x) {
      return &Direction::RIGHT;
    }
  }
  return nullptr;
}

// _____________________________________________________________________________

Island::~Island() {
  for (const auto &bridge : _bridges) {
    Island* other = bridge->_one == this ? bridge->_two : bridge->_one;
    other->_bridges.erase(std::remove(other->_bridges.begin(),
      other->_bridges.end(), bridge), other->_bridges.end());
    delete bridge;
  }
}

// _____________________________________________________________________________

// enum-like objects, the only different directions available
const Direction Direction::UP = Direction(0, -1);
const Direction Direction::RIGHT = Direction(1, 0);
const Direction Direction::DOWN = Direction(0, 1);
const Direction Direction::LEFT = Direction(-1, 0);
