/**
 * @file main.c
 * @brief muslimkit - Islamic prayer times terminal application
 *
 * A terminal-based application that fetches and displays Islamic prayer times
 * for Indonesian cities using the MyQuran API. Features an interactive city
 * selection interface with vim-like keybindings powered by the termbox library.
 */

/** @brief Define termbox implementation - must be defined before including termbox.h */
#define TB_IMPL

#include "include/domain/get_cities.h"
#include "include/presentation/uikit.h"

/**
 * @brief Application entry point
 *
 * Workflow:
 * 1. Fetches the list of Indonesian cities from the MyQuran API
 * 2. Displays an interactive terminal UI for city selection
 * 3. Returns the selected city (currently exits after selection)
 *
 * The application uses the termbox library for rendering a text-based UI
 * with support for both default arrow key navigation and vim keybindings.
 * Users can toggle between motion modes using Ctrl+/.
 *
 * @return 0 on success, 1 if city fetching fails
 *
 * @note Currently, the application only fetches cities and displays the
 *       selection UI. Prayer time fetching and display functionality is
 *       planned but not yet implemented.
 *
 * Memory management:
 * - Allocates memory for city data via get_city()
 * - Creates a stack-allocated listview_item array for UI rendering
 * - Properly frees all allocated memory before exit
 */
int main() {
  /* Fetch list of Indonesian cities from MyQuran API */
  struct cities_s *cities = get_city();

  /* Handle error: city data failed to fetch */
  if (cities == NULL) {
    get_city_free(cities); /* Safe to call on NULL pointer */
    printf("cities NULL\n");
    return 1;
  }

  /* Verify city data is valid before processing */
  if (cities->data != NULL && cities->size > 0) {
    /* Prepare UI data structure: convert cities to listview items */
    struct listview_item location[cities->size];
    memset(location, 0, sizeof(location)); /* Zero-initialize array */

    /* Map city data to listview items for UI rendering */
    for (size_t i = 0; i < cities->size; i++) {
      location[i].id = cities->data[i].id;       /* City ID (e.g., "1301") */
      location[i].name = cities->data[i].lokasi; /* City name (e.g., "Jakarta") */
    }

    /* Display interactive city selection UI */
    int selected = 0; /* Index of selected city (modified by listview) */
    char *title = "Choice your city";

    /*
     * Launch termbox UI with vim keybinding support:
     * - Default mode: Arrow keys, Enter to select, Esc to cancel
     * - Vim mode: j/k navigation, g/G (top/bottom), Ctrl+U/D (page up/down), '/' search
     * - Toggle modes with Ctrl+/
     */
    listview(title, location, cities->size, &selected);

    /* Display selected city (placeholder for future prayer time display) */
    printf("You are select %s[%s] Cities\n", location[selected].name, location[selected].id);
  }

  /* Clean up allocated memory before exit */
  get_city_free(cities);
  return 0;
}
