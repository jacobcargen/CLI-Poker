Very basic networking in c++

This is a game supporting a few players and allows them to play poker through a terminal on LAN.

## Building and Running (Apple Silicon / macOS)

### Build the Project

Use the provided build script:

```bash
./build_cpp.sh
```

Or compile manually:

```bash
clang++ -std=c++17 -arch arm64 src/*.cpp -o run.out
```

### Run the Application

```bash
./run.out
```

### VS Code Build Task

You can also build using VS Code's build task (⌘+Shift+B), which is configured for Apple Silicon (ARM64) architecture.

## Usage

When you run the application, you'll see a menu:

- **Host (h)**: Start a poker game as the host
- **Join (j)**: Join an existing game as a client
- **Quit (q)**: Exit the application

