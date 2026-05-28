export const Config = {
  TITLE: "Hugo - Into the Multiverse",
  SCR_WIDTH: 640,
  SCR_HEIGHT: 480,

  GOD_MODE: false,

  EFFECT_DURATION_SPLAT: 2.5,
  EFFECT_DURATION_ORB: 0.5,
  EFFECT_DURATION_INVERT: 3,
  INSTRUCTIONS_TIMEOUT: 3,
  ARGENTINE_VERSION: true,

  // Key mappings: index = player index (0-3)
  // Using KeyboardEvent.key strings
  BTN_OFF_HOOK: ["F1", "F3", "F5", "F7"],
  BTN_HUNG_UP:  ["F2", "F4", "F6", "F8"],
  BTN_EXIT: "F12",

  BTN_1: ["1", "4", "7", "1"],          // KP1 also "1" — handled by numpad detection
  BTN_2: ["2", "5", "8", "2"],
  BTN_3: ["3", "6", "9", "3"],
  BTN_4: ["q", "r", "u", "4"],
  BTN_5: ["w", "t", "i", "5"],
  BTN_6: ["e", "y", "o", "6"],
  BTN_7: ["a", "f", "j", "7"],
  BTN_8: ["s", "g", "k", "8"],
  BTN_9: ["d", "h", "l", "9"],
  BTN_0: ["z", "x", "c", "0"],

  COUNTRIES: ["ar", "cl", "dn", "fr"] as const,

  GAMES: {
    Forest: { name: "Selva" },
  },

  // Mutable at runtime (joystick axis 1)
  FOREST_BG_SPEED_MULTIPLIER: 1.0,
  FOREST_GROUND_SPEED: 75,
  FOREST_MAX_TIME: 45,
};
