#include "core/Emoji.h"

#include <string.h>

#include "core/Display.h"

namespace {

// ---------------------------------------------------------------------------
// Dibujos. '#' = píxel encendido, '.' = apagado. 8 filas cada uno.
// Los seis primeros son los que más se usan en el calendario.
// ---------------------------------------------------------------------------

const Emoji::Art GIRL = {{
    "..####..",
    "########",
    "##....##",
    "#.#..#.#",
    "#......#",
    "#..##..#",
    ".#....#.",
    "..####..",
}};

const Emoji::Art BABY = {{
    "...#....",
    "..####..",
    ".#....#.",
    "#.#..#.#",
    "#......#",
    "#..##..#",
    ".#....#.",
    "..####..",
}};

const Emoji::Art WAVE = {{
    "........",
    "........",
    ".##..##.",
    "#..##..#",
    "........",
    ".##..##.",
    "#..##..#",
    "........",
}};

const Emoji::Art DANCER = {{
    "...##..#",
    "...##.#.",
    "..####..",
    ".#.##...",
    "...##...",
    "..####..",
    ".######.",
    "..#..#..",
}};

const Emoji::Art SCHOOL = {{
    "...##...",
    "..#..#..",
    ".#.##.#.",
    "########",
    "#......#",
    "#.#..#.#",
    "#..##..#",
    "########",
}};

const Emoji::Art FLAG_GB = {{
    "##...#...##",
    "..#..#..#..",
    "...#.#.#...",
    "###########",
    "###########",
    "...#.#.#...",
    "..#..#..#..",
    "##...#...##",
}};

const Emoji::Art FLAG_ES = {{
    "###########",
    "###########",
    "#.........#",
    "#.........#",
    "#.........#",
    "#.........#",
    "###########",
    "###########",
}};

// Personas y caras

const Emoji::Art BOY = {{
    ".#.#.#..",
    ".######.",
    ".#....#.",
    "#.#..#.#",
    "#......#",
    "#..##..#",
    ".#....#.",
    "..####..",
}};

const Emoji::Art SMILE = {{
    "..####..",
    ".#....#.",
    "#.#..#.#",
    "#......#",
    "#.#..#.#",
    "#..##..#",
    ".#....#.",
    "..####..",
}};

const Emoji::Art SAD = {{
    "..####..",
    ".#....#.",
    "#.#..#.#",
    "#......#",
    "#..##..#",
    "#.#..#.#",
    ".#....#.",
    "..####..",
}};

const Emoji::Art COOL = {{
    "..####..",
    ".#....#.",
    "########",
    "###..###",
    "#......#",
    "#.#..#.#",
    ".#.##.#.",
    "..####..",
}};

const Emoji::Art RUN = {{
    ".....##.",
    ".....##.",
    "..####..",
    ".#.##.#.",
    "...##...",
    "..#..#..",
    ".#....#.",
    "#.....#.",
}};

const Emoji::Art SWIM = {{
    ".....##.",
    "..##.##.",
    ".#..##..",
    "........",
    ".##..##.",
    "#..##..#",
    "........",
    "........",
}};

const Emoji::Art MUSCLE = {{
    ".##.....",
    "#.#.....",
    "#.#.##..",
    "#..#..#.",
    "#.....#.",
    ".#....#.",
    "..####..",
    "........",
}};

// Corazones y símbolos

const Emoji::Art HEART = {{
    ".##..##.",
    "########",
    "########",
    "########",
    ".######.",
    "..####..",
    "...##...",
    "........",
}};

const Emoji::Art STAR = {{
    "...##...",
    "...##...",
    "########",
    ".######.",
    "..####..",
    ".######.",
    ".##..##.",
    ".#....#.",
}};

const Emoji::Art CHECK = {{
    "........",
    ".......#",
    "......##",
    "#....##.",
    "##..##..",
    ".####...",
    "..##....",
    "........",
}};

const Emoji::Art CROSS = {{
    "##....##",
    ".##..##.",
    "..####..",
    "...##...",
    "..####..",
    ".##..##.",
    "##....##",
    "........",
}};

const Emoji::Art EXCLAMATION = {{
    "...##...",
    "...##...",
    "...##...",
    "...##...",
    "...##...",
    "........",
    "...##...",
    "........",
}};

const Emoji::Art QUESTION = {{
    "..####..",
    ".##..##.",
    ".....##.",
    "....##..",
    "...##...",
    "........",
    "...##...",
    "........",
}};

const Emoji::Art WARNING = {{
    "...##...",
    "..#..#..",
    ".#.##.#.",
    ".#.##.#.",
    "#......#",
    "#..##..#",
    "########",
    "........",
}};

const Emoji::Art FIRE = {{
    "...#....",
    "..##..#.",
    ".###.##.",
    ".#..###.",
    "#.....##",
    "#......#",
    ".#....#.",
    "..####..",
}};

const Emoji::Art MONEY = {{
    "........",
    "########",
    "########",
    "#......#",
    "#.##...#",
    "########",
    "........",
    "........",
}};

// Naturaleza y tiempo

const Emoji::Art SUN = {{
    "...#....",
    ".#...#..",
    "..###...",
    "#.###.#.",
    "..###...",
    ".#...#..",
    "...#....",
    "........",
}};

const Emoji::Art MOON = {{
    "...###..",
    ".###....",
    ".##.....",
    "##......",
    "##......",
    ".##.....",
    ".###....",
    "...###..",
}};

const Emoji::Art CLOUD = {{
    "........",
    "...##...",
    "..#..##.",
    ".#.....#",
    "#......#",
    "#......#",
    ".######.",
    "........",
}};

const Emoji::Art RAIN = {{
    "...##...",
    ".##..##.",
    "#......#",
    ".######.",
    "........",
    "#..#..#.",
    ".#..#..#",
    "........",
}};

const Emoji::Art SNOW = {{
    "...#....",
    ".#.#.#..",
    "..###...",
    "#######.",
    "..###...",
    ".#.#.#..",
    "...#....",
    "........",
}};

const Emoji::Art FLOWER = {{
    ".#.#.#..",
    ".#####..",
    ".#####..",
    "..###...",
    "...#....",
    ".#.#.#..",
    "..###...",
    "...#....",
}};

const Emoji::Art TREE = {{
    "...#....",
    "..###...",
    ".#####..",
    "..###...",
    ".#####..",
    "#######.",
    "...#....",
    "...#....",
}};

const Emoji::Art MOUNTAIN = {{
    "........",
    "...#....",
    "..#.#...",
    "..#.#.#.",
    ".#...#.#",
    ".#.....#",
    "#......#",
    "########",
}};

const Emoji::Art DOG = {{
    "##....##",
    "#.####.#",
    "#.#..#.#",
    "#......#",
    ".#.##.#.",
    ".#....#.",
    "..####..",
    "........",
}};

const Emoji::Art CAT = {{
    "#......#",
    "##....##",
    "#.#..#.#",
    "#......#",
    "#..##..#",
    ".#....#.",
    "..####..",
    "........",
}};

// Fiestas

const Emoji::Art CAKE = {{
    ".#.#.#..",
    ".#.#.#..",
    "#######.",
    "#.....#.",
    "#######.",
    "#.....#.",
    "#######.",
    "........",
}};

const Emoji::Art GIFT = {{
    ".##.##..",
    "#..#..#.",
    "#######.",
    "#..#..#.",
    "#..#..#.",
    "#..#..#.",
    "#######.",
    "........",
}};

const Emoji::Art PARTY = {{
    ".#...#..",
    "...#...#",
    "#.....#.",
    ".##..#..",
    ".###....",
    ".####.#.",
    "######..",
    "........",
}};

const Emoji::Art BALLOON = {{
    "..###...",
    ".#####..",
    ".#####..",
    "..###...",
    "...#....",
    "....#...",
    "...#....",
    "....#...",
}};

// Comida y bebida

const Emoji::Art COFFEE = {{
    "..#.#...",
    ".#.#....",
    "######..",
    "#....##.",
    "#....#.#",
    "#....##.",
    ".####...",
    "........",
}};

const Emoji::Art CUTLERY = {{
    "#.#..#..",
    "#.#..##.",
    "###..##.",
    ".#...##.",
    ".#...#..",
    ".#...#..",
    ".#...#..",
    "........",
}};

const Emoji::Art PIZZA = {{
    "########",
    ".#....#.",
    ".#.#..#.",
    "..#..#..",
    "..#..#..",
    "...##...",
    "...##...",
    "........",
}};

const Emoji::Art BURGER = {{
    "..####..",
    ".######.",
    "########",
    "........",
    "########",
    "........",
    ".######.",
    "........",
}};

const Emoji::Art WINE = {{
    "#.....#.",
    "#.....#.",
    "#######.",
    ".#####..",
    "...#....",
    "...#....",
    ".#####..",
    "........",
}};

const Emoji::Art BEER = {{
    ".####...",
    "######..",
    "#....###",
    "#....#.#",
    "#....#.#",
    "#....###",
    "######..",
    "........",
}};

// Transporte y lugares

const Emoji::Art PLANE = {{
    "...#....",
    "...##...",
    "#..##...",
    "########",
    "#..##...",
    "...##...",
    "...#....",
    "........",
}};

const Emoji::Art CAR = {{
    "........",
    "..####..",
    ".#.#..#.",
    "########",
    "########",
    ".##..##.",
    ".##..##.",
    "........",
}};

const Emoji::Art BUS = {{
    "########",
    "#..#...#",
    "#..#...#",
    "########",
    "########",
    "########",
    ".##..##.",
    "........",
}};

const Emoji::Art TRAIN = {{
    ".######.",
    "#......#",
    "#......#",
    "########",
    "#.####.#",
    "########",
    ".#....#.",
    "#......#",
}};

const Emoji::Art BIKE = {{
    "....##..",
    "..###...",
    "...#.#..",
    ".##..##.",
    "#..##..#",
    "#..#...#",
    ".##..##.",
    "........",
}};

const Emoji::Art HOUSE = {{
    "...##...",
    "..####..",
    ".######.",
    "########",
    ".#....#.",
    ".#.##.#.",
    ".#.##.#.",
    ".######.",
}};

const Emoji::Art BEACH = {{
    "..####..",
    ".######.",
    "########",
    "...#....",
    "...#....",
    "...#....",
    "...#....",
    "########",
}};

const Emoji::Art MEDICAL = {{
    "........",
    "...##...",
    "...##...",
    ".######.",
    ".######.",
    "...##...",
    "...##...",
    "........",
}};

// Trabajo, estudio y cosas

const Emoji::Art BRIEFCASE = {{
    "..####..",
    "..#..#..",
    "########",
    "#......#",
    "########",
    "#......#",
    "########",
    "........",
}};

const Emoji::Art LAPTOP = {{
    "........",
    ".######.",
    ".#....#.",
    ".#....#.",
    ".#....#.",
    ".######.",
    "########",
    "........",
}};

const Emoji::Art PHONE = {{
    ".######.",
    ".#....#.",
    ".#....#.",
    ".#....#.",
    ".#....#.",
    ".######.",
    ".##..##.",
    ".######.",
}};

const Emoji::Art ENVELOPE = {{
    "........",
    "########",
    "##....##",
    "#.#..#.#",
    "#..##..#",
    "#......#",
    "########",
    "........",
}};

const Emoji::Art CALENDAR = {{
    ".#....#.",
    "########",
    "########",
    "#......#",
    "#.#.#..#",
    "#......#",
    "#.#.#..#",
    "########",
}};

const Emoji::Art BOOK = {{
    "........",
    ".##..##.",
    "#..##..#",
    "#..##..#",
    "#..##..#",
    "#..##..#",
    ".##..##.",
    "...##...",
}};

const Emoji::Art PENCIL = {{
    "......##",
    ".....###",
    "....###.",
    "...###..",
    "..###...",
    ".###....",
    "##......",
    "#.......",
}};

const Emoji::Art GRADUATION = {{
    "...##...",
    ".######.",
    "########",
    ".######.",
    "..####.#",
    "..####.#",
    ".......#",
    "........",
}};

const Emoji::Art PIN = {{
    "..###...",
    ".#####..",
    ".#####..",
    "..###...",
    "...#....",
    "...#....",
    "...#....",
    "........",
}};

const Emoji::Art CLOCK = {{
    "..####..",
    ".#.#..#.",
    "#..#...#",
    "#..###.#",
    "#......#",
    "#......#",
    ".#....#.",
    "..####..",
}};

const Emoji::Art BELL = {{
    "...#....",
    "..###...",
    ".#####..",
    ".#####..",
    ".#####..",
    "#######.",
    "...#....",
    "........",
}};

const Emoji::Art PILL = {{
    "........",
    "........",
    ".######.",
    "#...####",
    "#...####",
    ".######.",
    "........",
    "........",
}};

const Emoji::Art TOOTH = {{
    ".##..##.",
    "#..##..#",
    "#......#",
    "#......#",
    ".#....#.",
    ".#.##.#.",
    ".#.##.#.",
    "..#..#..",
}};

const Emoji::Art CART = {{
    "#.......",
    ".#######",
    ".#.....#",
    "..#....#",
    "..######",
    "..#.....",
    "...#..#.",
    "........",
}};

// Ocio

const Emoji::Art SOCCER = {{
    "..####..",
    ".##..##.",
    "#..##..#",
    "#.####.#",
    "#.####.#",
    "#..##..#",
    ".##..##.",
    "..####..",
}};

const Emoji::Art BASKETBALL = {{
    "..####..",
    ".#.#..#.",
    "#..#...#",
    "########",
    "#...#..#",
    "#...#..#",
    ".#..#.#.",
    "..####..",
}};

const Emoji::Art MUSIC = {{
    "..######",
    "..#....#",
    "..#....#",
    "..#....#",
    "..#....#",
    "###..###",
    "###..###",
    "........",
}};

const Emoji::Art MOVIE = {{
    ".#.#.#.#",
    "########",
    "........",
    "########",
    "#......#",
    "#......#",
    "#......#",
    "########",
}};

const Emoji::Art GAME = {{
    "........",
    ".######.",
    "#.#...##",
    "###..#.#",
    "#.#...##",
    ".######.",
    "........",
    "........",
}};

// ---------------------------------------------------------------------------
// Tabla de puntos de código. Varios emojis parecidos comparten dibujo
// (por ejemplo, todos los corazones de colores usan HEART).
// ---------------------------------------------------------------------------

struct Entry {
  uint32_t codepoint;
  const Emoji::Art* art;
};

const Entry TABLE[] = {
    // Los más usados
    {0x1F467, &GIRL},
    {0x1F476, &BABY},
    {0x1F30A, &WAVE},
    {0x1F483, &DANCER},
    {0x1F57A, &DANCER},
    {0x1F3EB, &SCHOOL},

    // Personas y caras
    {0x1F466, &BOY},
    {0x1F468, &BOY},
    {0x1F469, &GIRL},
    {0x1F600, &SMILE},
    {0x1F601, &SMILE},
    {0x1F603, &SMILE},
    {0x1F604, &SMILE},
    {0x1F606, &SMILE},
    {0x1F60A, &SMILE},
    {0x1F642, &SMILE},
    {0x263A, &SMILE},
    {0x1F973, &SMILE},
    {0x1F622, &SAD},
    {0x1F61E, &SAD},
    {0x1F614, &SAD},
    {0x1F62D, &SAD},
    {0x2639, &SAD},
    {0x1F60E, &COOL},
    {0x1F3C3, &RUN},
    {0x1F6B6, &RUN},
    {0x1F3CA, &SWIM},
    {0x1F4AA, &MUSCLE},
    {0x1F3CB, &MUSCLE},

    // Corazones y símbolos
    {0x2764, &HEART},
    {0x1F493, &HEART},
    {0x1F495, &HEART},
    {0x1F496, &HEART},
    {0x1F497, &HEART},
    {0x1F498, &HEART},
    {0x1F499, &HEART},
    {0x1F49A, &HEART},
    {0x1F49B, &HEART},
    {0x1F49C, &HEART},
    {0x1F49D, &HEART},
    {0x1F49E, &HEART},
    {0x1F5A4, &HEART},
    {0x1F90D, &HEART},
    {0x1F90E, &HEART},
    {0x1F9E1, &HEART},
    {0x2B50, &STAR},
    {0x1F31F, &STAR},
    {0x2728, &STAR},
    {0x2705, &CHECK},
    {0x2714, &CHECK},
    {0x2611, &CHECK},
    {0x274C, &CROSS},
    {0x274E, &CROSS},
    {0x2716, &CROSS},
    {0x2757, &EXCLAMATION},
    {0x2755, &EXCLAMATION},
    {0x203C, &EXCLAMATION},
    {0x2753, &QUESTION},
    {0x2754, &QUESTION},
    {0x26A0, &WARNING},
    {0x1F525, &FIRE},
    {0x1F4B0, &MONEY},
    {0x1F4B3, &MONEY},
    {0x1F4B5, &MONEY},
    {0x1F4B6, &MONEY},
    {0x1F4B8, &MONEY},

    // Naturaleza y tiempo
    {0x2600, &SUN},
    {0x1F31E, &SUN},
    {0x1F319, &MOON},
    {0x2601, &CLOUD},
    {0x26C5, &CLOUD},
    {0x1F327, &RAIN},
    {0x2614, &RAIN},
    {0x2602, &RAIN},
    {0x2744, &SNOW},
    {0x26C4, &SNOW},
    {0x1F328, &SNOW},
    {0x1F338, &FLOWER},
    {0x1F337, &FLOWER},
    {0x1F339, &FLOWER},
    {0x1F33A, &FLOWER},
    {0x1F33B, &FLOWER},
    {0x1F33C, &FLOWER},
    {0x1F490, &FLOWER},
    {0x1F332, &TREE},
    {0x1F333, &TREE},
    {0x1F384, &TREE},
    {0x26F0, &MOUNTAIN},
    {0x1F3D4, &MOUNTAIN},
    {0x1F3D5, &MOUNTAIN},
    {0x1F436, &DOG},
    {0x1F415, &DOG},
    {0x1F431, &CAT},
    {0x1F408, &CAT},

    // Fiestas
    {0x1F382, &CAKE},
    {0x1F370, &CAKE},
    {0x1F9C1, &CAKE},
    {0x1F381, &GIFT},
    {0x1F389, &PARTY},
    {0x1F38A, &PARTY},
    {0x1F388, &BALLOON},

    // Comida y bebida
    {0x2615, &COFFEE},
    {0x1F375, &COFFEE},
    {0x1F374, &CUTLERY},
    {0x1F37D, &CUTLERY},
    {0x1F355, &PIZZA},
    {0x1F354, &BURGER},
    {0x1F377, &WINE},
    {0x1F378, &WINE},
    {0x1F379, &WINE},
    {0x1F942, &WINE},
    {0x1F37A, &BEER},
    {0x1F37B, &BEER},

    // Transporte y lugares
    {0x2708, &PLANE},
    {0x1F6E9, &PLANE},
    {0x1F6EB, &PLANE},
    {0x1F6EC, &PLANE},
    {0x1F695, &CAR},
    {0x1F697, &CAR},
    {0x1F698, &CAR},
    {0x1F699, &CAR},
    {0x1F68C, &BUS},
    {0x1F68D, &BUS},
    {0x1F690, &BUS},
    {0x1F682, &TRAIN},
    {0x1F683, &TRAIN},
    {0x1F684, &TRAIN},
    {0x1F685, &TRAIN},
    {0x1F686, &TRAIN},
    {0x1F687, &TRAIN},
    {0x1F688, &TRAIN},
    {0x1F689, &TRAIN},
    {0x1F68A, &TRAIN},
    {0x1F6B2, &BIKE},
    {0x1F6B4, &BIKE},
    {0x1F6B5, &BIKE},
    {0x1F3E0, &HOUSE},
    {0x1F3E1, &HOUSE},
    {0x1F3D6, &BEACH},
    {0x26F1, &BEACH},
    {0x1F3E5, &MEDICAL},
    {0x1F691, &MEDICAL},
    {0x1FA7A, &MEDICAL},
    {0x2695, &MEDICAL},

    // Trabajo, estudio y cosas
    {0x1F4BC, &BRIEFCASE},
    {0x1F4BB, &LAPTOP},
    {0x1F5A5, &LAPTOP},
    {0x1F4F1, &PHONE},
    {0x1F4F2, &PHONE},
    {0x1F4DE, &PHONE},
    {0x260E, &PHONE},
    {0x2709, &ENVELOPE},
    {0x1F4E7, &ENVELOPE},
    {0x1F4E8, &ENVELOPE},
    {0x1F4E9, &ENVELOPE},
    {0x1F48C, &ENVELOPE},
    {0x1F4C5, &CALENDAR},
    {0x1F4C6, &CALENDAR},
    {0x1F5D3, &CALENDAR},
    {0x1F4DA, &BOOK},
    {0x1F4D2, &BOOK},
    {0x1F4D3, &BOOK},
    {0x1F4D4, &BOOK},
    {0x1F4D5, &BOOK},
    {0x1F4D6, &BOOK},
    {0x1F4D7, &BOOK},
    {0x1F4D8, &BOOK},
    {0x1F4D9, &BOOK},
    {0x270F, &PENCIL},
    {0x2712, &PENCIL},
    {0x1F4DD, &PENCIL},
    {0x1F58A, &PENCIL},
    {0x1F58B, &PENCIL},
    {0x1F393, &GRADUATION},
    {0x1F4CC, &PIN},
    {0x1F4CD, &PIN},
    {0x23F0, &CLOCK},
    {0x23F1, &CLOCK},
    {0x23F2, &CLOCK},
    {0x231A, &CLOCK},
    {0x1F514, &BELL},
    {0x1F48A, &PILL},
    {0x1F9B7, &TOOTH},
    {0x1F6D2, &CART},
    {0x1F6CD, &CART},

    // Ocio
    {0x26BD, &SOCCER},
    {0x1F3C0, &BASKETBALL},
    {0x1F3B5, &MUSIC},
    {0x1F3B6, &MUSIC},
    {0x1F3BC, &MUSIC},
    {0x1F3AC, &MOVIE},
    {0x1F3A5, &MOVIE},
    {0x1F37F, &MOVIE},
    {0x1F4FA, &MOVIE},
    {0x1F3AE, &GAME},
};

// Banderas: dos letras regionales (A = U+1F1E6, B = U+1F1E7...).
struct FlagEntry {
  char a;
  char b;
  const Emoji::Art* art;
};

const FlagEntry FLAGS[] = {
    {'G', 'B', &FLAG_GB},
    {'E', 'S', &FLAG_ES},
};

const uint32_t REGIONAL_A = 0x1F1E6;

}  // namespace

