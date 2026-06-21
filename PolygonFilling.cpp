/*=============================================================================
 * PolygonFilling.cpp - Topic 2: Polygon Filling & Primitive Construction
 *
 * This file demonstrates polygon filling and primitive construction:
 *
 *   1. FILLED RECTANGLE (GL_QUADS):
 *      drawRect() renders solid rectangles using four vertices.
 *      Used for ship bodies, health bars, HUD panels, and menu buttons.
 *
 *   2. FILLED TRIANGLE (GL_TRIANGLES):
 *      drawTriangle() renders solid isosceles triangles.
 *      Used for ship hulls (Interceptor, Vanguard) and decorative arrows.
 *
 *   3. BRESENHAM'S LINE ALGORITHM:
 *      drawBresenhamLine() rasterizes lines pixel-by-pixel using
 *      integer arithmetic. Avoids floating-point errors by using
 *      incremental error tracking (the decision variable 'err').
 *
 *   4. MIDPOINT CIRCLE ALGORITHM:
 *      drawMidpointCircle() draws circle outlines using the midpoint
 *      decision method with 8-way symmetry. Computes only one octant
 *      and mirrors to all eight symmetric positions.
 *
 *   5. HEALTH BAR RENDERING:
 *      drawHUD() constructs health bars by layering filled rectangles:
 *      a dark background rectangle + a colored foreground rectangle
 *      whose width is proportional to the current value.
 *
 *   6. MENU UI CONSTRUCTION:
 *      drawMainMenu() builds sci-fi buttons using GL_POLYGON with
 *      angled corners, GL_LINE_LOOP outlines, and GL_TRIANGLES
 *      decorative indicators.
 *=============================================================================*/

#include "ViewingClipping.h"
#include <GL/freeglut.h>
#include <cmath>
#include <cstdlib>
#include <sstream>

/*=============================================================================
 * FILLED RECTANGLE PRIMITIVE (GL_QUADS)
 *
 * Renders a solid filled rectangle centered at (cx, cy) with width w
 * and height h. Uses GL_QUADS to fill the interior with the current
 * OpenGL color. The four vertices define the rectangle corners:
 *   bottom-left, bottom-right, top-right, top-left.
 *
 * This is the most fundamental polygon filling operation in the game,
 * used for ship bodies, health bars, HUD panels, and menu backgrounds.
 *=============================================================================*/
void drawRect(float cx, float cy, float w, float h) {
  glBegin(GL_QUADS);
  glVertex2f(cx - w / 2, cy - h / 2);
  glVertex2f(cx + w / 2, cy - h / 2);
  glVertex2f(cx + w / 2, cy + h / 2);
  glVertex2f(cx - w / 2, cy + h / 2);
  glEnd();
}

/*=============================================================================
 * FILLED TRIANGLE PRIMITIVE (GL_TRIANGLES)
 *
 * Renders a solid filled isosceles triangle centered at (cx, cy).
 * The apex points upward at (cx, cy + h/2), and the base spans
 * from (cx - w/2, cy - h/2) to (cx + w/2, cy - h/2).
 *
 * Uses GL_TRIANGLES polygon filling to rasterize the interior.
 * Used for ship hulls (Interceptor, Vanguard) and enemy shapes.
 *=============================================================================*/
void drawTriangle(float cx, float cy, float w, float h) {
  glBegin(GL_TRIANGLES);
  glVertex2f(cx, cy + h / 2);
  glVertex2f(cx - w / 2, cy - h / 2);
  glVertex2f(cx + w / 2, cy - h / 2);
  glEnd();
}

/*=============================================================================
 * BRESENHAM'S LINE DRAWING ALGORITHM
 *
 * Rasterizes a line between two points using Bresenham's integer-based
 * incremental algorithm. This is a classic CG algorithm that avoids
 * expensive floating-point multiplication by using only integer addition,
 * subtraction, and bit shifting.
 *
 * Algorithm steps:
 *   1. Calculate absolute differences dx, dy and step directions sx, sy
 *   2. Initialize error variable err = dx - dy
 *   3. Plot current pixel and advance along the major axis
 *   4. Use the error variable to decide when to step in the minor axis
 *
 * The error variable 'err' tracks the accumulated deviation from the
 * ideal line. When it exceeds a threshold, we step in the minor axis
 * direction to stay close to the mathematically perfect line.
 *=============================================================================*/
