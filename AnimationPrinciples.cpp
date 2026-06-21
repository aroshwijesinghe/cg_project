/*=============================================================================
 * AnimationPrinciples.cpp - Topic 5: Animation Principles (Timing & Staging)
 *
 * This file demonstrates key animation principles from CG:
 *
 *   1. FRAMERATE MANAGEMENT (Timing):
 *      The game uses glutTimerFunc(16, timer, 0) in main.cpp to call the
 *      update loop approximately every 16ms (~60fps). This ensures smooth
 *      animation by maintaining consistent frame timing. All time-based
 *      values (timers, cooldowns) are tuned to this 16ms frame rate.
 *
 *   2. SCROLLING STAR PARALLAX (Staging/Depth):
 *      StarField creates 80 stars at different speeds (0.5 to 2.5 units/frame).
 *      Faster stars appear closer (foreground), slower stars appear distant
 *      (background). This parallax effect creates an illusion of depth
 *      and forward motion through space.
 *
 *   3. MULTI-LAYER NEBULA PARALLAX (Staging/Depth):
 *      NebulaField implements a two-layer parallax system:
 *        - Layer 1 (Background): Large nebula puffs drift slowly (0.15-0.4 u/f)
 *        - Layer 2 (Foreground): Small stars scroll faster (0.3-1.3 u/f)
 *      This depth separation creates a rich, immersive space environment.
 *
 *   4. ENEMY SPAWN TIMING:
 *      spawnEnemy() is called at timed intervals that decrease with wave
 *      number, creating escalating difficulty. The spawn interval is:
 *        interval = max(25, 75 - wave * 4) frames
 *      This timing creates proper pacing and dramatic tension.
 *
 *   5. UI SCREEN STAGING:
 *      Ship selection, upgrade shop, and level transition screens
 *      arrange UI elements spatially to guide the player's attention
 *      through the game flow (staging principle).
 *
 *   6. FOLLOW-THROUGH & REACTION FEEDBACK:
 *      Engine thruster flicker uses sin(elapsed * frequency) to create
 *      organic, alive-feeling animations. Shield flash on hit provides
 *      immediate visual feedback.
 *=============================================================================*/

#include "ViewingClipping.h"
#include <GL/freeglut.h>
#include <cstdlib>
#include <cmath>
#include <sstream>

/*=============================================================================
 * STAR FIELD INITIALIZATION (Staging Setup)
 *
 * Creates 80 stars distributed randomly across the viewport.
 * Each star has a different scrolling speed (0.5 to 2.5), implementing
 * the parallax depth effect: faster stars appear closer to the camera,
 * slower stars appear farther away.
 *
 * This speed variation is the key to the parallax staging effect.
 *=============================================================================*/
void StarField::init() {
    stars.clear();
    for (int i = 0; i < 80; ++i) {
        Star s;
        s.x = (float)(rand() % WIN_W);
        s.y = (float)(rand() % WIN_H);
        s.speed = 0.5f + (rand() % 100) / 100.0f * 2.0f;  // Parallax speed variation
        stars.push_back(s);
    }
}

/*=============================================================================
 * STAR FIELD SCROLLING UPDATE (Parallax Animation)
 *
 * Each frame, every star scrolls downward at its individual speed.
 * This creates the parallax motion effect:
 *   - Fast stars (speed ~2.5) scroll quickly — appear as nearby objects
 *   - Slow stars (speed ~0.5) scroll slowly — appear as distant objects
 *
 * When a star exits the bottom of the viewport (y < 0), it wraps
 * around to the top with a new random X position, creating an
 * infinite scrolling background.
 *
 * This per-frame update is driven by glutTimerFunc at ~60fps,
 * demonstrating the Timing animation principle.
 *=============================================================================*/
void StarField::update() {
    for (auto& s : stars) {
        s.y -= s.speed;           // Downward translation at individual speed
        if (s.y < 0) {
            s.y = (float)WIN_H;   // Wrap to top
            s.x = (float)(rand() % WIN_W);  // Randomize X position
        }
    }
}

/*=============================================================================
 * STAR FIELD RENDERING (Staging Visual)
 *
 * Renders all stars as white GL_POINTS, creating a starfield backdrop.
 * The visual simplicity (just points) combined with parallax motion
 * creates an effective space environment with minimal rendering cost.
 *=============================================================================*/
