/**
 * @file uikit.c
 * @brief Implementation of terminal UI components.
 *
 * Contains the implementation of terminal-based selection menu rendering
 * and event handling using the termbox library.
 */

#include "presentation/uikit.h"
#include <float.h>
#include <stdbool.h>

/**
 * @brief Fuzzy search result with score
 *
 * Stores an item string pointer along with its fuzzy match score for sorting.
 * Used internally by the list filtering system to rank search results.
 *
 * @param item   Pointer to the original item string
 * @param score  Fuzzy match score (higher is better match)
 */
struct fuzzy_score {
  char *id;
  char *item;
  double score;
};

/**
 * @brief Vim-like motion handler.
 *
 * Processes terminal events for vim-style navigation including j/k for up/down,
 * g/G for top/bottom, Ctrl+U/Ctrl+D for page navigation, / for search mode,
 * i for insert mode, and q to quit. Respects the current vim mode (NORMAL,
 * INSERT, or SEARCH).
 *
 * @param item_count     Total number of items in the list.
 * @param current_index  Pointer to the current selection index (modified by navigation).
 * @param mode           Pointer to the current vim mode (may be changed by mode switches).
 * @param ev             Terminal event to process.
 *
 * @return 0   Navigation occurred successfully.
 * @return 1   Selection confirmed (Enter key).
 * @return 2   Mode changed to INSERT or SEARCH.
 * @return -1  User requested quit or error (null pointer).
 */
int vim_motion(const int item_count, int *current_index, enum vim_mode *mode,
               const struct tb_event ev) {
  if (current_index == NULL || mode == NULL)
    return -1;

  int term_height = tb_height();
  int visible_lines = term_height - 10; // Leave space for header/footer

  if (ev.type == TB_EVENT_KEY) {
    switch (ev.key) {
    case TB_KEY_ESC:
      if (*mode != NORMAL) {
        *mode = NORMAL;
      }
      break;
    case TB_KEY_ENTER:
      return 1;
      break;
    case TB_KEY_CTRL_U:
      if (*mode == NORMAL) {
        *current_index -= visible_lines;
        if (*current_index < 0)
          *current_index = 0;
      }
      break;
    case TB_KEY_CTRL_D:
      if (*mode == NORMAL) {
        *current_index += visible_lines;
        if (*current_index >= item_count) {
          *current_index = item_count - 1;
        }
      }
      break;
    default:
      switch (ev.ch) {
      case BINDING_UP:
        if (*current_index > 0 && *mode == NORMAL)
          *current_index -= 1;
        break;
      case BINDING_DOWN:
        if (*current_index < item_count - 1 && *mode == NORMAL)
          *current_index += 1;
        break;
      case BINDING_TOP:
        if (*mode == NORMAL) {
          *current_index = 0;
        }
        break;
      case BINDING_BOTTOM:
        if (*mode == NORMAL) {
          *current_index = item_count - 1;
        }
        break;
      case BINDING_SEARCH:
        if (*mode == NORMAL) {
          *mode = SEARCH;
          return 2;
        }
        break;
      case BINDING_INSERT:
        if (*mode == NORMAL) {
          *mode = INSERT;
          return 2;
        }
        break;
      case BINDING_QUIT:
        if (*mode == NORMAL) {
          return -1;
        }
        break;
      }
    }
  }

  return 0;
}

/**
 * @brief Default motion handler with arrow keys and standard navigation.
 *
 * Processes terminal events for standard navigation using arrow keys, PgUp/PgDn
 * for page navigation, Home/End for top/bottom, mouse wheel scrolling, and
 * Enter for selection. Supports both keyboard and mouse events.
 *
 * @param item_count     Total number of items in the list.
 * @param current_index  Pointer to the current selection index (modified by navigation).
 * @param ev             Terminal event to process.
 *
 * @return 0   Navigation occurred successfully.
 * @return 1   Selection confirmed (Enter or left mouse click).
 * @return -1  User requested quit (ESC) or error (null pointer).
 */
