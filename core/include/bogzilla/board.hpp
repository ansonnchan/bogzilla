#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace bogzilla {

// Neighbours are worked out only once at construction, so the solver never recomputes them.
class Board {
public:
  static constexpr int kMinSize = 3;
  static constexpr int kMaxSize = 5; // i guess though, once we're sure 5x5 works, can increase more
  static constexpr int kMaxTiles = kMaxSize * kMaxSize;
  static constexpr int kMaxNeighbors = 8;

  // Builds a board from tiles as given, e.g. {"t", "s", "qu", ...}. Letters are
  // case-insensitive. 
  
  // Returns nullopt if the tile count isn't 9, 16 or 25, or a tile is
  // empty or contains anything other than a-z. 
  
  // of course, if we're being honest, this will never happen as what we're supporting is a browser game
  // which should always be a valid board. 
  static std::optional<Board> from_tiles(std::span<const std::string> tiles);


  static std::optional<Board> parse(std::string_view text);

  int size() const noexcept { 
    return size_; 
  }
  
  int tile_count() const noexcept { 
    return size_ * size_; 
  }
  const std::string& tile(int index) const { 
    return tiles_[to_index(index)]; 
  }

  int row(int index) const noexcept { 
    return index / size_; 
  }
  
  int col(int index) const noexcept { 
    return index % size_; 
  }

  // Tiles touching `index`, including diagonals. Corners have 3, edges 5, inner tiles 8.
  std::span<const std::uint8_t> neighbors(int index) const noexcept;
  bool adjacent(int a, int b) const noexcept;

  std::string to_string() const;

private:
  Board(std::vector<std::string> tiles, int size);
  static std::size_t to_index(int index) noexcept { return static_cast<std::size_t>(index); }

  int size_;
  std::vector<std::string> tiles_;
  std::array<std::array<std::uint8_t, kMaxNeighbors>, kMaxTiles> neighbors_{};
  std::array<std::uint8_t, kMaxTiles> neighbor_count_{};
};

}  // namespace bogzilla
