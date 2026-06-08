# 🚀 Space Odyssey: Rogue Starship

A 2D space shooter game built with **C++** and **OpenGL (freeGLUT)** for a university Computer Graphics course (Semester 4).

![Language](https://img.shields.io/badge/Language-C++-blue)
![Graphics](https://img.shields.io/badge/Graphics-OpenGL%20%7C%20freeGLUT-green)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey)

## 🎮 Game Overview

Defend your starbase from waves of enemy ships. Destroy enemies to collect **scrap credits** and upgrade your ship between waves. Survive boss encounters every 5th wave to keep pushing forward.

### Features

- 🚀 3 selectable starships with unique stats
- 👾 4 enemy types including a heavy boss carrier
- 🛒 Upgrade shop between waves (weapons, hull repair, shields)
- 💥 Particle effects and screen shake feedback
- ⭐ Scrolling star field background
- 📈 Progressive wave difficulty scaling

## 🎯 Controls

| Key | Action |
|-----|--------|
| `Arrow Keys` / `WASD` | Move ship |
| `Spacebar` | Fire weapons |
| `P` | Pause / Unpause |
| `ESC` | Exit game |
| `1` `2` `3` | Menu selections / Shop upgrades |
| `B` | Back to previous menu |
| `Enter` | Launch next wave (in shop) |
| `R` | Restart (on game over) |

## 🛸 Ship Classes

| Ship | Speed | Shields | Special |
|------|-------|---------|---------|
| **Interceptor** | Fast (6.5) | Weak (2) | Focused fire |
| **Aegis Tank** | Slow (4.5) | Heavy (4) | High durability |
| **Vanguard** | Medium (5.8) | Light (1) | Triple shot |

## 📁 Project Structure

```
spaceshooter/
├── main.cpp            Entry point, GLUT callbacks, game loop
├── Game.h              Game class declaration
├── Game.cpp            Game logic, rendering, input handling
├── Entities.h          Entity structs (Player, Enemy, Bullet, etc.)
├── Entities.cpp        Entity update and draw implementations
├── spaceshooter.cbp    Code::Blocks project file
└── README.md           This file
```

## 🔧 Building

### Using Code::Blocks (Recommended)

1. Open `spaceshooter.cbp` in Code::Blocks
2. Press `F9` to build and run

### Using Command Line (MinGW)

```bash
g++ main.cpp Game.cpp Entities.cpp -o spaceshooter.exe -lglut32 -lopengl32 -lglu32 -std=c++11
```

> **Note:** Make sure MinGW's `bin` directory is in your system PATH. If using Code::Blocks MinGW:
> ```
> set PATH=C:\Program Files\CodeBlocks\MinGW\bin;%PATH%
> ```

## 📦 Dependencies

- **MinGW** (g++ 8.1.0 or later)
- **freeGLUT / GLUT32**
- **OpenGL32**
- **GLU32**

All dependencies come bundled with Code::Blocks MinGW installation.

## 👥 Team

University group project — Semester 4, Computer Graphics course.

## 📄 License

This project is developed for educational purposes.
