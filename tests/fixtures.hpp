#pragma once

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace bogzilla::testing {

struct BoardFixture {
  std::vector<std::string> tiles;
  std::vector<std::string> answers;
};

// Reads tests/fixtures/wordshake_boards.txt: "tile,tile,...|word word ..." per line, # comments.
inline std::vector<BoardFixture> load_board_fixtures(const std::string& path) {
  std::ifstream in(path);
  if (!in) throw std::runtime_error("can't open " + path);

  std::vector<BoardFixture> fixtures;
  std::string line;
  while (std::getline(in, line)) {
    if (line.empty() || line[0] == '#') continue;
    const auto bar = line.find('|');
    if (bar == std::string::npos) throw std::runtime_error("bad fixture line: " + line);

    BoardFixture fixture;
    std::istringstream tiles(line.substr(0, bar));
    for (std::string tile; std::getline(tiles, tile, ',');) fixture.tiles.push_back(tile);
    std::istringstream answers(line.substr(bar + 1));
    for (std::string word; answers >> word;) fixture.answers.push_back(word);
    fixtures.push_back(std::move(fixture));
  }
  return fixtures;
}

}  // namespace bogzilla::testing
