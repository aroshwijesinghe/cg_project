#include "Game.h"
#include <GL/freeglut.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <sstream>
#include <algorithm>

Game::Game() 
    : state(MAIN_MENU), score(0), credits(0), wave(1), enemySpawnTimer(0),
      enemiesSpawned(0), maxEnemies(5), baseShields(100), maxBaseShields(100),
      keyLeft(false), keyRight(false), keyUp(false), keyDown(false),
      screenShakeTimer(0), baseFlashTimer(0) {
    player = { WIN_W / 2.0f, 60.0f, 40.0f, 30.0f, 6.0f };
}

void Game::init() {
    glClearColor(0.01f, 0.01f, 0.04f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    glMatrixMode(GL_MODELVIEW);
    srand((unsigned)time(0));
    starField.init();
    reset();
    state = MAIN_MENU;
}

void Game::reset() {
    score = 0;
    credits = 0;
    wave = 1;
    enemySpawnTimer = 0;
    enemiesSpawned = 0;
    maxEnemies = 5;
    baseShields = 100;
    maxBaseShields = 100;

    bullets.clear();
    enemyBullets.clear();
    enemies.clear();
    scraps.clear();
    particles.clear();
    
    player.x = WIN_W / 2.0f;
    player.y = 60.0f;
    player.shipType = 0;
    player.hull = 3;
    player.maxHull = 3;
    player.shields = 2;
    player.maxShields = 2;
    player.speed = 6.5f;
    player.weaponLevel = 1;
    player.hitFlashTimer = 0.0f;
}

void Game::startNextWave() {
    wave++;
    enemiesSpawned = 0;
    if (wave % 5 == 0) {
        maxEnemies = 1;
    } else {
        maxEnemies = 5 + wave * 2;
    }
    bullets.clear();
    enemyBullets.clear();
    enemies.clear();
    state = PLAYING;
}

void Game::spawnEnemy() {
    Enemy e;
    e.alive = true;
    e.shootCooldown = rand() % 50 + 30;

    if (wave % 5 == 0) {
        e.enemyType = 3;
        e.w = 110.0f; e.h = 60.0f;
        e.x = WIN_W / 2.0f;
        e.y = WIN_H + 50.0f;
        e.speed = 0.8f;
        e.maxHp = 20 + wave * 5;
        e.hp = e.maxHp;
    } else {
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

void Game::spawnEnemyBullet(const Enemy& e) {
    if (e.enemyType == 1) {
        EnemyBullet eb = { e.x, e.y - e.h/2 - 6, 4.2f + wave * 0.15f, true };
        enemyBullets.push_back(eb);
    } else if (e.enemyType == 3) {
        EnemyBullet eb1 = { e.x - 20, e.y - 30, 4.0f, true };
        EnemyBullet eb2 = { e.x, e.y - 30, 4.5f, true };
        EnemyBullet eb3 = { e.x + 20, e.y - 30, 4.0f, true };
        enemyBullets.push_back(eb1);
        enemyBullets.push_back(eb2);
        enemyBullets.push_back(eb3);
    }
}

void Game::spawnExplosion(float x, float y, float r, float g, float b) {
    for (int i = 0; i < 20; ++i) {
        Particle p;
        p.x = x; p.y = y;
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float spd = 1.0f + (rand() % 100) / 100.0f * 3.5f;
        p.vx = cos(angle) * spd;
        p.vy = sin(angle) * spd;
        p.r = r; p.g = g; p.b = b;
        p.lifetime = 0.7f + (rand() % 30) / 100.0f;
        particles.push_back(p);
    }
}

void Game::update() {
    if (screenShakeTimer > 0) screenShakeTimer -= 0.016f;
    if (baseFlashTimer > 0) baseFlashTimer -= 0.016f;

    if (state == PLAYING) {
        player.update(keyLeft, keyRight, keyUp, keyDown);

        for (auto& b : bullets) {
            b.update();
        }
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
            [](const Bullet& b){ return !b.alive; }), bullets.end());

        for (auto& eb : enemyBullets) {
            eb.update();
        }
        enemyBullets.erase(std::remove_if(enemyBullets.begin(), enemyBullets.end(),
            [](const EnemyBullet& eb){ return !eb.alive; }), enemyBullets.end());

        for (auto& e : enemies) {
            e.update(player.x);
            
            if (e.enemyType == 1 || e.enemyType == 3) {
                e.shootCooldown--;
                if (e.shootCooldown <= 0) {
                    spawnEnemyBullet(e);
                    e.shootCooldown = (e.enemyType == 3) ? (rand() % 40 + 30) : (rand() % 100 + 80);
                }
            }

            if (e.y < -20.0f) {
                e.alive = false;
                int dmg = (e.enemyType == 3) ? 50 : 10;
                baseShields -= dmg;
                if (baseShields < 0) baseShields = 0;
                baseFlashTimer = 0.25f;
                screenShakeTimer = 0.22f;

                if (baseShields <= 0) {
                    state = GAME_OVER;
                }
            }
        }
        enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
            [](const Enemy& e){ return !e.alive; }), enemies.end());

        enemySpawnTimer++;
        int spawnInterval = std::max(25, 75 - wave * 4);
        if (enemiesSpawned < maxEnemies && enemySpawnTimer >= spawnInterval) {
            spawnEnemy();
            enemySpawnTimer = 0;
        }

        if (enemiesSpawned >= maxEnemies && enemies.empty()) {
            state = UPGRADE_SHOP;
            player.shields = player.maxShields;
        }

        for (auto& s : scraps) {
            s.update();
            float dx = player.x - s.x;
            float dy = player.y - s.y;
            float dist = sqrt(dx*dx + dy*dy);
            if (dist < 100.0f) {
                s.x += (dx / dist) * 3.5f;
                s.y += (dy / dist) * 3.5f;
            }
        }
        scraps.erase(std::remove_if(scraps.begin(), scraps.end(),
            [](const Scrap& s){ return !s.alive; }), scraps.end());

        checkCollisions();
    }

    for (auto& p : particles) {
        p.update();
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(),
        [](const Particle& p){ return p.lifetime <= 0.0f; }), particles.end());

    starField.update();
}