void drawBresenhamLine(float x0, float y0, float x1, float y1) {
  int ix0 = (int)std::round(x0);
  int iy0 = (int)std::round(y0);
  int ix1 = (int)std::round(x1);
  int iy1 = (int)std::round(y1);

  int dx = std::abs(ix1 - ix0);
  int dy = std::abs(iy1 - iy0);
  int sx = (ix0 < ix1) ? 1 : -1;
  int sy = (iy0 < iy1) ? 1 : -1;
  int err = dx - dy;

  glBegin(GL_POINTS);
  while (true) {
    glVertex2i(ix0, iy0);
    if (ix0 == ix1 && iy0 == iy1)
      break;
    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      ix0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      iy0 += sy;
    }
  }
  glEnd();
}

/*=============================================================================
 * MIDPOINT CIRCLE ALGORITHM (8-Way Symmetry)
 *
 * Draws a circle outline using the midpoint decision method.
 * Exploits 8-way symmetry: for each computed point (x, y), seven
 * symmetric points are plotted automatically via the plot8 lambda.
 *
 * Algorithm:
 *   1. Start at (0, radius) — the topmost point of the circle
 *   2. Decision variable d = 1 - radius (determines inside/outside)
 *   3. If d < 0: midpoint is inside the circle, move east (x++)
 *   4. If d >= 0: midpoint is outside, move southeast (x++, y--)
 *   5. Plot all 8 symmetric points at each step
 *   6. Continue while x <= y (one octant)
 *
 * This algorithm only uses integer arithmetic for the decision variable,
 * making it very efficient for real-time circle rendering.
 *=============================================================================*/
void drawMidpointCircle(float cx, float cy, float radius) {
  int x = 0;
  int y = (int)std::round(radius);
  int d = 1 - y;

  // Lambda to plot all 8 symmetric points around center (cx, cy)
  auto plot8 = [&](int px, int py) {
    glVertex2i((int)std::round(cx) + px, (int)std::round(cy) + py);
    glVertex2i((int)std::round(cx) - px, (int)std::round(cy) + py);
    glVertex2i((int)std::round(cx) + px, (int)std::round(cy) - py);
    glVertex2i((int)std::round(cx) - px, (int)std::round(cy) - py);
    glVertex2i((int)std::round(cx) + py, (int)std::round(cy) + px);
    glVertex2i((int)std::round(cx) - py, (int)std::round(cy) + px);
    glVertex2i((int)std::round(cx) + py, (int)std::round(cy) - px);
    glVertex2i((int)std::round(cx) - py, (int)std::round(cy) - px);
  };

  glBegin(GL_POINTS);
  while (x <= y) {
    plot8(x, y);
    if (d < 0) {
      d += 2 * x + 3;
    } else {
      d += 2 * (x - y) + 5;
      --y;
    }
    ++x;
  }
  glEnd();
}

/*=============================================================================
 * BULLET RENDERING (Bresenham Line)
 *
 * Player bullets are rendered as vertical lines using the Bresenham
 * line algorithm. The cyan color (0.2, 1.0, 1.0) and point size 2.0
 * create a laser-like visual effect.
 *=============================================================================*/
void Bullet::draw() const {
  glColor3f(0.2f, 1.0f, 1.0f);
  glPointSize(8.0f);
  drawBresenhamLine(x, y - 10.0f, x, y + 10.0f);
  glPointSize(1.0f);
}

/*=============================================================================
 * ENEMY BULLET RENDERING (Bresenham Line)
 *
 * Enemy bullets use the same Bresenham line algorithm but with an
 * orange color (1.0, 0.4, 0.1) to distinguish them from player bullets.
 *=============================================================================*/
void EnemyBullet::draw() const {
  glColor3f(1.0f, 0.4f, 0.1f);
  glPointSize(2.0f);
  drawBresenhamLine(x, y - 6.0f, x, y + 6.0f);
  glPointSize(1.0f);
}

