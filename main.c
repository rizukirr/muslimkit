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
#include "include/domain/get_prayer_times.h"
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
  struct cities_s cities;
  memset(&cities, 0, sizeof(struct cities_s));
  int get_cities = get_city(&cities);

  /* Handle error: city data failed to fetch */
  if (get_cities < 0) {
    get_city_free(&cities); /* Safe to call on NULL pointer */
    printf("cities NULL\n");
    return 1;
  }

  /* Verify city data is valid before processing */
  if (cities.data != NULL && cities.size > 0) {
    /* Prepare UI data structure: convert cities to listview items */
    struct listview_item location[cities.size];
    memset(location, 0, sizeof(location)); /* Zero-initialize array */

    /* Map city data to listview items for UI rendering */
    for (size_t i = 0; i < cities.size; i++) {
      location[i].id = cities.data[i].id;       /* City ID (e.g., "1301") */
      location[i].name = cities.data[i].lokasi; /* City name (e.g., "Jakarta") */
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
    listview(title, location, cities.size, &selected);

    struct prayer_times prayer_t;
    memset(&prayer_t, 0, sizeof(prayer_t));
    int get_prayer = get_prayer_times(location[selected].id, &prayer_t);
    if (get_prayer < 0) {
      get_city_free(&cities);
      return 1;
    }

    printf("Schedule size: %d\n", prayer_t.data.schedule_size);

    for (int i = 0; i < prayer_t.data.schedule_size; i++) {
      printf("{\n");
      printf("  'date':'%s'\n", prayer_t.data.schedule[i].date);
      printf("  'fajr':'%s'\n", prayer_t.data.schedule[i].fajr);
      printf("  'dhuha':'%s'\n", prayer_t.data.schedule[i].dhuha);
      printf("  'dzuhr':'%s'\n", prayer_t.data.schedule[i].dzuhr);
      printf("  'ashr':'%s'\n", prayer_t.data.schedule[i].ashr);
      printf("  'magrib':'%s'\n", prayer_t.data.schedule[i].maghrib);
      printf("  'isya':'%s'\n", prayer_t.data.schedule[i].isya);
      printf("}\n");
    }

    get_prayer_times_free(&prayer_t);
  }

  /* Clean up allocated memory before exit */
  get_city_free(&cities);
  return 0;
}
