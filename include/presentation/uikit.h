/**
 * @file uikit.h
 * @brief Terminal UI components for interactive selection interface.
 *
 * Provides functions to draw terminal-based selection menus using termbox,
 * with rounded box borders, keyboard navigation, and visual selection feedback.
 */

#ifndef UIKIT_H
#define UIKIT_H

#include "../lib/termbox.h"

/* UI Box Drawing Characters (Unicode) */
#define BOX_ROUND_TOP_LEFT     0x256D /**< Rounded top-left corner: ╭ */
#define BOX_ROUND_TOP_RIGHT    0x256E /**< Rounded top-right corner: ╮ */
#define BOX_ROUND_BOTTOM_LEFT  0x2570 /**< Rounded bottom-left corner: ╰ */
#define BOX_ROUND_BOTTOM_RIGHT 0x256F /**< Rounded bottom-right corner: ╯ */
#define BOX_LINE_VERTICAL      0x2502 /**< Vertical line: │ */
#define BOX_LINE_HORIZONTAL    0x2500 /**< Horizontal line: ─ */
#define SCROLL_BAR_INDICATOR   0x2588 /**< Scroll bar indicator █*/

/* Terminal Layout Constants */
#define MARGIN_Y      1.0 /**< Vertical margin multiplier (1.0 = full height) */
#define MARGIN_X      1.0 /**< Horizontal margin multiplier (1.0 = full width) */
#define PADDING_START 3   /**< Left padding for list items */

/* VIM Keyboard Bindings */
#define BINDING_QUIT   'q' /**< Key to quit the selection interface */
#define BINDING_UP     'k' /**< Key to move selection up (vim-style) */
#define BINDING_DOWN   'j' /**< Key to move selection down (vim-style) */
#define BINDING_TOP    'g' /**< key to go to the top */
#define BINDING_BOTTOM 'G' /**< key to go to down end */
#define BINDING_INSERT 'i' /**< key to go to down end */
#define BINDING_SEARCH '/' /**< key to activate insert mode */

/* Color Scheme */
#define PRIMARY_COLOR    0x0007 /**< Primary color (Cyan) */
#define BACKGROUND_COLOR 0x0000 /**< Background color (Black) */

/**
 * @brief Vim Motion event
 *
 * Event for cursor movement and action
 * for vim motion
 *
 * @enum NORMAL     Normal Mode
 * @enum INSERT     Insert Mode
 * @enum SEARCH     Search Mode
 */
enum vim_mode {
  NORMAL,
  INSERT,
  SEARCH,
};

/**
 * @brief Motion mode
 *
 * This enum used for state of changing motion between normal mode and vim mode
 *
 * @enum NORMAL     Common motion
 * @enum VIM        Vim motion
 */
enum motion_mode { DEFAULT, VIM };

struct listview_item {
  char *id;
  char *name;
};

/**
 * @brief Vim-like motion
 *
 * Interactive cursor and event with vim-like motion.
 * this motion contain vertical movement with j/k,
 * page up/down using CTRL+U/D, Go to the top/bottom using g/G
 * and do Search using Slash (/). Insert mode, Search mode and normal mode is provided
 *
 * @param item_count        Total item to interact with motion
 * @param current_index     Current cursor in index position
 * @param mode              Current vim motion mode normal/insert/search
 * @param ev                Observe terminal event
 *
 * @return 0    Success
 * @return 1    selection event
 * @return 2    change mode insert/search
 * @return -1   Exit/Quit/Error
 *
 */
int vim_motion(const int item_count, int *current_index, enum vim_mode *mode,
               const struct tb_event ev);

/**
 * @brief Normal motion
 *
 * Interactive cursor and event with common motion.
 * this motion contain vertical movement with arrow,
 * page up/down using PgUp/PgDn, Go to the top/down using HOME/END
 *
 * @param item_count        Total item to interact with the motion
 * @param current_index     Current cursor in index position
 * @param ev                user motion event trigger
 *
 * @return 0    Success
 * @return 1    Selection event
 * @return -1   Exit/Quit/Error
 *
 */
int default_motion(const int item_count, int *current_index, const struct tb_event ev);

/**
 * @brief Handle keyboard input for text input fields.
 *
 * Processes keyboard events to build a text input string. Handles character
 * input, spaces, and backspace/delete operations. Accepts printable ASCII
 * characters (32-126).
 *
 * @param input       Pointer to the input buffer to modify.
 * @param input_size  Maximum size of the input buffer (including null terminator).
 * @param ev          Terminal event containing keyboard input.
 *
 * @note The input buffer must be null-terminated before first use.
 */
void inputtext_handler(char *input, const int input_size, const struct tb_event ev);

/**
 * @brief Render text input field with cursor.
 *
 * Displays a text input prompt with the current input text and a cursor indicator.
 * The input is prefixed with "> " and shows an underscore cursor at the end.
 *
 * @param x        X coordinate for rendering.
 * @param y        Y coordinate for rendering.
 * @param input    The current input text to display.
 * @param enabled  Whether to render the input field (0 = hidden, non-zero = visible).
 */
void inputtext(const int x, const int y, const char *input, const int enabled);

/**
 * @brief Manage scrollbar and automatic scrolling for list views.
 *
 * Automatically adjusts the viewport offset to keep the currently selected item
 * visible. If the list is longer than the visible area, renders a scrollbar on
 * the right side with a position indicator.
 *
 * @param current_index  The index of the currently selected item.
 * @param item_count     Total number of items in the list.
 * @param visible_lines  Number of lines visible in the viewport.
 * @param offset         Pointer to the viewport offset (will be modified to keep selection
 * visible).
 */
void scrollbar(int current_index, int item_count, int visible_lines, int *offset);

/**
 * @brief Filter and sort items based on fuzzy search query.
 *
 * Performs fuzzy matching on the list of items using the provided query string,
 * then reorders the items array in-place so that best matches appear first.
 * Items that don't match the query are still included but appear at the bottom
 * with negative scores.
 *
 * The fuzzy matching algorithm awards points for:
 * - Character matches (case-insensitive)
 * - Consecutive character matches (bonus points)
 * - Matches at word boundaries (extra bonus)
 *
 * @param items  Array of string pointers to filter and sort (modified in-place).
 * @param count  Number of items in the array.
 * @param query  The search query string (empty string matches all items equally).
 *
 * @note This function allocates temporary memory for scoring. Returns silently on
 *       allocation failure.
 */
void list_filter(struct listview_item items[], const int count, char *query);

/**
 * @brief Display an interactive selection menu.
 *
 * Initializes termbox and enters an event loop where the user can navigate
 * a list using arrow keys or vim-style bindings (j/k). Press Enter to confirm
 * selection or 'q' to quit.
 *
 * @param title     Title
 * @param items     Array of strings to display.
 * @param count     Number of items in the array.
 * @param selected  Pointer to an integer where the selected index will be stored.
 *                  Will be set to -1 if the user quits without selecting.
 *
 * @note This function handles termbox initialization and cleanup internally.
 */
void listview(const char *title, struct listview_item items[], const int count, int *selected);

#endif
