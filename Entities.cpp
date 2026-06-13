#include "Entities.h"
#include <GL/freeglut.h>
#include <cstdlib>
#include <cmath>

void drawRect(float cx, float cy, float w, float h) {
    glBegin(GL_QUADS);
        glVertex2f(cx - w/2, cy - h/2);
        glVertex2f(cx + w/2, cy - h/2);
        glVertex2f(cx + w/2, cy + h/2);
        glVertex2f(cx - w/2, cy + h/2);
    glEnd();
}

void drawTriangle(float cx, float cy, float w, float h) {
    glBegin(GL_TRIANGLES);
        glVertex2f(cx,         cy + h/2);
        glVertex2f(cx - w/2,   cy - h/2);
        glVertex2f(cx + w/2,   cy - h/2);
    glEnd();
}

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
        if (ix0 == ix1 && iy0 == iy1) break;
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

void drawMidpointCircle(float cx, float cy, float radius) {
    int x = 0;
    int y = (int)std::round(radius);
    int d = 1 - y;

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

bool aabb(float ax, float ay, float aw, float ah,
          float bx, float by, float bw, float bh) {
    return ax - aw/2 < bx + bw/2 &&
           ax + aw/2 > bx - bw/2 &&
           ay - ah/2 < by + bh/2 &&
           ay + ah/2 > by - bh/2;
}

void Particle::update() {
    x += vx;
    y += vy;
    lifetime -= 0.025f;
}

void Particle::draw() const {
    if (lifetime <= 0.0f) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, lifetime);
    float sz = 5.0f * lifetime + 1.0f;
    drawRect(x, y, sz, sz);
    glDisable(GL_BLEND);
}

void StarField::init() {
    stars.clear();
    for (int i = 0; i < 80; ++i) {
        Star s;
        s.x = (float)(rand() % WIN_W);
        s.y = (float)(rand() % WIN_H);
        s.speed = 0.5f + (rand() % 100) / 100.0f * 2.0f;
        stars.push_back(s);
    }
}

void StarField::update() {
    for (auto& s : stars) {
        s.y -= s.speed;
        if (s.y < 0) {
            s.y = (float)WIN_H;
            s.x = (float)(rand() % WIN_W);
        }
    }
}

void StarField::draw() const {
    glColor3f(1.0f, 1.0f, 1.0f);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (const Star& s : stars) {
        glVertex2f(s.x, s.y);
    }
    glEnd();
}

// ---- Nebula Field (Level 2 Background) ----