int default_motion(const int item_count, int *current_index, const struct tb_event ev) {
  if (current_index == NULL) {
    return -1;
  }

  int term_height = tb_height();
  int visible_lines = term_height - 10; // Leave space for header/footer

  if (ev.type == TB_EVENT_KEY || ev.type == TB_EVENT_MOUSE) {
    switch (ev.key) {
    case TB_KEY_MOUSE_WHEEL_UP:
    case TB_KEY_ARROW_UP:
      if (*current_index > 0)
        *current_index = *current_index - 1;
      break;

    case TB_KEY_MOUSE_WHEEL_DOWN:
    case TB_KEY_ARROW_DOWN:
      if (*current_index < item_count - 1)
        *current_index = *current_index + 1;
      break;

    case TB_KEY_PGUP:
      *current_index -= visible_lines;
      if (*current_index < 0)
        *current_index = 0;
      break;

    case TB_KEY_PGDN:
      *current_index += visible_lines;
      if (*current_index >= item_count) {
        *current_index = item_count - 1;
      }
      break;

    case TB_KEY_HOME:
      *current_index = 0;
      break;

    case TB_KEY_END:
      *current_index = item_count - 1;
      break;

    case TB_KEY_MOUSE_LEFT:
    case TB_KEY_ENTER:
      return 1;
    case TB_KEY_ESC:
      return -1;
    }
  }

  return 0;
}

/**
 * @brief Handle keyboard input for building text strings.
 *
 * Processes keyboard events to append characters, handle spaces, and process
 * backspace/delete operations. Only accepts printable ASCII characters (32-126).
 * Respects the maximum buffer size to prevent overflow.
 *
 * @param input       Pointer to the input buffer to modify.
 * @param input_size  Maximum size of the input buffer (including null terminator).
 * @param ev          Terminal event containing keyboard input.
 */
void inputtext_handler(char *input, const int input_size, const struct tb_event ev) {
  int input_len = strlen(input);

  if (ev.type == TB_EVENT_KEY) {
    switch (ev.key) {
    case TB_KEY_SPACE:
      if (input_len < input_size - 1) {
        input[input_len] = ' ';
        input[input_len + 1] = '\0';
      }
      break;
    case TB_KEY_DELETE:
    case TB_KEY_BACKSPACE2:
    case TB_KEY_BACKSPACE:
      if (input_len > 0) {
        input[input_len - 1] = '\0';
      }
      break;
    default:
      if (input_len < input_size - 1 && ev.ch >= 32 && ev.ch < 127) {
        input[input_len] = ev.ch;
        input[input_len + 1] = '\0';
      }
      break;
    }
  }
}

/**
 * @brief Fast alphanumeric character check (lowercase only).
 *
 * Checks if a character is a lowercase letter (a-z) or digit (0-9).
 * This is a performance-optimized inline function that avoids locale-dependent
 * isalnum() calls.
 *
 * @param c  Character to check.
 * @return   Non-zero if character is lowercase alphanumeric, 0 otherwise.
 */
static inline int is_alnum_fast(char c) {
  return ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'));
}

/**
 * @brief Calculate fuzzy match score between a string and pattern.
 *
 * Computes a score indicating how well a pattern matches a string using fuzzy
 * matching. The algorithm performs case-insensitive matching and awards points
 * for character matches, with bonuses for consecutive matches and matches at
 * word boundaries (end of string or before non-alphanumeric characters).
 *
 * Scoring breakdown:
 * - Base match: 10 points
 * - Consecutive matches: +5 points per consecutive character
 * - Word boundary matches: +15 points
 * - Non-matching characters: -1 point
 * - Pattern not fully matched: -DBL_MAX (essentially infinite penalty)
 *
 * @param str      The string to search within.
 * @param pattern  The pattern to match (query string).
 * @return         Score value (higher is better), or -DBL_MAX if pattern doesn't match.
 */
double fuzzy_score(const char *str, const char *pattern) {
  const int strl = strlen(str);
  const int patternl = strlen(pattern);

  if (patternl == 0)
    return 0.0;

  double score = 0.0;
  int si = 0, pi = 0, consecutive = 0;

  while (si < strl && pi < patternl) {
    unsigned char sc = str[si];
    unsigned char pc = pattern[pi];

    if (sc >= 'A' && sc <= 'z')
      sc += 32;
    if (pc >= 'A' && pc <= 'Z')
      pc += 32;

    if (sc == pc) {
      double s = 10.0 + consecutive * 5.0;

      if (si + 1 == strl || !is_alnum_fast(str[si + 1]))
        s += 15.0;

      score += s;
      consecutive++;
      pi++;
    } else {
      score -= 1.0;
      consecutive = 0;
    }

    si++;
    if (strl - si < patternl - pi)
      break;
  }

  if (pi < patternl)
    return -DBL_MAX;
  return score;
}