/*=============================================================================
 * PLAYER SHIP RENDERING (Polygon Filling with Multiple Primitives)
 *
 * Constructs the player ship visual using layered polygon primitives:
 *
 * Engine Thrusters:
 *   - GL_TRIANGLES with additive blending (GL_ONE) for glow effect
 *   - Outer flame + inner hot core triangles
 *   - Flicker animation using sin(elapsed * frequency)
 *
 * Ship Body (varies by shipType):
 *   Type 0 (Interceptor): drawTriangle main hull + drawRect wing pods
 *   Type 1 (Aegis Tank):  drawRect main hull + drawRect side armor
 *   Type 2 (Vanguard):    drawTriangle main + drawTriangle wing cannons
 *
 * Cockpit: White drawRect(x, y+2, 6, 6) on all ship types
 *
 * Shield Bubble:
 *   - Rendered when shields > 0 using drawMidpointCircle
 *   - Alpha-blended circle outline around the ship
 *   - Flashes bright cyan-white when hitFlashTimer > 0
 *     (demonstrates Animation Principles: reaction feedback)
 *=============================================================================*/
void Player::draw() const {
  // --- Engine thruster flame (drawn behind ship, pointing downward) ---
  float elapsed = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
  float flicker = 1.0f + 0.2f * sin(elapsed * 20.0f);
  float flicker2 = 1.0f + 0.2f * sin(elapsed * 22.0f + 1.5f);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for glow

  if (shipType == 0) {
    // Interceptor: single central thruster
    float fh = 16.0f * flicker;
    glColor4f(0.2f, 0.7f, 1.0f, 0.7f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 6.0f, y - h / 2);
    glVertex2f(x + 6.0f, y - h / 2);
    glVertex2f(x, y - h / 2 - fh);
    glEnd();
    // Hot inner core
    glColor4f(0.8f, 0.95f, 1.0f, 0.6f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 3.0f, y - h / 2);
    glVertex2f(x + 3.0f, y - h / 2);
    glVertex2f(x, y - h / 2 - fh * 0.55f);
    glEnd();
  } else if (shipType == 1) {
    // Aegis Tank: dual wide thrusters
    float fh = 14.0f * flicker;
    float fh2v = 14.0f * flicker2;
    // Left thruster
    glColor4f(0.1f, 0.5f, 1.0f, 0.65f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 14 - 4, y - h / 2);
    glVertex2f(x - 14 + 4, y - h / 2);
    glVertex2f(x - 14, y - h / 2 - fh);
    glEnd();
    // Right thruster
    glBegin(GL_TRIANGLES);
    glVertex2f(x + 14 - 4, y - h / 2);
    glVertex2f(x + 14 + 4, y - h / 2);
    glVertex2f(x + 14, y - h / 2 - fh2v);
    glEnd();
    // Inner cores
    glColor4f(0.7f, 0.9f, 1.0f, 0.5f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 14 - 2, y - h / 2);
    glVertex2f(x - 14 + 2, y - h / 2);
    glVertex2f(x - 14, y - h / 2 - fh * 0.5f);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex2f(x + 14 - 2, y - h / 2);
    glVertex2f(x + 14 + 2, y - h / 2);
    glVertex2f(x + 14, y - h / 2 - fh2v * 0.5f);
    glEnd();
  } else {
    // Vanguard: triple thrusters (red-orange)
    float fh = 14.0f * flicker;
    float fhL = 10.0f * flicker2;
    float fhR = 10.0f * (1.0f + 0.2f * sin(elapsed * 24.0f + 3.0f));
    // Center
    glColor4f(1.0f, 0.4f, 0.1f, 0.7f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 5.0f, y - h / 2);
    glVertex2f(x + 5.0f, y - h / 2);
    glVertex2f(x, y - h / 2 - fh);
    glEnd();
    // Left wing thruster
    glColor4f(1.0f, 0.5f, 0.0f, 0.55f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 15 - 3, y - h / 2 + 4);
    glVertex2f(x - 15 + 3, y - h / 2 + 4);
    glVertex2f(x - 15, y - h / 2 + 4 - fhL);
    glEnd();
    // Right wing thruster
    glBegin(GL_TRIANGLES);
    glVertex2f(x + 15 - 3, y - h / 2 + 4);
    glVertex2f(x + 15 + 3, y - h / 2 + 4);
    glVertex2f(x + 15, y - h / 2 + 4 - fhR);
    glEnd();
    // Center inner core
    glColor4f(1.0f, 0.9f, 0.5f, 0.5f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 2.5f, y - h / 2);
    glVertex2f(x + 2.5f, y - h / 2);
    glVertex2f(x, y - h / 2 - fh * 0.5f);
    glEnd();
  }
  glDisable(GL_BLEND);

  // --- Ship body (polygon filling with drawRect and drawTriangle) ---
  if (hitFlashTimer > 0) {
    glColor3f(1.0f, 0.0f, 0.0f);
  } else {
    if (shipType == 0)
      glColor3f(0.2f, 0.9f, 1.0f);
    else if (shipType == 1)
      glColor3f(0.1f, 0.6f, 0.9f);
    else
      glColor3f(0.9f, 0.2f, 0.2f);
  }

  if (shipType == 0) {
    drawTriangle(x, y, w, h);
    glColor3f(0.1f, 0.5f, 0.9f);
    drawRect(x - 18, y - 8, 10, 14);
    drawRect(x + 18, y - 8, 10, 14);
  } else if (shipType == 1) {
    drawRect(x, y, w, h);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawRect(x - 14, y + 2, 6, h);
    drawRect(x + 14, y + 2, 6, h);
  } else {
    drawTriangle(x, y, w, h);
    glColor3f(0.5f, 0.1f, 0.1f);
    drawTriangle(x - 15, y - 4, 12, h - 6);
    drawTriangle(x + 15, y - 4, 12, h - 6);
  }

  // Cockpit (small white rectangle)
  glColor3f(1.0f, 1.0f, 1.0f);
  drawRect(x, y + 2, 6, 6);

  // --- Shield bubble (midpoint circle algorithm) ---
  // Demonstrates reaction feedback: flashes bright when hit
  if (shields > 0) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (hitFlashTimer > 0) {
      glColor4f(0.8f, 1.0f, 1.0f, 0.95f); // Flashing bright cyan-white
    } else {
      glColor4f(0.0f, 0.8f, 1.0f, 0.4f); // Regular blue shield
    }
    glPointSize(2.0f);
    drawMidpointCircle(x, y, w + 6.0f);
    glDisable(GL_BLEND);
  }
}