void NebulaField::init() {
    puffs.clear();
    stars.clear();
    // Create dim background stars
    for (int i = 0; i < 40; ++i) {
        Star s;
        s.x = (float)(rand() % WIN_W);
        s.y = (float)(rand() % WIN_H);
        s.speed = 0.3f + (rand() % 100) / 100.0f * 1.0f;
        stars.push_back(s);
    }
    // Create nebula puffs with purple/magenta/teal colors
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

void NebulaField::draw() const {
    // Draw nebula puffs as filled circles with additive blending (2D Filling)
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

    // Draw dim stars on top (2D Drawing)
    glColor3f(0.6f, 0.6f, 0.8f);
    glPointSize(1.5f);
    glBegin(GL_POINTS);
    for (const Star& s : stars) {
        glVertex2f(s.x, s.y);
    }
    glEnd();
}

void Bullet::update() {
    x += vx;
    y += vy;
    if (y > WIN_H || x < 0 || x > WIN_W) {
        alive = false;
    }
}

void Bullet::draw() const {
    glColor3f(0.2f, 1.0f, 1.0f);
    glPointSize(2.0f);
    drawBresenhamLine(x, y - 6.0f, x, y + 6.0f);
    glPointSize(1.0f);
}

void EnemyBullet::update() {
    y -= vy;
    if (y < -10 || y > WIN_H + 10) {
        alive = false;
    }
}

void EnemyBullet::draw() const {
    glColor3f(1.0f, 0.4f, 0.1f);
    glPointSize(2.0f);
    drawBresenhamLine(x, y - 6.0f, x, y + 6.0f);
    glPointSize(1.0f);
}

void Scrap::update() {
    y -= speed;
    angle += 4.0f;
    if (angle >= 360.0f) angle -= 360.0f;
    if (y < -10) {
        alive = false;
    }
}

void Scrap::draw() const {
    // Golden glow circle behind the scrap
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(1.0f, 0.7f, 0.0f, 0.15f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= 16; ++i) {
        float theta = i * 2.0f * 3.14159f / 16.0f;
        glVertex2f(x + cos(theta) * 12.0f, y + sin(theta) * 12.0f);
    }
    glEnd();
    glDisable(GL_BLEND);

    // Rotating diamond
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glRotatef(angle, 0.0f, 0.0f, 1.0f);
    glColor3f(1.0f, 0.85f, 0.0f);
    glBegin(GL_TRIANGLES);
        glVertex2f(0, 5);
        glVertex2f(-5, 0);
        glVertex2f(5, 0);
        
        glVertex2f(0, -5);
        glVertex2f(-5, 0);
        glVertex2f(5, 0);
    glEnd();
    // Inner bright core
    glColor3f(1.0f, 1.0f, 0.6f);
    glBegin(GL_TRIANGLES);
        glVertex2f(0, 3);
        glVertex2f(-3, 0);
        glVertex2f(3, 0);
        
        glVertex2f(0, -3);
        glVertex2f(-3, 0);
        glVertex2f(3, 0);
    glEnd();
    glPopMatrix();
}

void Enemy::update(float playerX) {
    if (enemyType == 0) {
        y -= speed;
    }
    else if (enemyType == 1) {
        if (y > WIN_H - 160) {
            y -= speed;
        } else {
            x += sin(y * 0.05f) * 1.8f;
        }
    }
    else if (enemyType == 2) {
        y -= speed;
        if (x < playerX) x += 1.0f;
        if (x > playerX) x -= 1.0f;
    }
    else if (enemyType == 3) {
        if (y > WIN_H - 150) {
            y -= speed * 0.5f;
        } else {
            // Use a time-based counter for smooth oscillation instead of sin(y)
            moveTimer += 0.02f;
            x += sin(moveTimer) * 2.5f;
            // Keep the boss within screen bounds
            if (x < w/2 + 20) x = w/2 + 20;
            if (x > WIN_W - w/2 - 20) x = WIN_W - w/2 - 20;
        }
    }
    // --- Level 2 Enemies ---
    // Type 4: Phantom — Zigzag descent
    else if (enemyType == 4) {
        y -= speed;
        moveTimer += 0.05f;
        x += sin(moveTimer) * 3.5f;
    }
    // Type 5: Sentinel — Descends then orbits in place
    else if (enemyType == 5) {
        if (y > WIN_H - 200) {
            y -= speed;
        } else {
            moveTimer += 0.03f;
            x += cos(moveTimer) * 2.0f;
            y += sin(moveTimer) * 1.0f;
        }
    }
    // Type 6: Wraith — Fast tracker that fades in/out
    else if (enemyType == 6) {
        y -= speed * 0.6f;
        if (x < playerX) x += 1.8f;
        if (x > playerX) x -= 1.8f;
        moveTimer += 0.03f; // Used for alpha fade in draw()
    }
    // Type 7: Nebula Overlord Boss — 3-phase movement
    else if (enemyType == 7) {
        if (y > WIN_H - 150) {
            y -= speed * 0.4f;
        } else {
            moveTimer += 0.02f;
            float hpPct = (float)hp / maxHp;
            if (hpPct > 0.6f) {
                // Phase 1: Slow oscillation
                x += sin(moveTimer) * 2.0f;
            } else if (hpPct > 0.3f) {
                // Phase 2: Faster oscillation + vertical bobbing
                x += sin(moveTimer * 1.5f) * 3.5f;
                y += cos(moveTimer * 2.0f) * 1.2f;
            } else {
                // Phase 3: Erratic random movement
                x += sin(moveTimer * 2.0f) * 4.0f + (rand() % 5 - 2) * 0.5f;
                y += cos(moveTimer * 1.8f) * 1.5f;
            }
            if (x < w/2 + 20) x = w/2 + 20;
            if (x > WIN_W - w/2 - 20) x = WIN_W - w/2 - 20;
            if (y < WIN_H - 250) y = WIN_H - 250;
            if (y > WIN_H - 80) y = WIN_H - 80;
        }
    }
}

void Enemy::draw() const {
    // --- Engine thruster flame (drawn behind the enemy, pointing upward since enemies move down) ---
    float elapsed = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
    float flicker = 1.0f + 0.25f * sin(elapsed * 15.0f + x * 0.1f);

    if (enemyType == 0) {
        // Small red thruster on top
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        float fh = 10.0f * flicker;
        glColor4f(1.0f, 0.4f, 0.0f, 0.7f);
        glBegin(GL_TRIANGLES);
            glVertex2f(x - 5.0f, y + h/2);
            glVertex2f(x + 5.0f, y + h/2);
            glVertex2f(x, y + h/2 + fh);
        glEnd();
        glColor4f(1.0f, 0.8f, 0.2f, 0.5f);
        glBegin(GL_TRIANGLES);
            glVertex2f(x - 3.0f, y + h/2);
            glVertex2f(x + 3.0f, y + h/2);
            glVertex2f(x, y + h/2 + fh * 0.6f);
        glEnd();
        glDisable(GL_BLEND);

        glColor3f(1.0f, 0.3f, 0.3f);
        drawRect(x, y, w, h);
        glColor3f(0.6f, 0.0f, 0.0f);
        drawRect(x, y - 8, w - 8, 6);
        glColor3f(1.0f, 1.0f, 0.0f);
        drawRect(x, y, 6, 6);
    }
    else if (enemyType == 1) {
        // Purple thruster
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        float fh = 12.0f * flicker;
        glColor4f(0.8f, 0.2f, 1.0f, 0.6f);
        glBegin(GL_TRIANGLES);
            glVertex2f(x - 4.0f, y + h/2);
            glVertex2f(x + 4.0f, y + h/2);
            glVertex2f(x, y + h/2 + fh);
        glEnd();
        glDisable(GL_BLEND);

        glColor3f(0.7f, 0.1f, 0.8f);
        drawTriangle(x, y, w, h);
        glColor3f(0.2f, 0.9f, 1.0f);
        drawRect(x, y - 2, 8, 8);
    }
    else if (enemyType == 2) {
        // Orange thruster
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        float fh = 10.0f * flicker;
        glColor4f(1.0f, 0.6f, 0.0f, 0.7f);
        glBegin(GL_TRIANGLES);
            glVertex2f(x - 4.0f, y + h/2);
            glVertex2f(x + 4.0f, y + h/2);
            glVertex2f(x, y + h/2 + fh);
        glEnd();
        glDisable(GL_BLEND);

        glColor3f(1.0f, 0.5f, 0.0f);
        drawTriangle(x, y, w, h);
        glColor3f(1.0f, 1.0f, 0.0f);
        drawRect(x, y + 10, 4, 8);
    }
    else if (enemyType == 3) {
        // Boss: dual large thrusters
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        float fh = 18.0f * flicker;
        float flicker2 = 1.0f + 0.2f * sin(elapsed * 18.0f + 2.0f);
        float fh2 = 18.0f * flicker2;
        // Left thruster
        glColor4f(0.3f, 0.5f, 1.0f, 0.6f);
        glBegin(GL_TRIANGLES);
            glVertex2f(x - w/2 - 10 - 6, y + h/2 - 5);
            glVertex2f(x - w/2 - 10 + 6, y + h/2 - 5);
            glVertex2f(x - w/2 - 10, y + h/2 - 5 + fh);
        glEnd();
        // Right thruster
        glBegin(GL_TRIANGLES);
            glVertex2f(x + w/2 + 10 - 6, y + h/2 - 5);
            glVertex2f(x + w/2 + 10 + 6, y + h/2 - 5);
            glVertex2f(x + w/2 + 10, y + h/2 - 5 + fh2);
        glEnd();
        glDisable(GL_BLEND);

        glColor3f(0.4f, 0.4f, 0.45f);
        drawRect(x, y, w, h);
        glColor3f(0.25f, 0.25f, 0.3f);
        drawRect(x - w/2 - 10, y, 20, h - 10);
        drawRect(x + w/2 + 10, y, 20, h - 10);
        
        float pct = (float)hp / maxHp;
        glColor3f(1.0f, 0.0f, 0.0f);
        drawRect(x, y + h/2 + 15, w * pct, 6);
    }
    // --- Level 2 Enemies ---
    // Type 4: Phantom — Green/Teal Diamond
    else if (enemyType == 4) {
        // Teal thruster
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        float fh = 10.0f * flicker;
        glColor4f(0.0f, 1.0f, 0.6f, 0.6f);
        glBegin(GL_TRIANGLES);
            glVertex2f(x - 4.0f, y + h/2);
            glVertex2f(x + 4.0f, y + h/2);
            glVertex2f(x, y + h/2 + fh);
        glEnd();
        glDisable(GL_BLEND);

        // Diamond shape (rotated square) using GL_POLYGON (2D Filling)
        glBegin(GL_POLYGON);
            glColor3f(0.0f, 0.8f, 0.6f); // Teal top
            glVertex2f(x, y + h/2);
            glColor3f(0.1f, 0.5f, 0.4f); // Darker sides
            glVertex2f(x + w/2, y);
            glColor3f(0.0f, 0.7f, 0.5f);
            glVertex2f(x, y - h/2);
            glVertex2f(x - w/2, y);
        glEnd();
        // Inner diamond outline (2D Drawing)
        glColor3f(0.2f, 1.0f, 0.8f);
        glLineWidth(1.5f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(x, y + h/4);
            glVertex2f(x + w/4, y);
            glVertex2f(x, y - h/4);
            glVertex2f(x - w/4, y);
        glEnd();
    }
    // Type 5: Sentinel — Magenta/Pink Hexagon
    else if (enemyType == 5) {
        // Magenta thruster
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        float fh = 12.0f * flicker;
        glColor4f(0.8f, 0.2f, 1.0f, 0.6f);
        glBegin(GL_TRIANGLES);
            glVertex2f(x - 5.0f, y + h/2);
            glVertex2f(x + 5.0f, y + h/2);
            glVertex2f(x, y + h/2 + fh);
        glEnd();
        glDisable(GL_BLEND);

        // Hexagon body (2D Filling)
        float r = w / 2.0f;
        glBegin(GL_POLYGON);
        for (int i = 0; i < 6; ++i) {
            float theta = i * 2.0f * 3.14159f / 6.0f - 3.14159f / 6.0f;
            if (i < 3) glColor3f(0.8f, 0.15f, 0.6f); // Hot pink top
            else glColor3f(0.5f, 0.1f, 0.4f); // Darker bottom
            glVertex2f(x + cos(theta) * r, y + sin(theta) * r);
        }
        glEnd();
        // Hexagon outline (2D Drawing)
        glColor3f(1.0f, 0.4f, 0.8f);
        glLineWidth(1.5f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 6; ++i) {
            float theta = i * 2.0f * 3.14159f / 6.0f - 3.14159f / 6.0f;
            glVertex2f(x + cos(theta) * r, y + sin(theta) * r);
        }
        glEnd();
        // Center dot
        glColor3f(1.0f, 1.0f, 1.0f);
        drawRect(x, y, 4, 4);
    }
    // Type 6: Wraith — Fading dark purple pentagon
    else if (enemyType == 6) {
        float fadeAlpha = 0.3f + 0.7f * (0.5f + 0.5f * sin(moveTimer * 2.0f)); // Pulse in/out
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Pentagon body (2D Filling)
        float r = w / 2.0f;
        glBegin(GL_POLYGON);
        for (int i = 0; i < 5; ++i) {
            float theta = i * 2.0f * 3.14159f / 5.0f + 3.14159f / 2.0f;
            glColor4f(0.4f, 0.0f, 0.5f, fadeAlpha);
            glVertex2f(x + cos(theta) * r, y + sin(theta) * r);
        }
        glEnd();
        // Inner star pattern (2D Drawing)
        glColor4f(0.8f, 0.3f, 1.0f, fadeAlpha);
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        for (int i = 0; i < 5; ++i) {
            float theta1 = i * 2.0f * 3.14159f / 5.0f + 3.14159f / 2.0f;
            float theta2 = ((i + 2) % 5) * 2.0f * 3.14159f / 5.0f + 3.14159f / 2.0f;
            glVertex2f(x + cos(theta1) * r * 0.8f, y + sin(theta1) * r * 0.8f);
            glVertex2f(x + cos(theta2) * r * 0.8f, y + sin(theta2) * r * 0.8f);
        }
        glEnd();
        glDisable(GL_BLEND);
    }
    // Type 7: Nebula Overlord Boss
    else if (enemyType == 7) {
        float hpPct = (float)hp / maxHp;

        // Dual thrusters
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        float fh2L = 20.0f * flicker;
        float flicker2 = 1.0f + 0.2f * sin(elapsed * 18.0f + 2.0f);
        float fh2R = 20.0f * flicker2;
        // Left thruster — purple
        glColor4f(0.6f, 0.1f, 0.8f, 0.7f);
        glBegin(GL_TRIANGLES);
            glVertex2f(x - 25, y + h/2);
            glVertex2f(x - 15, y + h/2);
            glVertex2f(x - 20, y + h/2 + fh2L);
        glEnd();
        // Right thruster
        glBegin(GL_TRIANGLES);
            glVertex2f(x + 15, y + h/2);
            glVertex2f(x + 25, y + h/2);
            glVertex2f(x + 20, y + h/2 + fh2R);
        glEnd();
        glDisable(GL_BLEND);

        // Hexagonal body with phase-based color gradient (2D Filling)
        float bodyR = w / 2.0f;
        float bodyR2, bodyG2, bodyB2;
        if (hpPct > 0.6f) { bodyR2 = 0.3f; bodyG2 = 0.1f; bodyB2 = 0.5f; }      // Purple
        else if (hpPct > 0.3f) { bodyR2 = 0.6f; bodyG2 = 0.1f; bodyB2 = 0.5f; }  // Magenta
        else { bodyR2 = 0.8f; bodyG2 = 0.1f; bodyB2 = 0.2f; }                     // Red

        glBegin(GL_POLYGON);
        for (int i = 0; i < 6; ++i) {
            float theta = i * 2.0f * 3.14159f / 6.0f;
            if (i < 3) glColor3f(bodyR2 + 0.1f, bodyG2 + 0.1f, bodyB2 + 0.1f);
            else glColor3f(bodyR2 - 0.1f, bodyG2, bodyB2 - 0.1f);
            glVertex2f(x + cos(theta) * bodyR, y + sin(theta) * bodyR);
        }
        glEnd();

        // Wing panels
        glColor3f(bodyR2 - 0.15f, bodyG2, bodyB2 - 0.1f);
        drawRect(x - w/2 - 15, y, 25, h - 15);
        drawRect(x + w/2 + 15, y, 25, h - 15);

        // Rotating turrets using glPushMatrix/glRotatef (2D Transformations)
        float turretAngle = elapsed * 60.0f; // Spin over time
        // Left turret
        glPushMatrix();
            glTranslatef(x - w/2 - 15, y, 0.0f);
            glRotatef(turretAngle, 0.0f, 0.0f, 1.0f);
            glColor3f(1.0f, 0.4f, 0.8f);
            glBegin(GL_TRIANGLES);
                glVertex2f(0, 8);
                glVertex2f(-5, -5);
                glVertex2f(5, -5);
            glEnd();
        glPopMatrix();
        // Right turret
        glPushMatrix();
            glTranslatef(x + w/2 + 15, y, 0.0f);
            glRotatef(-turretAngle, 0.0f, 0.0f, 1.0f);
            glColor3f(1.0f, 0.4f, 0.8f);
            glBegin(GL_TRIANGLES);
                glVertex2f(0, 8);
                glVertex2f(-5, -5);
                glVertex2f(5, -5);
            glEnd();
        glPopMatrix();

        // Pulsing shield ring when hp > 30% (2D Drawing)
        if (hpPct > 0.3f) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            float pulseAlpha = 0.2f + 0.2f * sin(elapsed * 3.0f);
            glColor4f(0.8f, 0.2f, 1.0f, pulseAlpha);
            glLineWidth(2.0f);
            glBegin(GL_LINE_LOOP);
            for (int i = 0; i < 24; ++i) {
                float theta = i * 2.0f * 3.14159f / 24.0f;
                glVertex2f(x + cos(theta) * (bodyR + 20), y + sin(theta) * (bodyR + 20));
            }
            glEnd();
            glDisable(GL_BLEND);
        }

        // HP bar
        glColor3f(0.3f, 0.3f, 0.3f);
        drawRect(x, y + h/2 + 18, w + 20, 8);
        if (hpPct > 0.6f) glColor3f(0.5f, 0.1f, 0.8f);
        else if (hpPct > 0.3f) glColor3f(0.8f, 0.4f, 0.1f);
        else glColor3f(1.0f, 0.1f, 0.1f);
        drawRect(x - (w + 20)/2 + (w + 20) * hpPct / 2, y + h/2 + 18, (w + 20) * hpPct, 6);
    }

    // Draw a small health bar for standard multi-HP enemies that have taken damage
    if (enemyType != 3 && enemyType != 7 && maxHp > 1 && hp < maxHp) {
        float barW = w;
        float barH = 4.0f;
        float barY = y + h/2 + 8.0f;
        
        // Background (gray)
        glColor3f(0.2f, 0.2f, 0.2f);
        drawRect(x, barY, barW, barH);
        
        // Foreground (green if > 50%, red otherwise)
        float pct = (float)hp / maxHp;
        if (pct > 0.5f) {
            glColor3f(0.2f, 0.9f, 0.2f); // Green
        } else {
            glColor3f(1.0f, 0.3f, 0.3f); // Red
        }
        drawRect(x - barW/2 + (barW * pct)/2, barY, barW * pct, barH);
    }
}

void Player::update(bool left, bool right, bool up, bool down) {
    if (left)  x -= speed;
    if (right) x += speed;
    if (up)    y += speed;
    if (down)  y -= speed;

    if (x < w/2) x = w/2;
    if (x > WIN_W - w/2) x = WIN_W - w/2;
    if (y < h/2) y = h/2;
    if (y > WIN_H - h/2) y = WIN_H - h/2;

    if (hitFlashTimer > 0) {
        hitFlashTimer -= 0.016f;
    }
}

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
            glVertex2f(x - 6.0f, y - h/2);
            glVertex2f(x + 6.0f, y - h/2);
            glVertex2f(x, y - h/2 - fh);
        glEnd();
        // Hot inner core
        glColor4f(0.8f, 0.95f, 1.0f, 0.6f);
        glBegin(GL_TRIANGLES);
            glVertex2f(x - 3.0f, y - h/2);
            glVertex2f(x + 3.0f, y - h/2);
            glVertex2f(x, y - h/2 - fh * 0.55f);
        glEnd();
    }
    else if (shipType == 1) {
        // Aegis Tank: dual wide thrusters
        float fh = 14.0f * flicker;
        float fh2v = 14.0f * flicker2;
        // Left thruster
        glColor4f(0.1f, 0.5f, 1.0f, 0.65f);
        glBegin(GL_TRIANGLES);
            glVertex2f(x - 14 - 4, y - h/2);
            glVertex2f(x - 14 + 4, y - h/2);
            glVertex2f(x - 14, y - h/2 - fh);
        glEnd();
        // Right thruster
        glBegin(GL_TRIANGLES);
            glVertex2f(x + 14 - 4, y - h/2);
            glVertex2f(x + 14 + 4, y - h/2);
            glVertex2f(x + 14, y - h/2 - fh2v);
        glEnd();
        // Inner cores
        glColor4f(0.7f, 0.9f, 1.0f, 0.5f);
        glBegin(GL_TRIANGLES);
            glVertex2f(x - 14 - 2, y - h/2);
            glVertex2f(x - 14 + 2, y - h/2);
            glVertex2f(x - 14, y - h/2 - fh * 0.5f);
        glEnd();
        glBegin(GL_TRIANGLES);
            glVertex2f(x + 14 - 2, y - h/2);
            glVertex2f(x + 14 + 2, y - h/2);
            glVertex2f(x + 14, y - h/2 - fh2v * 0.5f);
        glEnd();
    }
    else {
        // Vanguard: triple thrusters (red-orange)
        float fh = 14.0f * flicker;
        float fhL = 10.0f * flicker2;
        float fhR = 10.0f * (1.0f + 0.2f * sin(elapsed * 24.0f + 3.0f));
        // Center
        glColor4f(1.0f, 0.4f, 0.1f, 0.7f);
        glBegin(GL_TRIANGLES);
            glVertex2f(x - 5.0f, y - h/2);
            glVertex2f(x + 5.0f, y - h/2);
            glVertex2f(x, y - h/2 - fh);
        glEnd();
        // Left wing thruster
        glColor4f(1.0f, 0.5f, 0.0f, 0.55f);
        glBegin(GL_TRIANGLES);
            glVertex2f(x - 15 - 3, y - h/2 + 4);
            glVertex2f(x - 15 + 3, y - h/2 + 4);
            glVertex2f(x - 15, y - h/2 + 4 - fhL);
        glEnd();
        // Right wing thruster
        glBegin(GL_TRIANGLES);
            glVertex2f(x + 15 - 3, y - h/2 + 4);
            glVertex2f(x + 15 + 3, y - h/2 + 4);
            glVertex2f(x + 15, y - h/2 + 4 - fhR);
        glEnd();
        // Center inner core
        glColor4f(1.0f, 0.9f, 0.5f, 0.5f);
        glBegin(GL_TRIANGLES);
            glVertex2f(x - 2.5f, y - h/2);
            glVertex2f(x + 2.5f, y - h/2);
            glVertex2f(x, y - h/2 - fh * 0.5f);
        glEnd();
    }
    glDisable(GL_BLEND);

    // --- Ship body ---
    if (hitFlashTimer > 0) {
        glColor3f(1.0f, 0.0f, 0.0f);
    } else {
        if (shipType == 0) glColor3f(0.2f, 0.9f, 1.0f);
        else if (shipType == 1) glColor3f(0.1f, 0.6f, 0.9f);
        else glColor3f(0.9f, 0.2f, 0.2f);
    }

    if (shipType == 0) {
        drawTriangle(x, y, w, h);
        glColor3f(0.1f, 0.5f, 0.9f);
        drawRect(x - 18, y - 8, 10, 14);
        drawRect(x + 18, y - 8, 10, 14);
    }
    else if (shipType == 1) {
        drawRect(x, y, w, h);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawRect(x - 14, y + 2, 6, h);
        drawRect(x + 14, y + 2, 6, h);
    }
    else {
        drawTriangle(x, y, w, h);
        glColor3f(0.5f, 0.1f, 0.1f);
        drawTriangle(x - 15, y - 4, 12, h - 6);
        drawTriangle(x + 15, y - 4, 12, h - 6);
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    drawRect(x, y + 2, 6, 6);

    if (shields > 0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if (hitFlashTimer > 0) {
            glColor4f(0.8f, 1.0f, 1.0f, 0.95f); // Flashing bright cyan-white
        } else {
            glColor4f(0.0f, 0.8f, 1.0f, 0.4f);  // Regular blue shield
        }
        glPointSize(2.0f);
        drawMidpointCircle(x, y, w + 6.0f);
        glDisable(GL_BLEND);
    }
}

void FloatingText::update() {
    x += vx;
    y += vy;
    alpha -= 0.02f; // Fade out slowly
    if (alpha <= 0.0f) {
        alive = false;
    }
}

void FloatingText::draw() const {
    if (!alive || alpha <= 0.0f) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, alpha);
    
    // Draw the text
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
    glDisable(GL_BLEND);
}
