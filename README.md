# 🌀 Multi-Level Maze Generator Game (Raylib Edition)

A **multi-level maze generation and exploration game** written in **C** and powered by **[raylib](https://www.raylib.com/)** for rendering.  
It uses **Depth-First Search (DFS)** to procedurally generate unique mazes, places features like stairs, and lets you navigate from a **start (S)** to an **exit (E)** across multiple levels.

---

## 📜 Features
- **Procedural maze generation** — new maze every run.
- **Multiple interconnected levels** with stairs:
  - `>` : Stairs down
  - `<` : Stairs up
- **Start & Exit**:
  - `S` : Start position on Level 0
  - `E` : Exit point on the last level
- **Randomized elements** for replayability.
- **Raylib-powered graphics** for smooth rendering.

---

## 🧩 How It Works
1. The first level starts with the player at `(1, 1)`.
2. Maze generated using **DFS algorithm**.
3. Stairs connect levels:
   - Downstairs (`>`) go deeper
   - Upstairs (`<`) return to upper levels
4. Player starts with:
   - Score **100**
   - No key (`hasKey = 0`)
5. Goal: Reach the **Exit (E)** on the final level.

---

## 🔧 Requirements
- **C Compiler**: GCC, Clang, or MSVC
- **raylib** (v4.5+ recommended)
- Terminal or console for running the game
