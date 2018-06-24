// Font generated from https://evanw.github.io/font-texture-generator/
// Works in conjunction with associated helvetica-32.png file.

typedef struct Character {
  int codePoint, x, y, width, height, originX, originY;
} Character;

typedef struct Font {
  const char *name;
  int size, bold, italic, width, height, characterCount;
  Character *characters;
} Font;

static Character characters_Helvetica[] = {
  {' ', 465, 96, 3, 3, 1, 1},
  {'!', 338, 67, 10, 29, 0, 26},
  {'"', 343, 96, 15, 16, 2, 26},
  {'#', 50, 67, 24, 29, 3, 26},
  {'$', 100, 0, 22, 35, 2, 28},
  {'%', 314, 0, 33, 30, 2, 26},
  {'&', 347, 0, 26, 30, 2, 26},
  {'\'', 358, 96, 10, 16, 1, 26},
  {'(', 35, 0, 14, 37, 1, 27},
  {')', 49, 0, 14, 37, 2, 27},
  {'*', 327, 96, 16, 16, 2, 26},
  {'+', 150, 96, 23, 23, 2, 20},
  {',', 368, 96, 10, 15, 1, 7},
  {'-', 416, 96, 15, 10, 2, 14},
  {'.', 431, 96, 10, 10, 1, 7},
  {'/', 292, 67, 16, 29, 3, 26},
  {'0', 443, 0, 22, 30, 2, 26},
  {'1', 308, 67, 15, 29, 0, 26},
  {'2', 212, 67, 22, 29, 2, 26},
  {'3', 398, 0, 23, 30, 3, 26},
  {'4', 122, 67, 23, 29, 3, 26},
  {'5', 168, 67, 22, 29, 2, 25},
  {'6', 465, 0, 22, 30, 2, 26},
  {'7', 376, 67, 22, 28, 2, 25},
  {'8', 0, 37, 22, 30, 2, 26},
  {'9', 22, 37, 22, 30, 2, 26},
  {':', 273, 96, 10, 23, 0, 20},
  {';', 398, 67, 10, 28, 0, 20},
  {'<', 0, 96, 25, 24, 3, 20},
  {'=', 304, 96, 23, 16, 2, 16},
  {'>', 25, 96, 25, 24, 3, 20},
  {'?', 88, 37, 21, 30, 1, 27},
  {'@', 151, 0, 32, 31, 0, 27},
  {'A', 267, 37, 27, 29, 3, 26},
  {'B', 452, 37, 25, 29, 1, 26},
  {'C', 240, 0, 27, 31, 2, 27},
  {'D', 348, 37, 26, 29, 1, 26},
  {'E', 74, 67, 24, 29, 1, 26},
  {'F', 145, 67, 23, 29, 1, 26},
  {'G', 212, 0, 28, 31, 2, 27},
  {'H', 25, 67, 25, 29, 1, 26},
  {'I', 348, 67, 10, 29, 0, 26},
  {'J', 130, 37, 20, 30, 3, 26},
  {'K', 374, 37, 26, 29, 1, 26},
  {'L', 190, 67, 22, 29, 1, 26},
  {'M', 210, 37, 29, 29, 1, 26},
  {'N', 477, 37, 25, 29, 1, 26},
  {'O', 183, 0, 29, 31, 2, 27},
  {'P', 98, 67, 24, 29, 1, 26},
  {'Q', 122, 0, 29, 32, 2, 27},
  {'R', 400, 37, 26, 29, 1, 26},
  {'S', 267, 0, 25, 31, 2, 27},
  {'T', 426, 37, 26, 29, 3, 26},
  {'U', 373, 0, 25, 30, 1, 26},
  {'V', 294, 37, 27, 29, 3, 26},
  {'W', 174, 37, 36, 29, 3, 26},
  {'X', 321, 37, 27, 29, 3, 26},
  {'Y', 239, 37, 28, 29, 3, 26},
  {'Z', 0, 67, 25, 29, 3, 26},
  {'[', 76, 0, 12, 36, 1, 26},
  {'\\', 274, 67, 18, 29, 4, 26},
  {']', 63, 0, 13, 36, 3, 26},
  {'^', 283, 96, 21, 20, 2, 26},
  {'_', 441, 96, 24, 8, 3, 1},
  {'`', 403, 96, 13, 11, 3, 27},
  {'a', 408, 67, 23, 25, 2, 21},
  {'b', 487, 0, 22, 30, 2, 26},
  {'c', 454, 67, 22, 25, 3, 21},
  {'d', 421, 0, 22, 30, 3, 26},
  {'e', 50, 96, 22, 24, 2, 20},
  {'f', 150, 37, 15, 30, 3, 27},
  {'g', 109, 37, 21, 30, 2, 20},
  {'h', 254, 67, 20, 29, 1, 26},
  {'i', 358, 67, 9, 29, 1, 26},
  {'j', 88, 0, 12, 36, 4, 26},
  {'k', 234, 67, 20, 29, 1, 26},
  {'l', 367, 67, 9, 29, 1, 26},
  {'m', 121, 96, 29, 23, 1, 20},
  {'n', 238, 96, 20, 23, 1, 20},
  {'o', 431, 67, 23, 25, 3, 21},
  {'p', 292, 0, 22, 31, 2, 21},
  {'q', 44, 37, 22, 30, 3, 20},
  {'r', 258, 96, 15, 23, 1, 20},
  {'s', 476, 67, 20, 25, 2, 21},
  {'t', 323, 67, 15, 29, 3, 25},
  {'u', 72, 96, 20, 24, 1, 20},
  {'v', 195, 96, 22, 23, 3, 20},
  {'w', 92, 96, 29, 23, 3, 20},
  {'x', 173, 96, 22, 23, 3, 20},
  {'y', 66, 37, 22, 30, 3, 20},
  {'z', 217, 96, 21, 23, 3, 20},
  {'{', 18, 0, 17, 37, 4, 27},
  {'|', 165, 37, 9, 30, 1, 27},
  {'}', 0, 0, 18, 37, 3, 27},
  {'~', 378, 96, 25, 13, 3, 15},
};

static Font font_Helvetica = {"Helvetica", 32, 0, 0, 512, 128, 95, characters_Helvetica};