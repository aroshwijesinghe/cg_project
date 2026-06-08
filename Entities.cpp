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
    if (y < -10) {
        alive = false;
    }
}

void Scrap::draw() const {
    glColor3f(1.0f, 0.85f, 0.0f);
    glBegin(GL_TRIANGLES);
        glVertex2f(x, y + 4);
        glVertex2f(x - 4, y);
        glVertex2f(x + 4, y);
        
        glVertex2f(x, y - 4);
        glVertex2f(x - 4, y);
        glVertex2f(x + 4, y);
    glEnd();
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
            x += sin(y * 0.02f) * 2.0f;
        }
    }
}

void Enemy::draw() const {
    if (enemyType == 0) {
        glColor3f(1.0f, 0.3f, 0.3f);
        drawRect(x, y, w, h);
        glColor3f(0.6f, 0.0f, 0.0f);
        drawRect(x, y - 8, w - 8, 6);
        glColor3f(1.0f, 1.0f, 0.0f);
        drawRect(x, y, 6, 6);
    }
    else if (enemyType == 1) {
        glColor3f(0.7f, 0.1f, 0.8f);
        drawTriangle(x, y, w, h);
        glColor3f(0.2f, 0.9f, 1.0f);
        drawRect(x, y - 2, 8, 8);
    }
    else if (enemyType == 2) {
        glColor3f(1.0f, 0.5f, 0.0f);
        drawTriangle(x, y, w, h);
        glColor3f(1.0f, 1.0f, 0.0f);
        drawRect(x, y + 10, 4, 8);
    }
    else if (enemyType == 3) {
        glColor3f(0.4f, 0.4f, 0.45f);
        drawRect(x, y, w, h);
        glColor3f(0.25f, 0.25f, 0.3f);
        drawRect(x - w/2 - 10, y, 20, h - 10);
        drawRect(x + w/2 + 10, y, 20, h - 10);
        
        float pct = (float)hp / maxHp;
        glColor3f(1.0f, 0.0f, 0.0f);
        drawRect(x, y + h/2 + 15, w * pct, 6);
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

    if (shields > 0 && hitFlashTimer <= 0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.8f, 1.0f, 0.4f);
        glBegin(GL_LINE_LOOP);
        for(int i = 0; i < 20; ++i) {
            float theta = i * 2.0f * 3.14159f / 20.0f;
            glVertex2f(x + cos(theta) * (w + 6), y + sin(theta) * (w + 6));
        }
        glEnd();
        glDisable(GL_BLEND);
    }
}