/*=============================================================================
 * HUD RENDERING (Health Bar Polygon Filling)
 *
 * The HUD demonstrates health bar rendering through layered rectangles:
 *   1. Dark background rectangle (full width) — the bar track
 *   2. Colored foreground rectangle (proportional width) — the bar fill
 *
 * Hull bars use discrete segments (individual drawRect calls per HP point).
 * Shield bars also use discrete segments.
 * Base shield bar uses a continuous fill scaled by percentage.
 *=============================================================================*/
void Game::drawHUD() {
  // Semi-transparent HUD background panel
  glColor4f(0.02f, 0.02f, 0.1f, 0.5f);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  drawRect(WIN_W / 2, WIN_H - 18, WIN_W, 36);
  glDisable(GL_BLEND);

  // Score, wave, and level text
  glColor3f(1.0f, 1.0f, 1.0f);
  std::stringstream sw;
  sw << "Score: " << score << "  Wave: " << wave << "  Lvl: " << level;
  drawText(10, WIN_H - 22, sw.str(), GLUT_BITMAP_HELVETICA_12);

  // Scrap credits
  std::stringstream sc;
  sc << "Scrap: " << credits;
  glColor3f(1.0f, 0.85f, 0.0f);
  drawText(200, WIN_H - 22, sc.str(), GLUT_BITMAP_HELVETICA_12);

  // --- Hull health bar (discrete segments using filled rectangles) ---
  glColor3f(1.0f, 1.0f, 1.0f);
  drawText(280, WIN_H - 22, "HULL:", GLUT_BITMAP_HELVETICA_12);
  glColor3f(0.3f, 0.0f, 0.0f); // Dark red background
  drawRect(348, WIN_H - 18, player.maxHull * 12, 10);
  glColor3f(0.2f, 0.9f, 0.2f); // Green fill segments
  for (int i = 0; i < player.hull; ++i) {
    drawRect(348 - (player.maxHull * 12) / 2.0f + 6.0f + i * 12.0f, WIN_H - 18,
             10, 8);
  }

  // --- Shield health bar (discrete segments using filled rectangles) ---
  glColor3f(1.0f, 1.0f, 1.0f);
  drawText(420, WIN_H - 22, "SHIELD:", GLUT_BITMAP_HELVETICA_12);
  glColor3f(0.0f, 0.1f, 0.3f); // Dark blue background
  drawRect(502, WIN_H - 18, player.maxShields * 12, 10);
  glColor3f(0.0f, 0.8f, 1.0f); // Cyan fill segments
  for (int i = 0; i < player.shields; ++i) {
    drawRect(502 - (player.maxShields * 12) / 2.0f + 6.0f + i * 12.0f,
             WIN_H - 18, 10, 8);
  }

  // --- Base shield bar (continuous fill scaled by percentage) ---
  glColor3f(1.0f, 1.0f, 1.0f);
  drawText(575, WIN_H - 22, "BASE:", GLUT_BITMAP_HELVETICA_12);
  glColor3f(0.2f, 0.2f, 0.2f); // Gray background
  drawRect(670, WIN_H - 18, 100, 10);
  glColor3f(0.8f, 0.0f, 1.0f); // Purple fill
  float pct = (float)baseShields / maxBaseShields;
  drawRect(670 - 50.0f + pct * 50.0f, WIN_H - 18, 100 * pct, 8);

  // Pause overlay
  if (state == PAUSED) {
    glColor3f(1.0f, 1.0f, 0.0f);
    drawText(WIN_W / 2 - 20, WIN_H / 2, "PAUSED", GLUT_BITMAP_HELVETICA_12);
  }

  // Game Over overlay
  if (state == GAME_OVER) {
    glColor3f(1.0f, 0.2f, 0.2f);
    drawText(WIN_W / 2 - 30, WIN_H / 2 + 20, "GAME OVER",
             GLUT_BITMAP_HELVETICA_12);
    glColor3f(1.0f, 1.0f, 1.0f);
    if (level == 2) {
      drawText(WIN_W / 2 - 55, WIN_H / 2 - 10, "Press R to retry Level 2",
               GLUT_BITMAP_HELVETICA_12);
    } else {
      drawText(WIN_W / 2 - 45, WIN_H / 2 - 10, "Press R to restart",
               GLUT_BITMAP_HELVETICA_12);
    }
  }
}

