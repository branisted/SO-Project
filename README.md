# Treasure Hunt System

This is a UNIX-based C project for my Operating Systems (OS) course.

## Project Structure

The source files are located in the `src/` directory. The project builds the following executables:

- `treasure_manager` — manages treasure data (creation, storage, updates)
- `treasure_hub` — interactive command-line hub for launching and managing hunts
- `monitor` — background process for monitoring treasure hunts
- `calculate_score` — utility for computing player scores based on owned treasures

## Build Instructions

To compile the project, make sure you have `gcc` installed, then use the included `Makefile`.

### Build All Binaries

```bash
make tmgr     # Builds treasure_manager
make thub     # Builds treasure_hub
make tmon     # Builds monitor
make calc     # Builds calculate_score
make clean    # Cleans build files
make help     # Displays a help menu
```

### Dependencies

GCC (tested with GCC 9+)
A POSIX-compliant environment (e.g., Linux or macOS)

### File Overview

| File                     | Description                                 |
| ------------------------ | ------------------------------------------- |
| `src/treasure_mgr_lib.c` | Shared library for treasure file operations |
| `src/treasure_manager.c` | Main file for managing treasure data        |
| `src/treasure_hub.c`     | Command-line interface and control center   |
| `src/monitor.c`          | Monitor process for tracking ongoing hunts  |
| `src/calculate_score.c`  | Calculates total treasure values per player |

## License

This project is purely for educational purposes.