void StarField::draw() const {
    glColor3f(1.0f, 1.0f, 1.0f);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (const Star& s : stars) {
        glVertex2f(s.x, s.y);
    }
    glEnd();
}

/*=============================================================================
 * NEBULA FIELD INITIALIZATION (Multi-Layer Staging Setup)
 *
 * Creates a two-layer parallax background for Level 2:
 *
 * Layer 1 — Background Stars (40 stars):
 *   Speed range: 0.3 to 1.3 units/frame (foreground parallax layer)
 *
 * Layer 2 — Nebula Puffs (15 puffs):
 *   Speed range: 0.15 to 0.4 units/frame (background parallax layer)
 *   Colors: Purple, Magenta, Teal, Dark Magenta, Deep Purple
 *   Variable radii (40-120 pixels) and low alpha (0.08-0.2)
 *
 * The speed difference between the two layers creates depth:
 * puffs drift slowly behind the faster-moving stars.
 *=============================================================================*/
void NebulaField::init() {
    puffs.clear();
    stars.clear();
    // Create dim background stars (foreground parallax layer — faster)
    for (int i = 0; i < 40; ++i) {
        Star s;
        s.x = (float)(rand() % WIN_W);
        s.y = (float)(rand() % WIN_H);
        s.speed = 0.3f + (rand() % 100) / 100.0f * 1.0f;
        stars.push_back(s);
    }
    // Create nebula puffs with purple/magenta/teal colors (background layer — slower)
    float colors[][3] = {
        {0.4f, 0.1f, 0.6f},  // Purple
        {0.6f, 0.1f, 0.5f},  // Magenta
        {0.1f, 0.3f, 0.5f},  // Teal
        {0.5f, 0.0f, 0.4f},  // Dark magenta
        {0.2f, 0.1f, 0.5f},  // Deep purple
    };
    for (int i = 0; i < 15; ++i) {
        NebulaPuff p;
        p.x = (float)(rand() % WIN_W);
        p.y = (float)(rand() % WIN_H);
        p.radius = 40.0f + (rand() % 80);
        p.speed = 0.15f + (rand() % 100) / 400.0f;
        int ci = rand() % 5;
        p.r = colors[ci][0];
        p.g = colors[ci][1];
        p.b = colors[ci][2];
        p.alpha = 0.08f + (rand() % 100) / 800.0f;
        puffs.push_back(p);
    }
}

/*=============================================================================
 * NEBULA FIELD SCROLLING UPDATE (Two-Layer Parallax Animation)
 *
 * Updates both parallax layers at different speeds:
 *
 * Stars (foreground layer): Scroll faster, creating a sense of proximity.
 * Nebula puffs (background layer): Drift slowly, appearing distant.
 *
 * The speed ratio between layers determines the perceived depth.
 * Both layers wrap around when exiting the viewport.
 *=============================================================================*/
void NebulaField::update() {
    // Stars scroll faster (foreground parallax layer)
    for (auto& s : stars) {
        s.y -= s.speed;
        if (s.y < 0) {
            s.y = (float)WIN_H;
            s.x = (float)(rand() % WIN_W);
        }
    }
    // Puffs drift slowly (background parallax layer)
    for (auto& p : puffs) {
        p.y -= p.speed;
        if (p.y < -p.radius) {
            p.y = WIN_H + p.radius;
            p.x = (float)(rand() % WIN_W);
        }
    }
}

/*=============================================================================
 * NEBULA FIELD RENDERING (Multi-Layer Staging Visual)
 *
 * Renders the two-layer nebula background:
 *
 * 1. Nebula puffs (drawn first — behind stars):
 *    Each puff is rendered as a filled circle using GL_TRIANGLE_FAN.
 *    The center vertex has the puff color, edge vertices fade to
 *    transparent, creating a soft glow effect.
 *    Additive blending (GL_ONE) makes overlapping puffs glow brighter.
 *    Vertex positions use cos(theta)/sin(theta) rotation vectors.
 *
 * 2. Stars (drawn on top):
 *    Dim purple-white GL_POINTS rendered over the nebula puffs.
 *=============================================================================*/
