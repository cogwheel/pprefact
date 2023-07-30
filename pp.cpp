//// TODO: fix indentation (tab stop is 3!!)

#include <cassert>
#include <cstdint>
#include <math.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>

#include <conio.h>
#include <dos.h>

using std::uint8_t;

//// TODO: Rearrange so we don't need prototypes?

// prototypes:
void init(void);
void blur(void);
struct PaletteDef;
void make_palette(PaletteDef const &pal_data);
inline void waves(void);
inline void dots(void);
inline void lines(void);

template <typename T> inline T clamp(T val, T min, T max) {
  return val < min ? min : val > max ? max : val;
}

#define NUM_EFFECTS 3

int curr_effect;

int score;

#define DIGIT_WIDTH 5
#define DIGIT_HEIGHT 7
#define DIGIT_SIZE (DIGIT_WIDTH * DIGIT_HEIGHT)
#define DIGIT_SPACING 7

// clang-format off
uint8_t digit_sprites[][DIGIT_HEIGHT][DIGIT_WIDTH] = {
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
#define NUM_DIGITS (sizeof(digit_sprites) / DIGIT_SIZE)

// 1021 is prime so it should be hard to stumble upon cycles
#define MAX_RAND_NUMS 1021

int rnd_tbl[MAX_RAND_NUMS];
int next_rnd_index;
float speed;
bool is_noisy;

#define NEBULA_PARTICLES 25
float neb_x[NEBULA_PARTICLES], neb_y[NEBULA_PARTICLES];
uint8_t neb_a[NEBULA_PARTICLES];

// TODO: these should be 256 not 255
double cosTable[255], sinTable[255];

inline int get_rnd(void) {
  if (++next_rnd_index >= MAX_RAND_NUMS) {
    next_rnd_index = 0;
  }
  return rnd_tbl[next_rnd_index];
}

void init_rnd(void) {
  next_rnd_index = 0;
  for (int i = 0; i < MAX_RAND_NUMS; i++) {
    rnd_tbl[i] = rand();
  }
}

#define VIDEO_INT 0x10          // the BIOS video interrupt.
#define WRITE_DOT 0x0C          // BIOS func to plot a pixel.
#define SET_MODE 0x00           // BIOS func to set the video mode.
#define GET_MODE 0x0F           // BIOS func to get the video mode.
#define VGA_256_COLOR_MODE 0x13 // use to set 256-color mode.
#define TEXT_MODE 0x03          // use to set 80x25 text mode.

#define SCREEN_WIDTH 320 // width in pixels of mode 0x13
#define MAX_X (SCREEN_WIDTH - 1)
#define MID_X (SCREEN_WIDTH >> 1)

#define SCREEN_HEIGHT 200 // height in pixels of mode 0x13
#define MAX_Y (SCREEN_HEIGHT - 1)
#define MID_Y (SCREEN_HEIGHT >> 1)

#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)

#define NUM_COLORS 256 // number of colors in mode 0x13
#define MAX_COLOR (NUM_COLORS - 1)
#define MAX_COLOR_COMPONENT 63 // RGB components in the palette

#define DISPLAY_ENABLE 0x01 // VGA input status bits
#define INPUT_STATUS 0x03da
#define VRETRACE 0x08

#define PALETTE_MASK 0x03c6
#define PALETTE_REGISTER_READ 0x03c7
#define PALETTE_REGISTER_WRITE 0x03c8
#define PALETTE_DATA 0x03c9

int mouse_x, mouse_y;
unsigned short target_x[SCREEN_WIDTH];
unsigned short target_y[SCREEN_HEIGHT];

// TODO: this should have a better name like `color_normalization`. It is a
// lookup table for taking the weighted sum around a target pixel
short colors[12 * MAX_COLOR];

float ball_x, ball_y, ball_x_delta, ball_y_delta, x_temp, y_temp;

void draw_score(uint8_t *buffer, int x, int y);

#define MAX_PALETTE_RANGES 8

struct PaletteColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

// TODO: Remove need to specify both start and end for each entry
struct PaletteRange {
  uint8_t first_index;
  uint8_t last_index;
  PaletteColor first_color;
  PaletteColor last_color;
};

// TODO: read these from a file
struct PaletteDef {
  // using the old static-sized approach since C++98 doesn't have initializer
  // lists
  int num_ranges;
  bool is_noisy;
  PaletteRange ranges[MAX_PALETTE_RANGES];
};

// clang-format off
PaletteDef pal_table[] = {
  { 5, true, {
    { 0, 31,
      {0,   0,   0},
      {0,   0,  63},
    },
    {32,  63,
      {0,   0,  63},
      {0,   0,   0},
    },
    {64,  95,
      {0,   0,   0},
      {63,   0,   0},
    },
    {96, 127,
      {63,   0,   0},
      {0,   0,   0},
    },
    {128, 255,
      {0,   0,   0},
      {63,  63,   0},
    },
  }},
  { 5, true, {
    {0,  31,
      {0,   0,   0},
      {21,   39,  23},
    },
    {32,  63,
      {21,   39,  23},
      {63,   19,   0},
    },
    {64,  95,
      {63,   19,   0},
      {32,   33,   27},
    },
    {96, 127,
      {32,   33,   27},
      {26,   5,   18},
    },
    {128, 255,
      {26,   5,   18},
      {63,  63,   0},
    },
  }},
  { 5, true, {
    {0,  31,
      {0,   0,   0},
      {21,   33,  40},
    },
    {32,  63,
      {21,   33,  40},
      {12,   12,   20},
    },
    {64,  110,
      {12,   12,   20},
      {43,   33,   38},
    },
    {111, 127,
      {43,   33,   38},
      {63,   17,   3},
    },
    {128, 255,
      {63,   17,   3},
      {54,  46,   30},
    },
  }},
  { 5, false, {
    {0,  31,
      {0,   0,   0},
      {57,   57,  63},
    },
    {32,  63,
      {57,   57,  63},
      {0,   0,   0},
    },
    {64,  110,
      {0,   0,   0},
      {63,   63,   63},
    },
    {111, 127,
      {63,   63,   63},
      {0,   0,   0},
    },
    {128, 255,
      {0,   0,   0},
      {63,  63,   63},
    },
  }}
};
// clang-format on

#define NUM_PALETTES (sizeof(pal_table) / sizeof(PaletteDef))

uint8_t *vga = (uint8_t *)0xA0000000L; // location of video memory
uint8_t *d_buffer, *x_buffer;

void set_pixel(uint8_t *buffer, int x, int y, uint8_t color);
void show_buffer(uint8_t *buffer);
void s_pal_entry(uint8_t index, uint8_t red, uint8_t green, uint8_t blue);
void line(uint8_t *buffer, int x1, int y1, int x2, int y2, uint8_t color);

uint8_t get_mode() {
  REGS regs;
  regs.h.ah = GET_MODE;
  int86(VIDEO_INT, &regs, &regs);
  return regs.h.al;
}

void set_mode(uint8_t mode) {
  union REGS regs;

  regs.h.ah = SET_MODE;
  regs.h.al = mode;
  int86(VIDEO_INT, &regs, &regs);
}

inline void set_pixel(uint8_t *buffer, int x, int y, uint8_t color) {
  buffer[y * SCREEN_WIDTH + x] = color;
}

inline void show_buffer(uint8_t *buffer) {
  while ((inp(INPUT_STATUS) & VRETRACE))
    ;
  while (!(inp(INPUT_STATUS) & VRETRACE))
    ;

  memcpy(x_buffer, buffer, SCREEN_SIZE);
  memcpy(vga, buffer, SCREEN_SIZE);
}

inline void s_pal_entry(uint8_t index, uint8_t red, uint8_t green,
                        uint8_t blue) {
  outp(PALETTE_MASK, 0xff);
  outp(PALETTE_REGISTER_WRITE, index); // tell it what index to use (0-255)
  outp(PALETTE_DATA, red);             // enter the red
  outp(PALETTE_DATA, green);           // green
  outp(PALETTE_DATA, blue);            // blue
}

void line(uint8_t *buffer, int x1, int y1, int x2, int y2, uint8_t color) {
  int dx, dy, xinc, yinc, two_dx, two_dy, x = x1, y = y1, i, error;

  if (x1 == x2 && y1 == y2) {
    set_pixel(buffer, x1, y1, color);
    return;
  }

  dx = (x2 - x1);
  if (dx < 0) {
    dx = -dx;
    xinc = -1;
  } else
    xinc = 1;

  dy = (y2 - y1);
  if (dy < 0) {
    dy = -dy;
    yinc = -1;
  } else
    yinc = 1;

  two_dx = dx + dx;
  two_dy = dy + dy;

  if (dx > dy) {
    error = 0;
    for (i = 0; i < dx; i++) {
      set_pixel(buffer, x, y, color);
      x += xinc;
      error += two_dy;
      if (error > dx) {
        error -= two_dx;
        y += yinc;
      }
    }
  } else {
    error = 0;
    for (i = 0; i < dy; i++) {
      set_pixel(buffer, x, y, color);
      y += yinc;
      error += two_dx;
      if (error > dy) {
        error -= two_dy;
        x += xinc;
      }
    }
  }
}

///--------------main stuff

// TODO: convert defines to consts?

#define COLLISION_THRESHOLD 15

// Visual
#define PADDLE_MARGIN 10
#define HALF_PADDLE 16

// Physical
#define PADDLE_MARGIN_HIT 13
#define HALF_PADDLE_HIT 18

#define SCORE_X 10
#define SCORE_Y 10

#define COUNTDOWN_X 150
#define COUNTDOWN_Y 91

#define START_SPEED 2.3

int main(void) {
  uint8_t old_mode = get_mode();

  init();

  if (get_mode() != VGA_256_COLOR_MODE) {
    fprintf(stderr, "Unable to set 320x200x256 color mode\n");
    exit(1);
  }

  union REGS r;
  r.x.bx = 0;
  while (r.x.bx != 3) {
    r.x.ax = 3;
    int86(0x33, &r, &r);

    // I *think* this magic math normalizes the mouse coordinates to the range
    // of the paddles
    mouse_x = r.x.cx * 0.420062695924 + 26;
    mouse_y = r.x.dx * 0.74 + 26;

    switch (curr_effect) {
    case 0:
      waves();
      break;
    case 1:
      dots();
      break;
    case 2:
      lines();
      break;
    }

    x_temp = ball_x;
    y_temp = ball_y;

    ball_x += ball_x_delta;
    ball_y += ball_y_delta;

    // TODO: De-nest these ifs (outer one seems redundant; might be
    // optimization)
    if (ball_x > (SCREEN_WIDTH - COLLISION_THRESHOLD) ||
        ball_x < COLLISION_THRESHOLD ||
        ball_y > (SCREEN_HEIGHT - COLLISION_THRESHOLD) ||
        ball_y < COLLISION_THRESHOLD) {
      if (ball_x >= SCREEN_WIDTH || ball_x < 0 || ball_y >= SCREEN_HEIGHT ||
          ball_y < 0) {
        ball_x = MID_X;
        ball_y = MID_Y;
        ball_x_delta = (get_rnd() % 2) ? START_SPEED : -START_SPEED;
        ball_y_delta = (get_rnd() % 2) ? START_SPEED : -START_SPEED;
        speed = START_SPEED;
        // TODO: Use a state machine or something so that input is still
        // processed while score is counting
        for (; score > 0; score--) {
          draw_score(x_buffer, COUNTDOWN_X, COUNTDOWN_Y);
          blur();
          show_buffer(d_buffer);
          draw_score(x_buffer, COUNTDOWN_X, COUNTDOWN_Y);
          blur();
          show_buffer(d_buffer);
        }
        init_rnd();
        make_palette(pal_table[get_rnd() % NUM_PALETTES]);
        curr_effect = get_rnd() % NUM_EFFECTS;
        score = 0;
      }
      if (ball_y > (mouse_y - HALF_PADDLE_HIT) &&
          ball_y < (mouse_y + HALF_PADDLE_HIT) && ball_x < PADDLE_MARGIN_HIT) {
        speed += .05;
        ball_x_delta = speed;
        ball_x = x_temp;
        ball_y_delta = (ball_y - mouse_y) / 4;
        make_palette(pal_table[get_rnd() % NUM_PALETTES]);
        curr_effect = get_rnd() % NUM_EFFECTS;
        score++;
      } else if (ball_y < (MAX_Y - (mouse_y - HALF_PADDLE_HIT)) &&
                 ball_y > (MAX_Y - (mouse_y + HALF_PADDLE_HIT)) &&
                 ball_x > (SCREEN_WIDTH - PADDLE_MARGIN_HIT)) {
        speed += .05;
        ball_x_delta = -1 * speed;
        ball_x = x_temp;
        ball_y_delta = (ball_y - (MAX_Y - mouse_y)) / 4;
        make_palette(pal_table[get_rnd() % NUM_PALETTES]);
        curr_effect = get_rnd() % NUM_EFFECTS;
        score++;
      } else if (ball_x < (MAX_X - (mouse_x - HALF_PADDLE_HIT)) &&
                 ball_x > (MAX_X - (mouse_x + HALF_PADDLE_HIT)) &&
                 ball_y < PADDLE_MARGIN_HIT) {
        speed += .05;
        ball_y_delta = speed;
        ball_y = y_temp;
        ball_x_delta = (ball_x - (MAX_X - mouse_x)) / 4;
        make_palette(pal_table[get_rnd() % NUM_PALETTES]);
        score++;
        curr_effect = get_rnd() % NUM_EFFECTS;
      } else if (ball_x > (mouse_x - HALF_PADDLE_HIT) &&
                 ball_x < (mouse_x + HALF_PADDLE_HIT) &&
                 ball_y >= (SCREEN_HEIGHT - PADDLE_MARGIN_HIT)) {
        speed += .05;
        ball_y_delta = -1 * speed;
        ball_y = y_temp;
        ball_x_delta = (ball_x - mouse_x) / 4;
        make_palette(pal_table[get_rnd() % NUM_PALETTES]);
        score++;
        curr_effect = get_rnd() % NUM_EFFECTS + 1;
      }
    }

    blur();

    draw_score(d_buffer, SCORE_X, SCORE_Y);

    // draw paddles

    // TOP
    line(d_buffer, MAX_X - (mouse_x - HALF_PADDLE), PADDLE_MARGIN,
         MAX_X - (mouse_x + HALF_PADDLE), PADDLE_MARGIN, MAX_COLOR);
    // BOTTOM
    line(d_buffer, mouse_x - HALF_PADDLE, SCREEN_HEIGHT - PADDLE_MARGIN,
         mouse_x + HALF_PADDLE, SCREEN_HEIGHT - PADDLE_MARGIN, MAX_COLOR);
    // LEFT
    line(d_buffer, PADDLE_MARGIN, mouse_y - HALF_PADDLE, PADDLE_MARGIN,
         mouse_y + HALF_PADDLE, MAX_COLOR);
    // RIGHT
    line(d_buffer, SCREEN_WIDTH - PADDLE_MARGIN,
         MAX_Y - (mouse_y - HALF_PADDLE), SCREEN_WIDTH - PADDLE_MARGIN,
         MAX_Y - (mouse_y + HALF_PADDLE), MAX_COLOR);

    // Draw "nucleus" (i think?)
    for (int i = 0; i < 5; i++) {
      line(d_buffer, (int)ball_x + get_rnd() % 6 - 3,
           (int)ball_y + get_rnd() % 6 - 3, (int)ball_x + get_rnd() % 6 - 3,
           (int)ball_y + get_rnd() % 6 - 3, 230);
    }

    float tempX;
    for (int i = 0; i < NEBULA_PARTICLES; i++) {

      tempX = neb_x[i];

      neb_x[i] = neb_x[i] * cosTable[neb_a[i]] - neb_y[i] * sinTable[neb_a[i]];
      neb_y[i] = neb_y[i] * cosTable[neb_a[i]] + tempX * sinTable[neb_a[i]];

      set_pixel(d_buffer, neb_x[i] + ball_x, neb_y[i] + ball_y, MAX_COLOR);
    }

    show_buffer(d_buffer);
  }

  set_mode(old_mode);

  return 0;
}

template <typename T> T clamp_color(T color) {
  return color < 0                     ? 0
         : color > MAX_COLOR_COMPONENT ? MAX_COLOR_COMPONENT
                                       : color;
}

uint8_t assert_color(uint8_t color) {
  assert(color <= MAX_COLOR_COMPONENT);
  return color;
}

void make_palette(PaletteDef const &pal_data) {
  uint8_t elem_start, elem_end, red_end, green_end, blue_end;

  // TODO: interpolate instead of integrate
  float red_inc, green_inc, blue_inc, difference, working_red, working_green,
      working_blue;

  for (int i = 0; i <= pal_data.num_ranges; ++i) {
    PaletteRange const &range = pal_data.ranges[i];
    elem_start = range.first_index;
    elem_end = range.last_index;
    difference = abs(elem_end - elem_start);

    working_red = assert_color(range.first_color.r);
    working_green = assert_color(range.first_color.g);
    working_blue = assert_color(range.first_color.b);
    red_end = assert_color(range.last_color.r);
    green_end = assert_color(range.last_color.g);
    blue_end = assert_color(range.last_color.b);

    red_inc = (float)(red_end - working_red) / (float)difference;
    green_inc = (float)(green_end - working_green) / (float)difference;
    blue_inc = (float)(blue_end - working_blue) / (float)difference;

    for (int j = elem_start; j <= elem_end; j++) {
      s_pal_entry(static_cast<uint8_t>(j), static_cast<uint8_t>(working_red),
                  static_cast<uint8_t>(working_green),
                  static_cast<uint8_t>(working_blue));

      working_red = clamp_color(working_red + red_inc);
      working_green = clamp_color(working_green + green_inc);
      working_blue = clamp_color(working_blue + blue_inc);
    }
  }

  is_noisy = pal_data.is_noisy;
}

void blur(void) {
  /*   int rand_x=0, rand_y=0;
  rand_x=rand()%4-2; rand_y=rand()%4-2; */
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {

      int new_color = 0;
      int index = target_y[y] + target_x[x];

      new_color += (x_buffer[index]) << 3;

      new_color += x_buffer[index + 1];
      new_color += x_buffer[index + SCREEN_WIDTH];

      new_color += x_buffer[index - 1];
      new_color += x_buffer[index - SCREEN_WIDTH];

      new_color = colors[new_color];
      if (is_noisy)
        new_color = clamp(new_color + get_rnd() % 2 - 1, 0, MAX_COLOR);
      set_pixel(d_buffer, x, y, static_cast<uint8_t>(new_color));
    }
  }
}

