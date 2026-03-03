CLI-Poker is a LAN poker game supporting 2 to 8 players. It is written in C++ and runs in a terminal.

## Platform Support

- **macOS**: Universal binary supporting both Apple Silicon (arm64) and Intel (x86_64)
- **Linux**: Native builds
- **Windows**: Via WSL or native compilation (using build_cpp.bat)

## Architecture

The project uses standard POSIX networking APIs (sys/socket.h, arpa/inet.h) which are compatible across Unix-like systems including macOS and Linux. The universal binary approach on macOS ensures optimal performance on both Apple Silicon and Intel processors.

