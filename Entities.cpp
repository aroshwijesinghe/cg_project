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

void Bullet::update() {
    x += vx;
    y += vy;
    if (y > WIN_H || x < 0 || x > WIN_W) {
        alive = false;
    }
}

void Bullet::draw() const {
    glColor3f(0.2f, 1.0f, 1.0f);
    drawRect(x, y, 4, 12);
}

void EnemyBullet::update() {
    y -= vy;
    if (y < -10) {
        alive = false;
    }
}

void EnemyBullet::draw() const {
    glColor3f(1.0f, 0.4f, 0.1f);
    drawRect(x, y, 5, 12);
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

    // Draw a small health bar for standard multi-HP enemies that have taken damage
    if (enemyType != 3 && maxHp > 1 && hp < maxHp) {
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
        glBegin(GL_LINE_LOOP);
        for(int i = 0; i < 20; ++i) {
            float theta = i * 2.0f * 3.14159f / 20.0f;
            glVertex2f(x + cos(theta) * (w + 6), y + sin(theta) * (w + 6));
        }
        glEnd();
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
