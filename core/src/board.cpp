#include "bogzilla/board.hpp"

#include <cctype>
#include <utility>

namespace bogzilla {
namespace {

// Side length for a tile count, or 0 if it isn't a supported square.
int size_for(std::size_t tile_count) {
  for (int size = Board::kMinSize; size <= Board::kMaxSize; ++size) {
    if (tile_count == static_cast<std::size_t>(size * size)) return size;
  }
  return 0;
}

char lower(char c) {
  return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

bool is_letter(char c) { return c >= 'a' && c <= 'z'; }

}  // namespace

Board::Board(std::vector<std::string> tiles, int size) : size_(size), tiles_(std::move(tiles)) {
  for (int i = 0; i < tile_count(); ++i) {
    const auto self = to_index(i);
    for (int dr = -1; dr <= 1; ++dr) {
      for (int dc = -1; dc <= 1; ++dc) {
        if (dr == 0 && dc == 0) continue;
        const int r = row(i) + dr;
        const int c = col(i) + dc;
        if (r < 0 || r >= size_ || c < 0 || c >= size_) continue;
        neighbors_[self][neighbor_count_[self]++] = static_cast<std::uint8_t>(r * size_ + c);
      }
    }
  }
}

std::optional<Board> Board::from_tiles(std::span<const std::string> tiles) {
  const int size = size_for(tiles.size());
  if (size == 0) return std::nullopt;

  std::vector<std::string> normalized;
  normalized.reserve(tiles.size());
  for (const auto& tile : tiles) {
    if (tile.empty()) return std::nullopt;
    std::string letters;
    letters.reserve(tile.size());
    for (char c : tile) {
      const char l = lower(c);
      if (!is_letter(l)) return std::nullopt;
      letters.push_back(l);
    }
    normalized.push_back(std::move(letters));
  }
  return Board(std::move(normalized), size);
}

std::optional<Board> Board::parse(std::string_view text) {
  std::vector<std::string> tiles;
  for (std::size_t i = 0; i < text.size(); ++i) {
    const char c = lower(text[i]);
    if (std::isspace(static_cast<unsigned char>(c))) continue;
    if (!is_letter(c)) return std::nullopt;
    if (c == 'q') {
      if (i + 1 < text.size() && lower(text[i + 1]) == 'u') ++i;
      tiles.emplace_back("qu");
    } else {
      tiles.emplace_back(1, c);
    }
  }
  return from_tiles(tiles);
}

std::span<const std::uint8_t> Board::neighbors(int index) const noexcept {
  const auto i = to_index(index);
  return {neighbors_[i].data(), neighbor_count_[i]};
}

bool Board::adjacent(int a, int b) const noexcept {
  if (a == b) return false;
  const int dr = row(a) - row(b);
  const int dc = col(a) - col(b);
  return dr >= -1 && dr <= 1 && dc >= -1 && dc <= 1;
}

std::string Board::to_string() const {
  std::string out;
  for (int i = 0; i < tile_count(); ++i) {
    if (i > 0 && col(i) == 0) out.push_back(' ');
    out += tile(i);
  }
  return out;
}

}  // namespace bogzilla