void Game::checkCollisions() {
    for (auto& b : bullets) {
        if (!b.alive) continue;
        for (auto& e : enemies) {
            if (!e.alive) continue;
            if (aabb(b.x, b.y, 4, 12, e.x, e.y, e.w, e.h)) {
                b.alive = false;
                e.hp--;
                spawnExplosion(b.x, b.y, 0.2f, 0.9f, 1.0f);

                if (e.hp <= 0) {
                    e.alive = false;
                    score += (e.enemyType == 3) ? 100 : 10;
                    spawnExplosion(e.x, e.y, 1.0f, 0.4f, 0.1f);

                    if (e.enemyType == 3) {
                        for (int i = 0; i < 6; ++i) {
                            Scrap s = { e.x + (rand() % 40 - 20), e.y + (rand() % 40 - 20), 1.2f, true };
                            scraps.push_back(s);
                        }
                    } else if (rand() % 100 < 55) {
                        Scrap s = { e.x, e.y, 1.5f, true };
                        scraps.push_back(s);
                    }
                }
            }
        }
    }

    for (auto& e : enemies) {
        if (!e.alive) continue;
        if (aabb(player.x, player.y, player.w, player.h, e.x, e.y, e.w, e.h)) {
            e.alive = false;
            spawnExplosion(e.x, e.y, 1.0f, 0.2f, 0.2f);
            
            player.hitFlashTimer = 0.2f;
            screenShakeTimer = 0.28f;
            int dmg = (e.enemyType == 3) ? 3 : 1;

            if (player.shields > 0) {
                player.shields -= dmg;
                if (player.shields < 0) {
                    player.hull += player.shields;
                    player.shields = 0;
                }
            } else {
                player.hull -= dmg;
            }

            if (player.hull <= 0) {
                player.hull = 0;
                state = GAME_OVER;
            }
        }
    }

    for (auto& eb : enemyBullets) {
        if (!eb.alive) continue;
        if (aabb(eb.x, eb.y, 5, 12, player.x, player.y, player.w, player.h)) {
            eb.alive = false;
            spawnExplosion(eb.x, eb.y, 1.0f, 0.5f, 0.0f);

            player.hitFlashTimer = 0.18f;
            screenShakeTimer = 0.18f;

            if (player.shields > 0) {
                player.shields--;
            } else {
                player.hull--;
            }

            if (player.hull <= 0) {
                player.hull = 0;
                state = GAME_OVER;
            }
        }
    }

    for (auto& s : scraps) {
        if (!s.alive) continue;
        if (aabb(s.x, s.y, 8, 8, player.x, player.y, player.w, player.h)) {
            s.alive = false;
            credits += 10;
            for (int i = 0; i < 5; ++i) {
                Particle p = { s.x, s.y, (float)(rand()%20-10)/5.0f, (float)(rand()%20-10)/5.0f, 1.0f, 0.85f, 0.0f, 0.5f };
                particles.push_back(p);
            }
        }
    }
}