void init(void) {
  score = 0;
  srand(15);
  init_rnd();

  // allocate mem for the d_buffer
  if ((d_buffer = (uint8_t *)malloc(SCREEN_SIZE)) == NULL) {
    fprintf(stderr, "Not enough memory for double buffer.\n");
    exit(1);
  }

  if ((x_buffer = (uint8_t *)malloc(SCREEN_SIZE)) == NULL) {
    fprintf(stderr, "Not enough memory for double buffer.\n");
    exit(1);
  }

  memset(d_buffer, 0, SCREEN_SIZE);
  memset(x_buffer, 0, SCREEN_SIZE);

  set_mode(VGA_256_COLOR_MODE);
  make_palette(pal_table[get_rnd() % NUM_PALETTES]);
  curr_effect = get_rnd() % NUM_EFFECTS + 1;

  for (int i = 0; i < SCREEN_WIDTH; i++) {
    short target = ((i - MID_X) / 1.03) + MID_X;
    target_x[i] = clamp<short>(target, 0, SCREEN_WIDTH);
  }

  for (int i = 0; i < SCREEN_HEIGHT; i++) {
    short target = (((i - MID_Y) / 1.03) + MID_Y);
    target_y[i] = SCREEN_WIDTH * clamp<short>(target, 0, SCREEN_HEIGHT);
  }

  for (int i = 0; i < (12 * MAX_COLOR); i++) {
    colors[i] = i / 12.1;
  }

  ball_x = MID_X;
  ball_y = MID_Y;
  speed = START_SPEED;
  ball_x_delta = (get_rnd() % 2) ? START_SPEED : -START_SPEED;
  ball_y_delta = (get_rnd() % 2) ? START_SPEED : -START_SPEED;

  for (int i = 0; i < NEBULA_PARTICLES; i++) {
    neb_x[i] = get_rnd() % 3 - 5;
    neb_y[i] = get_rnd() % 3 - 5;
    neb_a[i] = static_cast<uint8_t>(get_rnd() % 30 - 15);
  }

  for (int i = 0; i != 255; i++) {
    cosTable[i] = cos(((2 * 3.1415) / 256) * i);
    sinTable[i] = sin(((2 * 3.1415) / 256) * i);
  }
  curr_effect = 0;
}