void NebulaField::draw() const {
    // Draw nebula puffs as filled circles with additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive for glow
    for (const auto& p : puffs) {
        glBegin(GL_TRIANGLE_FAN);
            glColor4f(p.r, p.g, p.b, p.alpha);
            glVertex2f(p.x, p.y); // Center
            glColor4f(p.r, p.g, p.b, 0.0f); // Fade to transparent at edges
            int segments = 24;
            for (int i = 0; i <= segments; ++i) {
                float theta = i * 2.0f * 3.14159f / segments;
                glVertex2f(p.x + cos(theta) * p.radius, p.y + sin(theta) * p.radius);
            }
        glEnd();
    }
    glDisable(GL_BLEND);

    // Draw dim stars on top (foreground layer)
    glColor3f(0.6f, 0.6f, 0.8f);
    glPointSize(1.5f);
    glBegin(GL_POINTS);
    for (const Star& s : stars) {
        glVertex2f(s.x, s.y);
    }
    glEnd();
}

/*=============================================================================
 * ENEMY SPAWN TIMING
 *
 * Controls when and what type of enemies spawn. The spawn timing is
 * governed by the spawnInterval calculated in Game::update():
 *   interval = max(25, 75 - wave * 4) frames
 *
 * This creates escalating difficulty:
 *   Wave 1: 71 frames (~1.2 seconds) between spawns
 *   Wave 5: 55 frames (~0.9 seconds) between spawns
 *   Wave 10+: 25 frames (~0.4 seconds) between spawns
 *
 * Boss waves (every 5th wave) spawn a single powerful enemy.
 * Normal waves spawn a mix of enemy types based on probability.
 *=============================================================================*/
void Game::spawnEnemy() {
    Enemy e;
    e.alive = true;
    e.shootCooldown = rand() % 50 + 30;
    e.moveTimer = 0.0f;

    if (wave % 5 == 0) {
        // Boss wave — single powerful enemy
        if (level == 1) {
            e.enemyType = 3;
            e.w = 110.0f; e.h = 60.0f;
            e.x = WIN_W / 2.0f;
            e.y = WIN_H + 50.0f;
            e.speed = 0.8f;
            e.maxHp = 20 + wave * 5;
        } else {
            // Level 2 Boss: Nebula Overlord
            e.enemyType = 7;
            e.w = 130.0f; e.h = 70.0f;
            e.x = WIN_W / 2.0f;
            e.y = WIN_H + 60.0f;
            e.speed = 0.8f;
            e.maxHp = 60;
        }
        e.hp = e.maxHp;
    } else if (level == 2) {
        // Level 2 normal enemies — probability-based type selection
        int roll = rand() % 100;
        if (roll < 40) {
            e.enemyType = 4; // Phantom
            e.w = 34.0f; e.h = 34.0f;
            e.speed = 1.4f + (rand() % 100) / 100.0f + (wave - 5) * 0.1f;
            e.maxHp = 2;
        } else if (roll < 75) {
            e.enemyType = 5; // Sentinel
            e.w = 36.0f; e.h = 36.0f;
            e.speed = 1.0f + (rand() % 50) / 100.0f + (wave - 5) * 0.08f;
            e.maxHp = 3;
        } else {
            e.enemyType = 6; // Wraith
            e.w = 28.0f; e.h = 28.0f;
            e.speed = 2.5f + (rand() % 100) / 100.0f + (wave - 5) * 0.15f;
            e.maxHp = 1;
        }
        e.x = (float)(rand() % (WIN_W - 80) + 40);
        e.y = WIN_H + 20.0f;
        e.hp = e.maxHp;
    } else {
        // Level 1 normal enemies — probability-based type selection
        int roll = rand() % 100;
        if (roll < 45) {
            e.enemyType = 0;
            e.w = 32.0f; e.h = 28.0f;
            e.speed = 1.6f + (rand() % 100) / 100.0f + wave * 0.1f;
            e.maxHp = 1;
        } else if (roll < 80) {
            e.enemyType = 1;
            e.w = 36.0f; e.h = 32.0f;
            e.speed = 1.2f + (rand() % 50) / 100.0f + wave * 0.08f;
            e.maxHp = 2;
        } else {
            e.enemyType = 2;
            e.w = 28.0f; e.h = 28.0f;
            e.speed = 2.2f + (rand() % 100) / 100.0f + wave * 0.15f;
            e.maxHp = 1;
        }
        e.x = (float)(rand() % (WIN_W - 80) + 40);
        e.y = WIN_H + 20.0f;
        e.hp = e.maxHp;
    }
    enemies.push_back(e);
    enemiesSpawned++;
}

