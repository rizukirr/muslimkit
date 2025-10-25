# muslimkit

A comprehensive C-based desktop application providing essential Islamic tools and reminders for Muslim users.

## Overview

muslimkit is an all-in-one desktop application designed to help Muslims maintain their daily religious practices. The application integrates prayer time notifications, Islamic calendar reminders, and time-based sunnah recommendations to provide a complete spiritual companion for your desktop workflow.

**Note:** This project is currently under active development. Many features are planned but not yet implemented.

## Vision & Features

### Core Features

- **Prayer Time Notifications**: Automatic desktop notifications for the five daily prayers based on your location
- **Interactive City Selection**: Browse and search Indonesian cities with vim-like navigation
- **Prayer Schedule Display**: View daily prayer times in an elegant terminal interface

### Planned Features

- **Islamic Calendar Reminders**:
  - Fasting days (Ramadan, Ayyamul Bidh, Arafah, Ashura, etc.)
  - Important Islamic dates (Eid celebrations, Ramadhan counter, etc.)
  - Customizable reminder notifications

- **Muslim Time Reminders**:
  - Daily 5 times prayer
  - Jum'at sunnah reminder (reciting Al-Kahf, Shadaqah, ext)
  - Morning-Evening dzikr
  - No more idea yet

- **Additional Tools**:
  - Quran
  - Hadith
  - Hijri calendar converter
  - Customizable notification preferences

## Current Status

### Implemented ✓

- Fetch complete list of Indonesian cities from [MyQuran API](https://api.myquran.com)
- Interactive city selection interface with dual navigation modes:
  - **Default mode**: Arrow keys for navigation
  - **Vim mode**: j/k (up/down), g/G (top/bottom), Ctrl+U/Ctrl+D (page navigation), '/' (search), 'q' (quit)
  - Toggle between modes with Ctrl+/
- HTTPS/SSL connection handling
- JSON response parsing
- Chunked transfer encoding support

### In Development 🚧

- Prayer time fetching and display
- Desktop notification system

### Planned 📋

- Islamic calendar integration
- Sunnah reminder system
- Customizable notification preferences
- International prayer times reminder support
- Configuration file management

## Requirements

- CMake 4.0 or higher
- C compiler (GCC or Clang)
- OpenSSL development libraries
- valgrind (Optional for developer)

### Installing Dependencies

**Arch Linux:**

```bash
sudo pacman -S cmake openssl
```

**Debian/Ubuntu:**

```bash
sudo apt install cmake libssl-dev
```

**Fedora:**

```bash
sudo dnf install cmake openssl-devel
```

## Building

```bash
# Configure the build
cmake -B build

# Compile
cmake --build build

# Run the application
./build/muslimkit
```

## Architecture

The project follows a clean three-layer architecture designed for extensibility:

1. **Network Layer**: Low-level TCP/SSL connections and HTTP protocol handling
2. **Domain Layer**: Business logic, data models, and API integration
3. **Presentation Layer**: Terminal UI using the embedded termbox library

The architecture is designed to accommodate future expansion with dedicated subdirectories for models, services, repositories, and configuration management.

## Usage

### Current Features

1. Launch the application: `./build/muslimkit`
2. Browse and search through Indonesian cities
3. Navigate using arrow keys or vim keybindings (toggle with Ctrl+/)
4. Select your city

### Future Usage

Once fully implemented, muslimkit will run as a background service, providing:

- Automatic prayer time notifications
- Islamic calendar reminders
- Time-based sunnah recommendations
- Customizable notification settings

## Development

This project uses consistent patterns for memory management, error handling, and architectural organization. Key principles:

- Each allocating function has a corresponding `_free()` function
- Clean separation of concerns across layers
- Modular design for easy feature addition
- Header files organized by architectural layer

## API

muslimkit uses the [MyQuran API](https://api.myquran.com) for:

- Indonesian city listings
- Prayer time schedules
- Islamic calendar data

## Contributing

Contributions are welcome! Whether you want to add new features, improve existing functionality, or enhance documentation, please feel free to submit pull requests.

Please ensure your code:

- Follows the existing architectural patterns
- Maintains the memory management conventions
- Includes appropriate error handling
- Updates relevant documentation

## License

[License information to be added]

## Acknowledgments

- [MyQuran API](https://api.myquran.com) for providing Islamic prayer time data
- termbox library for terminal UI capabilities
