#include "sprites.hpp"

// TODO: this should be read from a file instead of stored in the EXE
// TODO: consider using cogp47

// clang-format off
std::uint8_t const digit_sprites[10][DIGIT_HEIGHT][DIGIT_WIDTH] = {
  { 0, 255, 255, 255, 0,
    255, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    0, 255, 255, 255, 0 },

  { 0, 255, 255, 0, 0,
    0, 0, 255, 0, 0,
    0, 0, 255, 0, 0,
    0, 0, 255, 0, 0,
    0, 0, 255, 0, 0,
    0, 0, 255, 0, 0,
    0, 255, 255, 255, 0 },

  { 0, 255, 255, 255, 0,
    255, 0, 0, 0, 255,
    0, 0, 0, 0, 255,
    0, 0, 255, 255, 0,
    0, 255, 0, 0, 0,
    255, 0, 0, 0, 0,
    255, 255, 255, 255, 255 },

  { 0, 255, 255, 255, 0,
    255, 0, 0, 0, 255,
    0, 0, 0, 0, 255,
    0, 0, 255, 255, 0,
    0, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    0, 255, 255, 255, 0 },

  { 255, 0, 0, 255, 0,
    255, 0, 0, 255, 0,
    255, 0, 0, 255, 0,
    255, 0, 0, 255, 0,
    255, 255, 255, 255, 255,
    0, 0, 0, 255, 0,
    0, 0, 0, 255, 0 },

  { 255, 255, 255, 255, 255,
    255, 0, 0, 0, 0,
    255, 0, 0, 0, 0,
    255, 255, 255, 255, 0,
    0, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    0, 255, 255, 255, 0 },

  { 0, 255, 255, 255, 0,
    255, 0, 0, 0, 255,
    255, 0, 0, 0, 0,
    255, 255, 255, 255, 0,
    255, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    0, 255, 255, 255, 0 },

  { 255, 255, 255, 255, 255,
    0, 0, 0, 0, 255,
    0, 0, 0, 255, 0,
    0, 0, 255, 0, 0,
    0, 0, 255, 0, 0,
    0, 0, 255, 0, 0,
    0, 0, 255, 0, 0 },

  { 0, 255, 255, 255, 0,
    255, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    0, 255, 255, 255, 0,
    255, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    0, 255, 255, 255, 0 },

  { 0, 255, 255, 255, 0,
    255, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    0, 255, 255, 255, 255,
    0, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    0, 255, 255, 255, 0 }
};
// clang-format on