namespace Emoji {

const Art* find(uint32_t codepoint) {
  // Las esferas de reloj (de la una a las doce y media) usan todas el mismo dibujo.
  if (codepoint >= 0x1F550 && codepoint <= 0x1F567) {
    return &CLOCK;
  }
  for (size_t i = 0; i < sizeof(TABLE) / sizeof(TABLE[0]); i++) {
    if (TABLE[i].codepoint == codepoint) {
      return TABLE[i].art;
    }
  }
  return nullptr;
}

const Art* findFlag(uint32_t first, uint32_t second) {
  const char a = 'A' + (char)(first - REGIONAL_A);
  const char b = 'A' + (char)(second - REGIONAL_A);
  for (size_t i = 0; i < sizeof(FLAGS) / sizeof(FLAGS[0]); i++) {
    if (FLAGS[i].a == a && FLAGS[i].b == b) {
      return FLAGS[i].art;
    }
  }
  return nullptr;
}

int width(const Art* art) {
  return (int)strlen(art->rows[0]);
}

void draw(int x, int topY, const Art* art) {
  U8G2& g = Display::gfx();
  for (int row = 0; row < HEIGHT; row++) {
    const char* line = art->rows[row];
    for (int col = 0; line[col] != '\0'; col++) {
      if (line[col] == '#') {
        g.drawPixel(x + col, topY + row);
      }
    }
  }
}

}  // namespace Emoji
