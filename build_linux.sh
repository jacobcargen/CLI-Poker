#!/bin/bash
g++ src/server.cpp src/host.cpp src/game.cpp src/ui.cpp -std=c++17 -o server.out
g++ src/client.cpp src/host.cpp src/game.cpp src/ui.cpp src/main.cpp -std=c++17 -o run.out