void Game::draw() {
    glClear(GL_COLOR_BUFFER_BIT);

    glPushMatrix();
    if (screenShakeTimer > 0) {
        float shakeX = (rand() % 10 - 5) * 1.5f;
        float shakeY = (rand() % 10 - 5) * 1.5f;
        glTranslatef(shakeX, shakeY, 0.0f);
    }

    starField.draw();

    if (state == MAIN_MENU) {
        drawMainMenu();
    }
    else if (state == GUIDELINES) {
        drawGuidelines();
    }
    else if (state == SHIP_SELECT) {
        drawShipSelect();
    }
    else if (state == UPGRADE_SHOP) {
        drawUpgradeShop();
    }
    else if (state == PLAYING || state == GAME_OVER) {
        for (const auto& b : bullets) b.draw();
        for (const auto& eb : enemyBullets) eb.draw();
        for (const auto& s : scraps) s.draw();
        for (const auto& e : enemies) e.draw();
        player.draw();
        drawHUD();
    }

    for (const auto& p : particles) {
        p.draw();
    }

    if (baseFlashTimer > 0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 0.0f, 0.0f, baseFlashTimer * 0.8f);
        glLineWidth(12.0f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(0, 0);
            glVertex2f(WIN_W, 0);
            glVertex2f(WIN_W, WIN_H);
            glVertex2f(0, WIN_H);
        glEnd();
        glDisable(GL_BLEND);
    }

    glPopMatrix();
    glutSwapBuffers();
}