/*=============================================================================
 * MAIN MENU RENDERING (Polygon Construction)
 *
 * Constructs a sci-fi themed main menu using multiple polygon types:
 *   - GL_POLYGON: Angled-corner buttons with gradient fill
 *   - GL_LINE_LOOP: Cyberpunk-style button outlines
 *   - GL_TRIANGLES: Decorative arrow indicators on buttons
 *   - GL_LINES: Accent lines flanking title text
 *
 * Each button is built from layers:
 *   1. Gradient-filled polygon background (dark blue gradient)
 *   2. Cyan outline for sci-fi aesthetic
 *   3. Orange triangle indicator on the left
 *   4. White text label
 *=============================================================================*/
void Game::drawMainMenu() {
  // 1. Draw Title Accents (Modern Sci-Fi Lines)
  glLineWidth(2.0f);
  glBegin(GL_LINES);
  glColor3f(0.2f, 0.9f, 1.0f); // Cyan
  glVertex2f(WIN_W / 2 - 200, WIN_H / 2 + 160);
  glVertex2f(WIN_W / 2 - 180, WIN_H / 2 + 160);

  glVertex2f(WIN_W / 2 + 180, WIN_H / 2 + 160);
  glVertex2f(WIN_W / 2 + 200, WIN_H / 2 + 160);

  glColor3f(1.0f, 0.8f, 0.2f); // Orange
  glVertex2f(WIN_W / 2 - 110, WIN_H / 2 + 110);
  glVertex2f(WIN_W / 2 - 100, WIN_H / 2 + 110);

  glVertex2f(WIN_W / 2 + 100, WIN_H / 2 + 110);
  glVertex2f(WIN_W / 2 + 110, WIN_H / 2 + 110);
  glEnd();

  // 2. Title Text
  glColor3f(0.2f, 0.9f, 1.0f);
  drawText(WIN_W / 2 - 45, WIN_H / 2 + 150, "SPACE ODYSSEY",
           GLUT_BITMAP_HELVETICA_12);
  glColor3f(1.0f, 0.8f, 0.2f);
  drawText(WIN_W / 2 - 48, WIN_H / 2 + 120, "ROGUE STARSHIP",
           GLUT_BITMAP_HELVETICA_12);

  std::string btns[3] = {"[1] START GAME", "[2] HOW TO PLAY", "[3] EXIT"};
  float ys[3] = {WIN_H / 2.0f + 30.0f, WIN_H / 2.0f - 30.0f,
                 WIN_H / 2.0f - 90.0f};

  for (int i = 0; i < 3; ++i) {
    float bx = WIN_W / 2.0f;
    float by = ys[i];
    float bw = 260.0f;
    float bh = 45.0f;
    float corner = 12.0f; // Sci-fi angled corners

    // 3. Gradient filled polygon background for button (2D Filling)
    glBegin(GL_POLYGON);
    glColor3f(0.04f, 0.08f, 0.2f); // Darker blue at top
    glVertex2f(bx - bw / 2 + corner, by + bh / 2);
    glVertex2f(bx + bw / 2 - corner, by + bh / 2);

    glColor3f(0.1f, 0.3f, 0.6f); // Lighter blue at bottom
    glVertex2f(bx + bw / 2, by + bh / 2 - corner);
    glVertex2f(bx + bw / 2, by - bh / 2 + corner);
    glVertex2f(bx + bw / 2 - corner, by - bh / 2);
    glVertex2f(bx - bw / 2 + corner, by - bh / 2);
    glVertex2f(bx - bw / 2, by - bh / 2 + corner);
    glVertex2f(bx - bw / 2, by + bh / 2 - corner);
    glEnd();

    // 4. Cyberpunk outline (2D Drawing)
    glColor3f(0.2f, 0.9f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(bx - bw / 2 + corner, by + bh / 2);
    glVertex2f(bx + bw / 2 - corner, by + bh / 2);
    glVertex2f(bx + bw / 2, by + bh / 2 - corner);
    glVertex2f(bx + bw / 2, by - bh / 2 + corner);
    glVertex2f(bx + bw / 2 - corner, by - bh / 2);
    glVertex2f(bx - bw / 2 + corner, by - bh / 2);
    glVertex2f(bx - bw / 2, by - bh / 2 + corner);
    glVertex2f(bx - bw / 2, by + bh / 2 - corner);
    glEnd();

    // 5. Decorative filled triangle on the left side (2D Filling)
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.8f, 0.2f); // Orange
    glVertex2f(bx - bw / 2 + 15.0f, by);
    glVertex2f(bx - bw / 2 + 23.0f, by + 5.0f);
    glVertex2f(bx - bw / 2 + 23.0f, by - 5.0f);
    glEnd();

    // 6. Button Text
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(bx - btns[i].length() * 3.5f + 15.0f, by - 5.0f, btns[i],
             GLUT_BITMAP_HELVETICA_12);
  }
}