/**
 * @brief Comparison function for sorting fuzzy scores in descending order.
 *
 * Used by qsort() to order fuzzy_score structures so that higher scores appear
 * first. This allows best matches to be displayed at the top of the list.
 *
 * @param a  Pointer to first fuzzy_score structure.
 * @param b  Pointer to second fuzzy_score structure.
 * @return   Negative if b > a, positive if a > b, 0 if equal (descending order).
 */
int compare_score_desc(const void *a, const void *b) {
  double sa = ((struct fuzzy_score *)a)->score;
  double sb = ((struct fuzzy_score *)b)->score;
  return (sb > sa) - (sb < sa); // descending order
}

/**
 * @brief Filter and reorder items based on fuzzy search query.
 *
 * Implementation of the list filtering system. For each item, calculates a fuzzy
 * match score against the query, sorts items by score (best matches first), and
 * reorders the input array in-place to reflect the new ranking.
 *
 * The function allocates temporary memory to store items with their scores during
 * sorting, then copies the reordered pointers back to the original array.
 *
 * @param items  Array of string pointers to filter and sort (modified in-place).
 * @param count  Number of items in the array.
 * @param query  The search query string (empty string results in original order).
 */
void list_filter(struct listview_item items[], const int count, char *query) {
  if (!items || !query || count <= 0)
    return;

  struct fuzzy_score *arr = malloc(sizeof(struct fuzzy_score) * count);
  if (!arr)
    return;

  for (int i = 0; i < count; i++) {
    arr[i].id = items[i].id;
    arr[i].item = items[i].name;
    arr[i].score = fuzzy_score(items[i].name, query);
  }

  qsort(arr, count, sizeof(struct fuzzy_score), compare_score_desc);

  for (int i = 0; i < count; i++) {
    items[i].name = arr[i].item;
    items[i].id = arr[i].id;
  }

  free(arr);
}

/**
 * @brief Render a text input field with prompt and cursor.
 *
 * Displays the input prompt "> " followed by the current input text and an
 * underscore cursor indicator. If disabled, nothing is rendered. Uses PRIMARY_COLOR
 * for all text rendering.
 *
 * @param x        X coordinate for rendering the input field.
 * @param y        Y coordinate for rendering the input field.
 * @param input    The current input text to display.
 * @param enabled  Whether to render the input field (0 = hidden, non-zero = visible).
 */
void inputtext(const int x, const int y, const char *input, const int enabled) {

  if (!enabled) {
    return;
  }

  tb_print(x, y, PRIMARY_COLOR, TB_DEFAULT, "> ");
  tb_print(x + 2, y, PRIMARY_COLOR, TB_DEFAULT, input);
  tb_print(x + 2 + strlen(input), y, PRIMARY_COLOR, TB_DEFAULT, "_");
}

/**
 * @brief Manage viewport scrolling and render scrollbar.
 *
 * Automatically adjusts the viewport offset to ensure the currently selected item
 * is always visible. If the total item count exceeds the visible area, renders a
 * vertical scrollbar on the right edge with a position indicator showing the
 * current scroll position.
 *
 * @param current_index  The index of the currently selected item.
 * @param item_count     Total number of items in the list.
 * @param visible_lines  Number of lines visible in the viewport.
 * @param offset         Pointer to the viewport offset (modified to keep selection visible).
 */
void scrollbar(int current_index, int item_count, int visible_lines, int *offset) {
  // Auto-scroll: keep selected item visible
  if (current_index < *offset) {
    *offset = current_index;
  }

  if (current_index >= *offset + visible_lines) {
    *offset = current_index - visible_lines + 1;
  }

  if (item_count > visible_lines) {
    int scrollbar_x = tb_width() - 2;
    int scrollbar_height = visible_lines;
    int thumb_pos = (*offset * scrollbar_height) / item_count;

    for (int i = 0; i < scrollbar_height; i++) {
      if (i == thumb_pos) {
        tb_set_cell(scrollbar_x, 5 + i, SCROLL_BAR_INDICATOR, PRIMARY_COLOR, TB_DEFAULT); // █
      } else {
        tb_set_cell(scrollbar_x, 5 + i, BOX_LINE_VERTICAL, TB_WHITE, TB_DEFAULT); // │
      }
    }
  }
}