inline void waves(void) {
  int vertices[11];
  for (int i = 0; i <= 10; i++) {
    vertices[i] = get_rnd() % 60 + 60;
  }
  for (int i = 0; i < 10; i++) {
    line(x_buffer, i * 32, vertices[i], i * 32 + 32, vertices[i + 1], 128);
  }
  int drop_x = get_rnd() % MAX_X, drop_y = get_rnd() % 119;
  set_pixel(x_buffer, drop_x, drop_y, MAX_COLOR);
  set_pixel(x_buffer, drop_x + 1, drop_y, MAX_COLOR);
  set_pixel(x_buffer, drop_x - 1, drop_y, MAX_COLOR);
  set_pixel(x_buffer, drop_x, drop_y + 1, MAX_COLOR);
  set_pixel(x_buffer, drop_x, drop_y - 1, MAX_COLOR);
}

inline void draw_digit(uint8_t *buffer, int x, int y, int digit) {
  for (int y_loop = 0; y_loop < DIGIT_HEIGHT; y_loop++) {
    for (int x_loop = 0; x_loop < DIGIT_WIDTH; x_loop++) {
      set_pixel(buffer, x_loop + x, y_loop + y,
                digit_sprites[digit][y_loop][x_loop]);
    }
  }
}

void draw_score(uint8_t *buffer, int x, int y) {
  if (score == 0) {
    draw_digit(buffer, x, y, 0);
    return;
  }

  int divisor = 10000; // Max 16-bit power of 10
  while (divisor > score) {
    divisor /= 10;
  }
  int value = score;
  int offset = 0;
  do {
    int digit = value / divisor;
    draw_digit(buffer, x + offset, y, digit);
    offset += DIGIT_SPACING;
    value %= divisor;
    divisor /= 10;
  } while (divisor > 0);
}

inline void dots(void) {
  for (int i = 0; i < 8; i++) {
    int drop_x = get_rnd() % SCREEN_WIDTH, drop_y = get_rnd() % SCREEN_HEIGHT;
    set_pixel(x_buffer, drop_x, drop_y, MAX_COLOR);
    set_pixel(x_buffer, drop_x + 1, drop_y, MAX_COLOR);
    set_pixel(x_buffer, drop_x - 1, drop_y, MAX_COLOR);
    set_pixel(x_buffer, drop_x, drop_y + 1, MAX_COLOR);
    set_pixel(x_buffer, drop_x, drop_y - 1, MAX_COLOR);
  }
}

inline void lines(void) {

  line(x_buffer, get_rnd() % SCREEN_WIDTH, get_rnd() % SCREEN_HEIGHT,
       get_rnd() % SCREEN_WIDTH, get_rnd() % SCREEN_HEIGHT,
       static_cast<uint8_t>(get_rnd() % NUM_COLORS));
}