/*=============================================================================
 * GUIDELINES SCREEN (Polygon Construction for UI)
 *
 * Constructs the instructions screen with:
 *   - Title text rendering
 *   - Instruction text lines
 *   - A "Back" button using drawRect (filled rectangle) + GL_LINE_LOOP outline
 *=============================================================================*/
void Game::drawGuidelines() {
  glColor3f(0.2f, 0.9f, 1.0f);
  drawText(WIN_W / 2 - 75, WIN_H - 100, "HOW TO PLAY / INSTRUCTIONS",
           GLUT_BITMAP_HELVETICA_12);

  glColor3f(1.0f, 1.0f, 1.0f);
  drawText(80, WIN_H - 160,
           "- Control your ship using Arrow Keys or WASD keys.",
           GLUT_BITMAP_HELVETICA_12);
  drawText(80, WIN_H - 190, "- Press Spacebar to fire your lasers.",
           GLUT_BITMAP_HELVETICA_12);
  drawText(80, WIN_H - 220, "- Dodge incoming orange enemy plasma bolts.",
           GLUT_BITMAP_HELVETICA_12);
  drawText(80, WIN_H - 250,
           "- Do not let enemies bypass the bottom boundary. It drains base "
           "shields!",
           GLUT_BITMAP_HELVETICA_12);
  drawText(80, WIN_H - 280,
           "- Destroy enemies to collect golden Scrap Credits. Use them to "
           "upgrade in the shop.",
           GLUT_BITMAP_HELVETICA_12);
  drawText(80, WIN_H - 310,
           "- Defeat the Heavy Carrier Boss on Wave 5 to survive.",
           GLUT_BITMAP_HELVETICA_12);
  drawText(80, WIN_H - 340,
           "- Press F to trigger a Refract Wave that reflects bullets and "
           "damages nearby enemies.",
           GLUT_BITMAP_HELVETICA_12);

  float bx = WIN_W / 2.0f;
  float by = 100.0f;
  float bw = 200.0f;
  float bh = 40.0f;

  // Filled rectangle button background
  glColor3f(0.08f, 0.15f, 0.3f);
  drawRect(bx, by, bw, bh);

  // Button outline
  glColor3f(0.2f, 0.9f, 1.0f);
  glBegin(GL_LINE_LOOP);
  glVertex2f(bx - bw / 2, by - bh / 2);
  glVertex2f(bx + bw / 2, by - bh / 2);
  glVertex2f(bx + bw / 2, by + bh / 2);
  glVertex2f(bx - bw / 2, by + bh / 2);
  glEnd();

  glColor3f(1.0f, 1.0f, 1.0f);
  drawText(bx - 45, by - 6, "[B] BACK TO MENU", GLUT_BITMAP_HELVETICA_12);
}