/**
 * @brief Display an interactive selection menu with vim and default motion modes.
 *
 * Initializes termbox and enters an event loop where users can navigate a list
 * using either default navigation (arrow keys, PgUp/PgDn, Home/End) or vim-style
 * navigation (j/k, g/G, Ctrl+U/Ctrl+D). Supports search and insert modes in vim
 * mode. Press Enter to confirm selection or ESC/q to quit.
 *
 * The function displays:
 * - A title at the top
 * - Item count information
 * - An optional input field (for search/insert modes)
 * - The visible portion of the item list with highlighting for the selected item
 * - A scrollbar when needed
 * - A footer with keybinding help text
 * - A mode indicator when in vim mode
 *
 * @param title     Title to display at the top of the menu.
 * @param items     Array of strings representing the selectable items.
 * @param count     Number of items in the array.
 * @param selected  Pointer to an integer where the selected index will be stored.
 *                  Will be set to -1 if the user quits without selecting.
 *
 * @note This function handles termbox initialization and cleanup internally.
 * @note The function blocks until the user makes a selection or quits.
 */
void listview(const char *title, struct listview_item items[], const int count, int *selected) {
  if (items == NULL || selected == NULL || count <= 0) {
    return;
  }

  char input[100] = "";
  char *vmode_names[3] = {" Normal ", " Insert ", " Search "};

  int current_index = *selected;
  int offset = 0;

  bool running = true;

  enum vim_mode vmode = NORMAL;
  enum motion_mode current_mode = DEFAULT;

  tb_init();

  while (running) {
    tb_clear();

    int term_height = tb_height();
    int term_width = tb_width();
    int visible_lines = term_height - 10;

    // Draw header
    tb_print(5, 2, TB_YELLOW | TB_BOLD, TB_DEFAULT, title);
    tb_printf(5, 3, TB_GREEN, TB_DEFAULT, "Showing %d-%d of %d", offset + 1,
              (offset + visible_lines < count) ? offset + visible_lines : count, count);

    bool inputtext_enabled = !(vmode == NORMAL && current_mode == VIM);
    inputtext(5, 4, input, inputtext_enabled);

    // Draw Footer
    if (current_mode == DEFAULT) {
      tb_print(5, term_height - 2, PRIMARY_COLOR, TB_DEFAULT,
               "↑/↓: Navigate "
               "| PgUp/pgDn: Fast scroll "
               "| HOME/END: Top/Bottom "
               "| Enter: Select "
               "| ESC: Quit "
               "| CTRL-/: Vim mode");
    } else {
      tb_print(5, term_height - 2, PRIMARY_COLOR, TB_DEFAULT,
               "k/j: Navigate "
               "| CTRL+U/CTRL+D: Fast scroll "
               "| g/G: Top/Bottom "
               "| Enter: Select "
               "| q: Quit "
               "| /: Search "
               "| ESC: Normal mode"
               "| CTRL-/: Default mode");

      tb_print(term_width - strlen(vmode_names[vmode]) - 5, term_height - 1, TB_BLACK,
               PRIMARY_COLOR, vmode_names[vmode]);
    }

    // Draw visible items
    list_filter(items, count, input);
    for (int i = 0; i < visible_lines && (offset + i) < count; i++) {
      int idx = offset + i;

      if (idx == current_index) {
        tb_print(5, 5 + i, TB_BLACK, PRIMARY_COLOR, "> ");
        tb_print(7, 5 + i, TB_BLACK, PRIMARY_COLOR, items[idx].name);
      } else {
        tb_print(5, 5 + i, PRIMARY_COLOR, TB_DEFAULT, "  ");
        tb_print(7, 5 + i, PRIMARY_COLOR, TB_DEFAULT, items[idx].name);
      }
    }

    scrollbar(current_index, count, visible_lines, &offset);

    // Show everything to the front
    tb_present();

    // handle event after ui show
    struct tb_event term_ev;
    tb_poll_event(&term_ev);

    if (current_mode == DEFAULT) {
      int motion = default_motion(count, &current_index, term_ev);
      if (motion < 0) {
        running = false;
      } else if (motion == 1) {
        *selected = current_index;
        running = false;
      }

      switch (term_ev.key) {
      case TB_KEY_CTRL_SLASH:
        current_mode = VIM;
        break;
      }
    } else {
      int motion = vim_motion(count, &current_index, &vmode, term_ev);

      if (motion < 0) {
        running = false;
      } else if (motion == 1) {
        *selected = current_index;
        running = false;
      } else if (motion == 2) {
        continue;
      }

      switch (term_ev.key) {
      case TB_KEY_CTRL_SLASH:
        current_mode = DEFAULT;
        break;
      }

      // Clear input when vim mode in normal
      if (vmode == NORMAL && current_mode == VIM) {
        input[0] = '\0';
      }
    }

    // input text after the motion
    inputtext_handler(input, sizeof(input), term_ev);
  }

  tb_shutdown();
}