/*=============================================================================
 * SHIP SELECTION SCREEN (UI Staging)
 *
 * Demonstrates the staging animation principle by arranging three ship
 * cards horizontally. Each card acts as a visual stage presenting:
 *   - Ship preview (drawn using drawTriangle/drawRect)
 *   - Ship stats text
 *   - Selection button
 *
 * The spatial layout guides the player's eye from left to right,
 * comparing options before making a choice.
 *=============================================================================*/
void Game::drawShipSelect() {
    glColor3f(0.2f, 0.9f, 1.0f);
    drawText(WIN_W / 2 - 60, WIN_H - 100, "CHOOSE YOUR STARSHIP", GLUT_BITMAP_HELVETICA_12);

    std::string btns[3] = { "[1] INTERCEPTOR", "[2] AEGIS TANK", "[3] VANGUARD" };

    for (int i = 0; i < 3; ++i) {
        float cx = 180.0f + i * 220.0f;
        float cy = WIN_H / 2.0f + 50.0f;
        
        // Card background
        glColor3f(0.1f, 0.15f, 0.25f);
        drawRect(cx, cy, 180, 200);

        // Card outline
        glColor3f(0.2f, 0.8f, 1.0f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(cx - 90, cy - 100);
            glVertex2f(cx + 90, cy - 100);
            glVertex2f(cx + 90, cy + 100);
            glVertex2f(cx - 90, cy + 100);
        glEnd();

        // Ship preview and stats
        glColor3f(1.0f, 1.0f, 1.0f);
        if (i == 0) {
            drawText(cx - 35, cy + 80, "INTERCEPTOR", GLUT_BITMAP_HELVETICA_12);
            glColor3f(0.2f, 0.9f, 1.0f);
            drawTriangle(cx, cy + 10, 30, 25);
            glColor3f(1.0f, 1.0f, 1.0f);
            drawText(cx - 60, cy - 20, "Speed: Fast (6.5)", GLUT_BITMAP_HELVETICA_12);
            drawText(cx - 60, cy - 38, "Shields: Weak (2)", GLUT_BITMAP_HELVETICA_12);
            drawText(cx - 60, cy - 56, "Weapons: Focused", GLUT_BITMAP_HELVETICA_12);
        }
        else if (i == 1) {
            drawText(cx - 30, cy + 80, "AEGIS TANK", GLUT_BITMAP_HELVETICA_12);
            glColor3f(0.1f, 0.6f, 0.9f);
            drawRect(cx, cy + 10, 26, 26);
            glColor3f(1.0f, 1.0f, 1.0f);
            drawText(cx - 60, cy - 20, "Speed: Slow (4.5)", GLUT_BITMAP_HELVETICA_12);
            drawText(cx - 60, cy - 38, "Shields: Heavy (4)", GLUT_BITMAP_HELVETICA_12);
            drawText(cx - 60, cy - 56, "Weapons: Standard", GLUT_BITMAP_HELVETICA_12);
        }
        else {
            drawText(cx - 25, cy + 80, "VANGUARD", GLUT_BITMAP_HELVETICA_12);
            glColor3f(0.9f, 0.2f, 0.2f);
            drawTriangle(cx, cy + 10, 30, 25);
            glColor3f(0.5f, 0.1f, 0.1f);
            drawTriangle(cx - 12, cy + 5, 10, 18);
            drawTriangle(cx + 12, cy + 5, 10, 18);
            glColor3f(1.0f, 1.0f, 1.0f);
            drawText(cx - 60, cy - 20, "Speed: Med (5.8)", GLUT_BITMAP_HELVETICA_12);
            drawText(cx - 60, cy - 38, "Shields: Light (1)", GLUT_BITMAP_HELVETICA_12);
            drawText(cx - 60, cy - 56, "Weapons: Triple", GLUT_BITMAP_HELVETICA_12);
        }

        // Selection button
        float bx = cx;
        float by = WIN_H / 2.0f - 100.0f;
        float bw = 150.0f;
        float bh = 40.0f;

        glColor3f(0.08f, 0.15f, 0.3f);
        drawRect(bx, by, bw, bh);

        glColor3f(0.2f, 0.9f, 1.0f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(bx - bw/2, by - bh/2);
            glVertex2f(bx + bw/2, by - bh/2);
            glVertex2f(bx + bw/2, by + bh/2);
            glVertex2f(bx - bw/2, by + bh/2);
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(bx - btns[i].length() * 3.5f, by - 6, btns[i], GLUT_BITMAP_HELVETICA_12);
    }

    glColor3f(0.5f, 0.5f, 0.5f);
    drawText(WIN_W / 2 - 55, 50, "[B] BACK TO MAIN MENU", GLUT_BITMAP_HELVETICA_12);
}

/*=============================================================================
 * UPGRADE SHOP SCREEN (UI Staging)
 *
 * Arranges the upgrade shop as a two-panel layout:
 *   Left panel: Current ship stats display
 *   Right panel: Purchasable upgrades with availability indicators
 *
 * This staging guides the player to compare their current stats
 * (left) with available upgrades (right), creating a clear
 * information flow for decision-making.
 *=============================================================================*/
void Game::drawUpgradeShop() {
    glColor3f(1.0f, 0.85f, 0.0f);
    drawText(WIN_W / 2 - 65, WIN_H - 80, "ROGUE UPGRADE STATION", GLUT_BITMAP_HELVETICA_12);

    std::stringstream ss;
    ss << "AVAILABLE SCRAP: " << credits << " CREDITS";
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(WIN_W / 2 - 65, WIN_H - 110, ss.str(), GLUT_BITMAP_HELVETICA_12);

    // Left panel: current stats
    glColor3f(0.1f, 0.15f, 0.2f);
    drawRect(150, WIN_H / 2, 220, 240);
    glColor3f(0.2f, 0.8f, 1.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(40, WIN_H/2 - 120);
        glVertex2f(260, WIN_H/2 - 120);
        glVertex2f(260, WIN_H/2 + 120);
        glVertex2f(40, WIN_H/2 + 120);
    glEnd();
    
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(70, WIN_H/2 + 90, "CURRENT SHIP STATS:", GLUT_BITMAP_HELVETICA_12);
    std::stringstream sh;
    sh << "- Weapon Level: Lvl " << player.weaponLevel;
    drawText(60, WIN_H/2 + 55, sh.str(), GLUT_BITMAP_HELVETICA_12);
    std::stringstream hl;
    hl << "- Hull Points: " << player.hull << " / " << player.maxHull;
    drawText(60, WIN_H/2 + 25, hl.str(), GLUT_BITMAP_HELVETICA_12);
    std::stringstream sl;
    sl << "- Shield Level: Lvl " << player.maxShields;
    drawText(60, WIN_H/2 - 5, sl.str(), GLUT_BITMAP_HELVETICA_12);
    std::stringstream bs;
    bs << "- Base Shields: " << baseShields << "%";
    drawText(60, WIN_H/2 - 35, bs.str(), GLUT_BITMAP_HELVETICA_12);

    // Right panel: upgrade options
    bool canBuyWeapon = (credits >= 100 && player.weaponLevel < 3);
    bool canRepairHull = (credits >= 30 && player.hull < player.maxHull);
    bool canBuyShield = (credits >= 50);
    bool available[4] = { canBuyWeapon, canRepairHull, canBuyShield, true };

    std::string reasonTexts[4] = { "", "", "", "" };
    if (!canBuyWeapon) {
        if (player.weaponLevel >= 3) reasonTexts[0] = "[MAX LEVEL]";
        else reasonTexts[0] = "[NOT ENOUGH SCRAP]";
    }
    if (!canRepairHull) {
        if (player.hull >= player.maxHull) reasonTexts[1] = "[HULL FULL]";
        else reasonTexts[1] = "[NOT ENOUGH SCRAP]";
    }
    if (!canBuyShield) {
        reasonTexts[2] = "[NOT ENOUGH SCRAP]";
    }

    std::string shopTexts[4] = {
        "[1] UPGRADE WEAPONS (100 SCRAP)",
        "[2] REPAIR HULL (30 SCRAP)",
        "[3] UPGRADE SHIELDS (50 SCRAP)",
        "[ENTER] LAUNCH NEXT WAVE"
    };
    float ys[4] = {
        WIN_H / 2.0f + 60.0f,
        WIN_H / 2.0f,
        WIN_H / 2.0f - 60.0f,
        WIN_H / 2.0f - 140.0f
    };

    for (int i = 0; i < 4; ++i) {
        float bx = WIN_W / 2.0f + 70.0f;
        float by = ys[i];
        float bw = 350.0f;
        float bh = 45.0f;

        if (available[i]) {
            glColor3f(0.08f, 0.15f, 0.3f);
        } else {
            glColor3f(0.06f, 0.06f, 0.08f);
        }
        drawRect(bx, by, bw, bh);

        if (available[i]) {
            if (i == 3) {
                glColor3f(0.0f, 1.0f, 0.4f);
            } else {
                glColor3f(1.0f, 0.85f, 0.0f);
            }
        } else {
            glColor3f(0.3f, 0.3f, 0.3f);
        }
        glBegin(GL_LINE_LOOP);
            glVertex2f(bx - bw/2, by - bh/2);
            glVertex2f(bx + bw/2, by - bh/2);
            glVertex2f(bx + bw/2, by + bh/2);
            glVertex2f(bx - bw/2, by + bh/2);
        glEnd();

        if (available[i]) {
            glColor3f(1.0f, 1.0f, 1.0f);
        } else {
            glColor3f(0.4f, 0.4f, 0.4f);
        }
        // Center text in button: Helvetica 12 chars are ~7px wide
        drawText(bx - shopTexts[i].length() * 3.5f, by + 2, shopTexts[i], GLUT_BITMAP_HELVETICA_12);

        // Show reason text BELOW the main button text
        if (!available[i] && reasonTexts[i].length() > 0) {
            glColor3f(1.0f, 0.3f, 0.3f);
            drawText(bx - reasonTexts[i].length() * 3.5f, by - 12, reasonTexts[i], GLUT_BITMAP_HELVETICA_12);
        }
    }
}

/*=============================================================================
 * LEVEL TRANSITION SCREEN (Staging)
 *
 * Presents the Level 2 transition as a dramatic reveal stage:
 *   - Big title text ("LEVEL 2 - NEBULA ZONE")
 *   - Decorative divider lines
 *   - Carry-over stats summary
 *   - Action prompt
 *
 * The vertical layout creates a top-to-bottom information flow,
 * building anticipation before the player enters the new level.
 *=============================================================================*/
void Game::drawLevelTransition() {
    // Big title
    glColor3f(0.6f, 0.1f, 0.8f);
    drawText(WIN_W / 2 - 22, WIN_H / 2 + 160, "LEVEL 2", GLUT_BITMAP_HELVETICA_12);

    glColor3f(0.8f, 0.3f, 1.0f);
    drawText(WIN_W / 2 - 35, WIN_H / 2 + 130, "NEBULA ZONE", GLUT_BITMAP_HELVETICA_12);

    // Decorative line
    glLineWidth(2.0f);
    glColor3f(0.6f, 0.1f, 0.8f);
    glBegin(GL_LINES);
        glVertex2f(WIN_W / 2 - 150, WIN_H / 2 + 110);
        glVertex2f(WIN_W / 2 + 150, WIN_H / 2 + 110);
    glEnd();

    // Info
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(WIN_W / 2 - 75, WIN_H / 2 + 70, "New enemies. New environment.", GLUT_BITMAP_HELVETICA_12);
    drawText(WIN_W / 2 - 60, WIN_H / 2 + 40, "Your upgrades carry over.", GLUT_BITMAP_HELVETICA_12);

    // Stats carry-over summary
    glColor3f(0.2f, 0.9f, 1.0f);
    std::stringstream hp;
    hp << "Hull: " << player.hull << "/" << player.maxHull << "  Shields: " << player.shields << "/" << player.maxShields;
    drawText(WIN_W / 2 - 85, WIN_H / 2, hp.str(), GLUT_BITMAP_HELVETICA_12);

    std::stringstream wp;
    wp << "Weapon Level: " << player.weaponLevel << "  Scrap: " << credits;
    drawText(WIN_W / 2 - 70, WIN_H / 2 - 25, wp.str(), GLUT_BITMAP_HELVETICA_12);

    // Decorative line
    glLineWidth(2.0f);
    glColor3f(0.6f, 0.1f, 0.8f);
    glBegin(GL_LINES);
        glVertex2f(WIN_W / 2 - 150, WIN_H / 2 - 50);
        glVertex2f(WIN_W / 2 + 150, WIN_H / 2 - 50);
    glEnd();

    // Prompt
    glColor3f(1.0f, 0.85f, 0.0f);
    drawText(WIN_W / 2 - 65, WIN_H / 2 - 90, "Press ENTER to begin Level 2", GLUT_BITMAP_HELVETICA_12);
}