/*=============================================================================
 * VICTORY SCREEN (Polygon Construction)
 *
 * Displays final score with decorative horizontal lines (GL_LINES)
 * and text elements arranged in a centered layout.
 *=============================================================================*/
void Game::drawVictoryScreen() {
  // Title
  glColor3f(1.0f, 0.85f, 0.0f);
  drawText(WIN_W / 2 - 55, WIN_H / 2 + 140, "CONGRATULATIONS!",
           GLUT_BITMAP_HELVETICA_12);

  glColor3f(0.2f, 0.9f, 1.0f);
  drawText(WIN_W / 2 - 40, WIN_H / 2 + 100, "YOU HAVE WON!",
           GLUT_BITMAP_HELVETICA_12);

  // Decorative lines
  glLineWidth(2.0f);
  glColor3f(1.0f, 0.85f, 0.0f);
  glBegin(GL_LINES);
  glVertex2f(WIN_W / 2 - 150, WIN_H / 2 + 80);
  glVertex2f(WIN_W / 2 + 150, WIN_H / 2 + 80);
  glEnd();

  // Stats
  glColor3f(1.0f, 1.0f, 1.0f);
  std::stringstream scr;
  scr << "FINAL SCORE: " << score;
  drawText(WIN_W / 2 - 40, WIN_H / 2 + 45, scr.str(),
           GLUT_BITMAP_HELVETICA_12);

  std::stringstream wv;
  wv << "WAVES SURVIVED: " << wave;
  drawText(WIN_W / 2 - 45, WIN_H / 2 + 15, wv.str(),
           GLUT_BITMAP_HELVETICA_12);

  std::stringstream cr;
  cr << "SCRAP COLLECTED: " << credits;
  drawText(WIN_W / 2 - 50, WIN_H / 2 - 15, cr.str(),
           GLUT_BITMAP_HELVETICA_12);

  // Decorative lines
  glLineWidth(2.0f);
  glColor3f(1.0f, 0.85f, 0.0f);
  glBegin(GL_LINES);
  glVertex2f(WIN_W / 2 - 150, WIN_H / 2 - 40);
  glVertex2f(WIN_W / 2 + 150, WIN_H / 2 - 40);
  glEnd();

  // Prompt to go back
  glColor3f(0.6f, 0.6f, 0.6f);
  drawText(WIN_W / 2 - 80, WIN_H / 2 - 80, "Press R to return to Main Menu",
           GLUT_BITMAP_HELVETICA_12);
}

/*=============================================================================
 * TEXT RENDERING UTILITY
 *
 * Renders a string character-by-character using GLUT bitmap fonts.
 * Sets the raster position and iterates through each character.
 *=============================================================================*/
void Game::drawText(float x, float y, const std::string &s, void *font) {
  glRasterPos2f(x, y);
  for (char c : s) {
    glutBitmapCharacter(font, c);
  }
}