void Game::drawMainMenu() {
    glColor3f(0.2f, 0.9f, 1.0f);
    drawText(WIN_W / 2 - 170, WIN_H / 2 + 150, "SPACE ODYSSEY", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(1.0f, 0.8f, 0.2f);
    drawText(WIN_W / 2 - 90, WIN_H / 2 + 100, "ROGUE STARSHIP", GLUT_BITMAP_HELVETICA_18);

    std::string btns[3] = { "[1] START GAME", "[2] HOW TO PLAY", "[3] EXIT" };
    float ys[3] = { WIN_H / 2.0f + 30.0f, WIN_H / 2.0f - 30.0f, WIN_H / 2.0f - 90.0f };

    for (int i = 0; i < 3; ++i) {
        float bx = WIN_W / 2.0f;
        float by = ys[i];
        float bw = 240.0f;
        float bh = 45.0f;

        glColor3f(0.08f, 0.15f, 0.3f);
        drawRect(bx, by, bw, bh);
        
        glColor3f(0.2f, 0.9f, 1.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(bx - bw/2, by - bh/2);
            glVertex2f(bx + bw/2, by - bh/2);
            glVertex2f(bx + bw/2, by + bh/2);
            glVertex2f(bx - bw/2, by + bh/2);
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(bx - btns[i].length() * 4.5f, by - 6.0f, btns[i], GLUT_BITMAP_HELVETICA_12);
    }
}

void Game::drawGuidelines() {
    glColor3f(0.2f, 0.9f, 1.0f);
    drawText(WIN_W / 2 - 120, WIN_H - 100, "HOW TO PLAY / INSTRUCTIONS", GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(80, WIN_H - 180, "- Control your ship using Arrow Keys or WASD keys.", GLUT_BITMAP_HELVETICA_18);
    drawText(80, WIN_H - 220, "- Press Spacebar to fire your lasers.", GLUT_BITMAP_HELVETICA_18);
    drawText(80, WIN_H - 260, "- Dodge incoming orange enemy plasma bolts.", GLUT_BITMAP_HELVETICA_18);
    drawText(80, WIN_H - 300, "- Do not let enemies bypass the bottom boundary. It drains base shields!", GLUT_BITMAP_HELVETICA_18);
    drawText(80, WIN_H - 340, "- Destroy enemies to collect golden Scrap Credits. Use them to upgrade in the shop.", GLUT_BITMAP_HELVETICA_18);
    drawText(80, WIN_H - 380, "- Defeat the Heavy Carrier Boss on Wave 5 to survive.", GLUT_BITMAP_HELVETICA_18);

    float bx = WIN_W / 2.0f;
    float by = 100.0f;
    float bw = 200.0f;
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
    drawText(bx - 65, by - 6, "[B] BACK TO MENU", GLUT_BITMAP_HELVETICA_12);
}

void Game::drawShipSelect() {
    glColor3f(0.2f, 0.9f, 1.0f);
    drawText(WIN_W / 2 - 130, WIN_H - 100, "CHOOSE YOUR STARSHIP", GLUT_BITMAP_TIMES_ROMAN_24);

    std::string btns[3] = { "[1] INTERCEPTOR", "[2] AEGIS TANK", "[3] VANGUARD" };

    for (int i = 0; i < 3; ++i) {
        float cx = 180.0f + i * 220.0f;
        float cy = WIN_H / 2.0f + 50.0f;
        
        glColor3f(0.1f, 0.15f, 0.25f);
        drawRect(cx, cy, 180, 200);

        glColor3f(0.2f, 0.8f, 1.0f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(cx - 90, cy - 100);
            glVertex2f(cx + 90, cy - 100);
            glVertex2f(cx + 90, cy + 100);
            glVertex2f(cx - 90, cy + 100);
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        if (i == 0) {
            drawText(cx - 50, cy + 80, "INTERCEPTOR", GLUT_BITMAP_HELVETICA_12);
            glColor3f(0.2f, 0.9f, 1.0f);
            drawTriangle(cx, cy + 10, 30, 25);
            glColor3f(1.0f, 1.0f, 1.0f);
            drawText(cx - 70, cy - 20, "Speed: Fast (6.5)", GLUT_BITMAP_HELVETICA_10);
            drawText(cx - 70, cy - 40, "Shields: Weak (2)", GLUT_BITMAP_HELVETICA_10);
            drawText(cx - 70, cy - 60, "Weapons: Focused", GLUT_BITMAP_HELVETICA_10);
        }
        else if (i == 1) {
            drawText(cx - 40, cy + 80, "AEGIS TANK", GLUT_BITMAP_HELVETICA_12);
            glColor3f(0.1f, 0.6f, 0.9f);
            drawRect(cx, cy + 10, 26, 26);
            glColor3f(1.0f, 1.0f, 1.0f);
            drawText(cx - 70, cy - 20, "Speed: Slow (4.5)", GLUT_BITMAP_HELVETICA_10);
            drawText(cx - 70, cy - 40, "Shields: Heavy (4)", GLUT_BITMAP_HELVETICA_10);
            drawText(cx - 70, cy - 60, "Weapons: Standard", GLUT_BITMAP_HELVETICA_10);
        }
        else {
            drawText(cx - 40, cy + 80, "VANGUARD", GLUT_BITMAP_HELVETICA_12);
            glColor3f(0.9f, 0.2f, 0.2f);
            drawTriangle(cx, cy + 10, 30, 25);
            glColor3f(0.5f, 0.1f, 0.1f);
            drawTriangle(cx - 12, cy + 5, 10, 18);
            drawTriangle(cx + 12, cy + 5, 10, 18);
            glColor3f(1.0f, 1.0f, 1.0f);
            drawText(cx - 70, cy - 20, "Speed: Med (5.8)", GLUT_BITMAP_HELVETICA_10);
            drawText(cx - 70, cy - 40, "Shields: Light (1)", GLUT_BITMAP_HELVETICA_10);
            drawText(cx - 70, cy - 60, "Weapons: Triple", GLUT_BITMAP_HELVETICA_10);
        }

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
        drawText(bx - btns[i].length() * 4.0f, by - 6, btns[i], GLUT_BITMAP_HELVETICA_12);
    }

    glColor3f(0.5f, 0.5f, 0.5f);
    drawText(WIN_W / 2 - 80, 50, "[B] BACK TO MAIN MENU", GLUT_BITMAP_HELVETICA_12);
}

void Game::drawUpgradeShop() {
    glColor3f(1.0f, 0.85f, 0.0f);
    drawText(WIN_W / 2 - 120, WIN_H - 80, "ROGUE UPGRADE STATION", GLUT_BITMAP_TIMES_ROMAN_24);

    std::stringstream ss;
    ss << "AVAILABLE SCRAP: " << credits << " CREDITS";
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(WIN_W / 2 - 110, WIN_H - 120, ss.str(), GLUT_BITMAP_HELVETICA_18);

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
    drawText(60, WIN_H/2 + 80, "CURRENT SHIP STATS:", GLUT_BITMAP_HELVETICA_12);
    std::stringstream sh;
    sh << "- Weapon Level: Lvl " << player.weaponLevel;
    drawText(60, WIN_H/2 + 40, sh.str(), GLUT_BITMAP_HELVETICA_12);
    std::stringstream hl;
    hl << "- Hull Points: " << player.hull << " / " << player.maxHull;
    drawText(60, WIN_H/2, hl.str(), GLUT_BITMAP_HELVETICA_12);
    std::stringstream sl;
    sl << "- Shield Level: Lvl " << player.maxShields;
    drawText(60, WIN_H/2 - 40, sl.str(), GLUT_BITMAP_HELVETICA_12);
    std::stringstream bs;
    bs << "- Base Shields: " << baseShields << "%";
    drawText(60, WIN_H/2 - 80, bs.str(), GLUT_BITMAP_HELVETICA_12);

    bool canBuyWeapon = (credits >= 100 && player.weaponLevel < 3);
    bool canRepairHull = (credits >= 30 && player.hull < player.maxHull);
    bool canBuyShield = (credits >= 50);
    bool available[4] = { canBuyWeapon, canRepairHull, canBuyShield, true };

    std::string reasonTexts[4] = { "", "", "", "" };
    if (!canBuyWeapon) {
        if (player.weaponLevel >= 3) reasonTexts[0] = "  [MAX LEVEL]";
        else reasonTexts[0] = "  [NOT ENOUGH SCRAP]";
    }
    if (!canRepairHull) {
        if (player.hull >= player.maxHull) reasonTexts[1] = "  [HULL FULL]";
        else reasonTexts[1] = "  [NOT ENOUGH SCRAP]";
    }
    if (!canBuyShield) {
        reasonTexts[2] = "  [NOT ENOUGH SCRAP]";
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
        float bx = WIN_W / 2.0f;
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
        drawText(bx - shopTexts[i].length() * 4.0f, by - 6, shopTexts[i], GLUT_BITMAP_HELVETICA_12);

        if (!available[i] && reasonTexts[i].length() > 0) {
            glColor3f(1.0f, 0.3f, 0.3f);
            drawText(bx - shopTexts[i].length() * 4.0f + shopTexts[i].length() * 8.0f, by - 6, reasonTexts[i], GLUT_BITMAP_HELVETICA_12);
        }
    }
}

void Game::drawHUD() {
    glColor4f(0.02f, 0.02f, 0.1f, 0.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawRect(WIN_W/2, WIN_H - 25, WIN_W, 50);
    glDisable(GL_BLEND);

    glColor3f(1.0f, 1.0f, 1.0f);
    std::stringstream sw;
    sw << "Score: " << score << "   Wave: " << wave;
    drawText(15, WIN_H - 32, sw.str(), GLUT_BITMAP_HELVETICA_18);

    std::stringstream sc;
    sc << "Scrap: " << credits;
    glColor3f(1.0f, 0.85f, 0.0f);
    drawText(220, WIN_H - 32, sc.str(), GLUT_BITMAP_HELVETICA_18);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(360, WIN_H - 32, "HULL: ", GLUT_BITMAP_HELVETICA_12);
    glColor3f(0.3f, 0.0f, 0.0f);
    drawRect(440, WIN_H - 25, player.maxHull * 15, 12);
    glColor3f(0.2f, 0.9f, 0.2f);
    for (int i = 0; i < player.hull; ++i) {
        drawRect(440 - (player.maxHull * 15)/2.0f + 7.5f + i * 15.0f, WIN_H - 25, 12, 10);
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(495, WIN_H - 32, "SHIELD: ", GLUT_BITMAP_HELVETICA_12);
    glColor3f(0.0f, 0.1f, 0.3f);
    drawRect(585, WIN_H - 25, player.maxShields * 15, 12);
    glColor3f(0.0f, 0.8f, 1.0f);
    for (int i = 0; i < player.shields; ++i) {
        drawRect(585 - (player.maxShields * 15)/2.0f + 7.5f + i * 15.0f, WIN_H - 25, 12, 10);
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(635, WIN_H - 32, "BASE: ", GLUT_BITMAP_HELVETICA_12);
    glColor3f(0.2f, 0.2f, 0.2f);
    drawRect(720, WIN_H - 25, 100, 12);
    glColor3f(0.8f, 0.0f, 1.0f);
    float pct = (float)baseShields / maxBaseShields;
    drawRect(720 - 50.0f + pct * 50.0f, WIN_H - 25, 100 * pct, 10);

    if (state == PAUSED) {
        glColor3f(1.0f, 1.0f, 0.0f);
        drawText(WIN_W/2 - 40, WIN_H/2, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);
    }

    if (state == GAME_OVER) {
        glColor3f(1.0f, 0.2f, 0.2f);
        drawText(WIN_W/2 - 70, WIN_H/2 + 20, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(WIN_W/2 - 90, WIN_H/2 - 10, "Press R to restart", GLUT_BITMAP_HELVETICA_18);
    }
}

void Game::drawText(float x, float y, const std::string& s, void* font) {
    glRasterPos2f(x, y);
    for (char c : s) {
        glutBitmapCharacter(font, c);
    }
}

void Game::handleInput(unsigned char key, bool pressed) {
    if (pressed) {
        if (key == 27) exit(0);

        if (state == MAIN_MENU) {
            if (key == '1') {
                state = SHIP_SELECT;
            } else if (key == '2') {
                state = GUIDELINES;
            } else if (key == '3') {
                exit(0);
            }
            return;
        }
        else if (state == GUIDELINES) {
            if (key == 'b' || key == 'B') {
                state = MAIN_MENU;
            }
            return;
        }
        else if (state == SHIP_SELECT) {
            if (key == '1') {
                reset();
                player.shipType = 0;
                player.hull = 3; player.maxHull = 3;
                player.shields = 2; player.maxShields = 2;
                player.speed = 6.5f;
                state = PLAYING;
            }
            else if (key == '2') {
                reset();
                player.shipType = 1;
                player.hull = 5; player.maxHull = 5;
                player.shields = 4; player.maxShields = 4;
                player.speed = 4.5f;
                state = PLAYING;
            }
            else if (key == '3') {
                reset();
                player.shipType = 2;
                player.hull = 3; player.maxHull = 3;
                player.shields = 1; player.maxShields = 1;
                player.speed = 5.8f;
                state = PLAYING;
            }
            else if (key == 'b' || key == 'B') {
                state = MAIN_MENU;
            }
            return;
        }
        else if (state == UPGRADE_SHOP) {
            if (key == '1') {
                if (credits >= 100 && player.weaponLevel < 3) {
                    credits -= 100;
                    player.weaponLevel++;
                }
            }
            else if (key == '2') {
                if (credits >= 30 && player.hull < player.maxHull) {
                    credits -= 30;
                    player.hull++;
                }
            }
            else if (key == '3') {
                if (credits >= 50) {
                    credits -= 50;
                    player.maxShields++;
                    player.shields = player.maxShields;
                }
            }
            else if (key == 13) {
                startNextWave();
            }
            return;
        }
        else if (state == GAME_OVER) {
            if (key == 'r' || key == 'R') {
                reset();
                state = MAIN_MENU;
            }
            return;
        }

        if (state == PLAYING) {
            if (key == 'p' || key == 'P') {
                state = PAUSED;
            }
            else if (key == ' ') {
                if (player.weaponLevel == 1) {
                    Bullet b = { player.x, player.y + player.h/2.0f, 0.0f, 9.0f, true };
                    bullets.push_back(b);
                }
                else if (player.weaponLevel == 2) {
                    Bullet b1 = { player.x - 8.0f, player.y + player.h/2.0f, 0.0f, 9.0f, true };
                    Bullet b2 = { player.x + 8.0f, player.y + player.h/2.0f, 0.0f, 9.0f, true };
                    bullets.push_back(b1);
                    bullets.push_back(b2);
                }
                else {
                    Bullet b1 = { player.x, player.y + player.h/2.0f, 0.0f, 9.0f, true };
                    Bullet b2 = { player.x - 8.0f, player.y + player.h/2.0f, -2.0f, 8.5f, true };
                    Bullet b3 = { player.x + 8.0f, player.y + player.h/2.0f, 2.0f, 8.5f, true };
                    bullets.push_back(b1);
                    bullets.push_back(b2);
                    bullets.push_back(b3);
                }
            }
            if (key == 'a' || key == 'A') keyLeft = true;
            if (key == 'd' || key == 'D') keyRight = true;
            if (key == 'w' || key == 'W') keyUp = true;
            if (key == 's' || key == 'S') keyDown = true;
        }
        else if (state == PAUSED) {
            if (key == 'p' || key == 'P') {
                state = PLAYING;
            }
        }
    } else {
        if (state == PLAYING) {
            if (key == 'a' || key == 'A') keyLeft = false;
            if (key == 'd' || key == 'D') keyRight = false;
            if (key == 'w' || key == 'W') keyUp = false;
            if (key == 's' || key == 'S') keyDown = false;
        }
    }
}

void Game::handleSpecialInput(int key, bool pressed) {
    if (state == PLAYING) {
        if (pressed) {
            if (key == GLUT_KEY_LEFT)  keyLeft  = true;
            if (key == GLUT_KEY_RIGHT) keyRight = true;
            if (key == GLUT_KEY_UP)    keyUp    = true;
            if (key == GLUT_KEY_DOWN)  keyDown  = true;
        } else {
            if (key == GLUT_KEY_LEFT)  keyLeft  = false;
            if (key == GLUT_KEY_RIGHT) keyRight = false;
            if (key == GLUT_KEY_UP)    keyUp    = false;
            if (key == GLUT_KEY_DOWN)  keyDown  = false;
        }
    }
}
