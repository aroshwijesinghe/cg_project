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
      level(1), keyLeft(false), keyRight(false), keyUp(false), keyDown(false),
      screenShakeTimer(0), baseFlashTimer(0), superPowerCooldownTimer(0),
      superPowerPulseTimer(0), superPowerCenterX(WIN_W / 2.0f), superPowerCenterY(60.0f),
      superPowerReadyNotified(true) {
    // Restore player default state
    player = { WIN_W / 2.0f, 60.0f, 40.0f, 30.0f, 6.0f };
}

void Game::init() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    glMatrixMode(GL_MODELVIEW);
    srand((unsigned)time(0));
    starField.init();
    nebulaField.init();
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
    level = 1;
    superPowerCooldownTimer = 0.0f;
    superPowerPulseTimer = 0.0f;
    superPowerCenterX = WIN_W / 2.0f;
    superPowerCenterY = 60.0f;
    superPowerReadyNotified = true;

    bullets.clear();
    enemyBullets.clear();
    enemies.clear();
    scraps.clear();
    particles.clear();
    floatingTexts.clear();
    
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
    floatingTexts.clear();
    state = PLAYING;

    // Spawn floating text for wave start
    if (wave % 5 == 0) {
        if (level == 2) {
            spawnFloatingText(WIN_W / 2.0f - 110.0f, WIN_H / 2.0f, "OVERLORD INCOMING", 0.8f, 0.1f, 1.0f, 0.6f);
        } else {
            spawnFloatingText(WIN_W / 2.0f - 90.0f, WIN_H / 2.0f, "BOSS WAVE " + std::to_string(wave), 1.0f, 0.2f, 0.2f, 0.6f);
        }
    } else {
        spawnFloatingText(WIN_W / 2.0f - 50.0f, WIN_H / 2.0f, "WAVE " + std::to_string(wave), 0.2f, 0.9f, 1.0f, 0.6f);
    }
}

void Game::spawnEnemy() {
    Enemy e;
    e.alive = true;
    e.shootCooldown = rand() % 50 + 30;
    e.moveTimer = 0.0f;

    if (wave % 5 == 0) {
        // Boss wave
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
        // Level 2 normal enemies
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
        // Level 1 normal enemies
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
    } else if (e.enemyType == 5) {
        // Sentinel: fires 2 spread bullets
        EnemyBullet eb1 = { e.x - 10, e.y - e.h/2 - 6, 4.5f, true };
        EnemyBullet eb2 = { e.x + 10, e.y - e.h/2 - 6, 4.5f, true };
        enemyBullets.push_back(eb1);
        enemyBullets.push_back(eb2);
    } else if (e.enemyType == 7) {
        // Nebula Overlord: phase-based attack patterns
        float hpPct = (float)e.hp / e.maxHp;
        if (hpPct > 0.6f) {
            // Phase 1: 3 bullets downward
            EnemyBullet eb1 = { e.x - 25, e.y - 35, 4.0f, true };
            EnemyBullet eb2 = { e.x, e.y - 35, 4.5f, true };
            EnemyBullet eb3 = { e.x + 25, e.y - 35, 4.0f, true };
            enemyBullets.push_back(eb1);
            enemyBullets.push_back(eb2);
            enemyBullets.push_back(eb3);
        } else if (hpPct > 0.3f) {
            // Phase 2: 5-bullet spread fan
            for (int i = -2; i <= 2; ++i) {
                EnemyBullet eb = { e.x + i * 18.0f, e.y - 35, 4.2f + abs(i) * 0.3f, true };
                enemyBullets.push_back(eb);
            }
        } else {
            // Phase 3: 3 aimed bullets + spawn 2 Phantom minions occasionally
            EnemyBullet eb1 = { e.x - 20, e.y - 35, 5.0f, true };
            EnemyBullet eb2 = { e.x, e.y - 35, 5.5f, true };
            EnemyBullet eb3 = { e.x + 20, e.y - 35, 5.0f, true };
            enemyBullets.push_back(eb1);
            enemyBullets.push_back(eb2);
            enemyBullets.push_back(eb3);
        }
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

void Game::spawnMuzzleFlash(float x, float y, float r, float g, float b) {
    for (int i = 0; i < 4; ++i) {
        Particle p;
        p.x = x; p.y = y;
        p.vx = (rand() % 100 - 50) / 30.0f;
        p.vy = 2.0f + (rand() % 100) / 25.0f;
        p.r = r; p.g = g; p.b = b;
        p.lifetime = 0.15f + (rand() % 100) / 1000.0f;
        particles.push_back(p);
    }
}

void Game::spawnFloatingText(float x, float y, const std::string& text, float r, float g, float b, float vy) {
    FloatingText ft;
    ft.x = x;
    ft.y = y;
    ft.text = text;
    ft.vx = 0.0f;
    ft.vy = vy;
    ft.r = r;
    ft.g = g;
    ft.b = b;
    ft.alpha = 1.0f;
    ft.alive = true;
    floatingTexts.push_back(ft);
}

void Game::activateSuperPower() {
    if (state != PLAYING || superPowerCooldownTimer > 0.0f) {
        return;
    }

    superPowerCooldownTimer = SUPER_POWER_COOLDOWN_MAX;
    superPowerPulseTimer = SUPER_POWER_PULSE_MAX;
    superPowerCenterX = player.x;
    superPowerCenterY = player.y;
    superPowerReadyNotified = false;
    screenShakeTimer = std::max(screenShakeTimer, 0.22f);

    spawnFloatingText(player.x - 70.0f, player.y + 30.0f, "SUPER POWER: REFRACT", 0.2f, 0.9f, 1.0f, 1.1f);

    for (auto& eb : enemyBullets) {
        if (!eb.alive) continue;
        eb.vy = -std::abs(eb.vy);
        eb.y += 8.0f;
    }

    for (auto& e : enemies) {
        if (!e.alive) continue;
        float dx = e.x - player.x;
        float dy = e.y - player.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist <= 170.0f) {
            int damage = (e.enemyType == 3 || e.enemyType == 7) ? 4 : 2;
            e.hp -= damage;
            spawnExplosion(e.x, e.y, 0.3f, 0.9f, 1.0f);
            if (e.hp <= 0) {
                e.alive = false;
                int pts = (e.enemyType == 3 || e.enemyType == 7) ? 100 : 10;
                score += pts;
                spawnExplosion(e.x, e.y, 1.0f, 0.4f, 0.1f);
                spawnFloatingText(e.x, e.y, "+" + std::to_string(pts), 1.0f, 1.0f, 1.0f, 1.0f);

                if (e.enemyType == 3 || e.enemyType == 7) {
                    for (int i = 0; i < 8; ++i) {
                        Scrap s = { e.x + (rand() % 60 - 30), e.y + (rand() % 60 - 30), 1.2f, 0.0f, true };
                        scraps.push_back(s);
                    }
                } else if (rand() % 100 < 55) {
                    Scrap s = { e.x, e.y, 1.5f, 0.0f, true };
                    scraps.push_back(s);
                }
            }
        }
    }
}

void Game::update() {
    if (screenShakeTimer > 0) screenShakeTimer -= 0.016f;
    if (baseFlashTimer > 0) baseFlashTimer -= 0.016f;
    
    float prevCooldown = superPowerCooldownTimer;
    if (superPowerCooldownTimer > 0.0f) superPowerCooldownTimer -= 1.0f;
    if (superPowerCooldownTimer < 0.0f) superPowerCooldownTimer = 0.0f;
    
    // Notify when super power becomes ready
    if (state == PLAYING && prevCooldown > 0.0f && superPowerCooldownTimer == 0.0f && !superPowerReadyNotified) {
        superPowerReadyNotified = true;
        spawnFloatingText(player.x - 80.0f, player.y + 40.0f, "SUPER POWER READY!", 0.2f, 0.9f, 1.0f, 1.5f);
        
        // Spawn cyan particle burst around player
        for (int i = 0; i < 12; ++i) {
            Particle p;
            p.x = player.x;
            p.y = player.y;
            float angle = i * 2.0f * 3.14159f / 12.0f;
            float spd = 2.0f + (rand() % 100) / 100.0f;
            p.vx = cos(angle) * spd;
            p.vy = sin(angle) * spd;
            p.r = 0.2f; p.g = 0.9f; p.b = 1.0f;
            p.lifetime = 0.6f + (rand() % 30) / 100.0f;
            particles.push_back(p);
        }
    }
    
    if (superPowerPulseTimer > 0.0f) superPowerPulseTimer -= 0.016f;
    if (superPowerPulseTimer < 0.0f) superPowerPulseTimer = 0.0f;

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
            
            // Shooting enemies: type 1 (Shooter), 3 (L1 Boss), 5 (Sentinel), 7 (L2 Boss)
            if (e.enemyType == 1 || e.enemyType == 3 || e.enemyType == 5 || e.enemyType == 7) {
                e.shootCooldown--;
                if (e.shootCooldown <= 0) {
                    spawnEnemyBullet(e);
                    if (e.enemyType == 3) e.shootCooldown = rand() % 40 + 30;
                    else if (e.enemyType == 5) e.shootCooldown = rand() % 80 + 60;
                    else if (e.enemyType == 7) {
                        float hpPct = (float)e.hp / e.maxHp;
                        if (hpPct > 0.6f) e.shootCooldown = rand() % 40 + 35;
                        else if (hpPct > 0.3f) e.shootCooldown = rand() % 30 + 25;
                        else e.shootCooldown = rand() % 20 + 15;
                    }
                    else e.shootCooldown = rand() % 100 + 80;
                }
            }

            if (e.y < -20.0f) {
                e.alive = false;
                int dmg = (e.enemyType == 3 || e.enemyType == 7) ? 50 : 10;
                baseShields -= dmg;
                if (baseShields < 0) baseShields = 0;
                baseFlashTimer = 0.25f;
                screenShakeTimer = 0.22f;

                // Spawn floating text at the bottom where the enemy leaked
                spawnFloatingText(e.x, 30.0f, "BASE HIT! -" + std::to_string(dmg), 1.0f, 0.0f, 0.0f, 1.5f);

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
            if (wave % 5 == 0) {
                if (level == 1) {
                    // Level 1 boss defeated — transition to Level 2
                    state = LEVEL_TRANSITION;
                } else {
                    // Level 2 boss defeated — final victory!
                    state = GAME_WON;
                }
            } else {
                state = UPGRADE_SHOP;
                player.shields = player.maxShields;
            }
        }

        for (auto& s : scraps) {
            s.update();
            float dx = player.x - s.x;
            float dy = player.y - s.y;
            float dist = sqrt(dx*dx + dy*dy);
            if (dist < 100.0f) {
                s.x += (dx / dist) * 3.5f;
                s.y += (dy / dist) * 3.5f;
                
                // Spawn a golden particle trail when pulled by the player
                if (rand() % 100 < 30) {
                    Particle p = { s.x, s.y, 
                                   (float)(rand() % 20 - 10) / 10.0f, 
                                   (float)(rand() % 20 - 10) / 10.0f, 
                                   1.0f, 0.85f, 0.0f, 
                                   0.3f + (rand() % 100) / 500.0f };
                    particles.push_back(p);
                }
            } else {
                // Faint trail even when just drifting down
                if (rand() % 100 < 8) {
                    Particle p = { s.x, s.y, 
                                   (float)(rand() % 10 - 5) / 10.0f, 
                                   0.5f + (float)(rand() % 10) / 10.0f,
                                   1.0f, 0.8f, 0.2f, 
                                   0.2f + (rand() % 100) / 500.0f };
                    particles.push_back(p);
                }
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

    for (auto& ft : floatingTexts) {
        ft.update();
    }
    floatingTexts.erase(std::remove_if(floatingTexts.begin(), floatingTexts.end(),
        [](const FloatingText& ft){ return !ft.alive; }), floatingTexts.end());

    starField.update();
    nebulaField.update();
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
                    int pts = (e.enemyType == 3 || e.enemyType == 7) ? 100 : 10;
                    score += pts;
                    spawnExplosion(e.x, e.y, 1.0f, 0.4f, 0.1f);
                    spawnFloatingText(e.x, e.y, "+" + std::to_string(pts), 1.0f, 1.0f, 1.0f, 1.0f);

                    if (e.enemyType == 3 || e.enemyType == 7) {
                        for (int i = 0; i < 8; ++i) {
                            Scrap s = { e.x + (rand() % 60 - 30), e.y + (rand() % 60 - 30), 1.2f, 0.0f, true };
                            scraps.push_back(s);
                        }
                    } else if (rand() % 100 < 55) {
                        Scrap s = { e.x, e.y, 1.5f, 0.0f, true };
                        scraps.push_back(s);
                    }
                }
            }
        }
    }

    for (auto& eb : enemyBullets) {
        if (!eb.alive || eb.vy >= 0.0f) continue;
        for (auto& e : enemies) {
            if (!e.alive) continue;
            if (aabb(eb.x, eb.y, 5, 12, e.x, e.y, e.w, e.h)) {
                eb.alive = false;
                e.hp--;
                spawnExplosion(eb.x, eb.y, 0.2f, 0.9f, 1.0f);

                if (e.hp <= 0) {
                    e.alive = false;
                    int pts = (e.enemyType == 3 || e.enemyType == 7) ? 100 : 10;
                    score += pts;
                    spawnExplosion(e.x, e.y, 1.0f, 0.4f, 0.1f);
                    spawnFloatingText(e.x, e.y, "+" + std::to_string(pts), 1.0f, 1.0f, 1.0f, 1.0f);

                    if (e.enemyType == 3 || e.enemyType == 7) {
                        for (int i = 0; i < 8; ++i) {
                            Scrap s = { e.x + (rand() % 60 - 30), e.y + (rand() % 60 - 30), 1.2f, 0.0f, true };
                            scraps.push_back(s);
                        }
                    } else if (rand() % 100 < 55) {
                        Scrap s = { e.x, e.y, 1.5f, 0.0f, true };
                        scraps.push_back(s);
                    }
                }
                break;
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
            int dmg = (e.enemyType == 3 || e.enemyType == 7) ? 3 : 1;
            int prevShields = player.shields;
            int prevHull = player.hull;

            if (player.shields > 0) {
                player.shields -= dmg;
                if (player.shields < 0) {
                    player.hull += player.shields;
                    player.shields = 0;
                }
            } else {
                player.hull -= dmg;
            }

            // Spawn damage popups
            if (prevShields > 0) {
                int shieldDmg = prevShields - player.shields;
                if (shieldDmg > 0) {
                    spawnFloatingText(player.x, player.y + 15.0f, "SHIELD HIT! -" + std::to_string(shieldDmg), 0.0f, 0.8f, 1.0f, 1.4f);
                }
            }
            if (prevHull > player.hull) {
                int hullDmg = prevHull - player.hull;
                spawnFloatingText(player.x, player.y + 15.0f, "HULL HIT! -" + std::to_string(hullDmg), 1.0f, 0.2f, 0.2f, 1.4f);
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
                spawnFloatingText(player.x + 10.0f, player.y + 20.0f, "-1 SHIELD", 0.0f, 0.8f, 1.0f, 1.3f);
            } else {
                player.hull--;
                spawnFloatingText(player.x + 10.0f, player.y + 20.0f, "-1 HULL", 1.0f, 0.2f, 0.2f, 1.3f);
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
            spawnFloatingText(s.x, s.y, "+10 SCRAP", 1.0f, 0.85f, 0.0f, 1.2f);
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

    // Draw background based on current level
    if (level == 2 && state != MAIN_MENU && state != GUIDELINES && state != SHIP_SELECT) {
        nebulaField.draw();
    } else {
        starField.draw();
    }

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
    else if (state == GAME_WON) {
        drawVictoryScreen();
    }
    else if (state == LEVEL_TRANSITION) {
        drawLevelTransition();
    }
    else if (state == PLAYING || state == GAME_OVER) {
        for (const auto& b : bullets) b.draw();
        for (const auto& eb : enemyBullets) eb.draw();
        for (const auto& s : scraps) s.draw();
        for (const auto& e : enemies) e.draw();
        player.draw();
        if (superPowerPulseTimer > 0.0f) {
            float progress = 1.0f - (superPowerPulseTimer / SUPER_POWER_PULSE_MAX);
            float radius = 28.0f + progress * 180.0f;
            float beamRadius = radius - 16.0f;

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glColor4f(0.2f, 0.9f, 1.0f, 0.75f * (1.0f - progress));
            glPointSize(2.0f);
            drawMidpointCircle(superPowerCenterX, superPowerCenterY, radius);

            glColor4f(0.8f, 1.0f, 1.0f, 0.55f * (1.0f - progress));
            for (int i = 0; i < 8; ++i) {
                float theta = i * 2.0f * 3.14159f / 8.0f;
                float ex = superPowerCenterX + cos(theta) * beamRadius;
                float ey = superPowerCenterY + sin(theta) * beamRadius;
                drawBresenhamLine(superPowerCenterX, superPowerCenterY, ex, ey);
            }
            glDisable(GL_BLEND);
        }
        drawHUD();
    }

    for (const auto& p : particles) {
        p.draw();
    }

    for (const auto& ft : floatingTexts) {
        ft.draw();
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
    // Changed from Times Roman to Helvetica for a modern sci-fi look
    drawText(WIN_W / 2 - 90, WIN_H / 2 + 150, "SPACE ODYSSEY", GLUT_BITMAP_HELVETICA_18);
    glColor3f(1.0f, 0.8f, 0.2f);
    drawText(WIN_W / 2 - 95, WIN_H / 2 + 100, "ROGUE STARSHIP", GLUT_BITMAP_HELVETICA_18);

    std::string btns[3] = { "[1] START GAME", "[2] HOW TO PLAY", "[3] EXIT" };
    float ys[3] = { WIN_H / 2.0f + 30.0f, WIN_H / 2.0f - 30.0f, WIN_H / 2.0f - 90.0f };

    for (int i = 0; i < 3; ++i) {
        float bx = WIN_W / 2.0f;
        float by = ys[i];
        float bw = 260.0f;
        float bh = 45.0f;
        float corner = 12.0f; // Sci-fi angled corners

        // 3. Gradient filled background for button (2D Filling)
        glBegin(GL_POLYGON);
            glColor3f(0.04f, 0.08f, 0.2f); // Darker blue at top
            glVertex2f(bx - bw/2 + corner, by + bh/2);
            glVertex2f(bx + bw/2 - corner, by + bh/2);
            
            glColor3f(0.1f, 0.3f, 0.6f); // Lighter blue at bottom
            glVertex2f(bx + bw/2, by + bh/2 - corner);
            glVertex2f(bx + bw/2, by - bh/2 + corner);
            glVertex2f(bx + bw/2 - corner, by - bh/2);
            glVertex2f(bx - bw/2 + corner, by - bh/2);
            glVertex2f(bx - bw/2, by - bh/2 + corner);
            glVertex2f(bx - bw/2, by + bh/2 - corner);
        glEnd();
        
        // 4. Cyberpunk outline (2D Drawing)
        glColor3f(0.2f, 0.9f, 1.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(bx - bw/2 + corner, by + bh/2);
            glVertex2f(bx + bw/2 - corner, by + bh/2);
            glVertex2f(bx + bw/2, by + bh/2 - corner);
            glVertex2f(bx + bw/2, by - bh/2 + corner);
            glVertex2f(bx + bw/2 - corner, by - bh/2);
            glVertex2f(bx - bw/2 + corner, by - bh/2);
            glVertex2f(bx - bw/2, by - bh/2 + corner);
            glVertex2f(bx - bw/2, by + bh/2 - corner);
        glEnd();

        // 5. Decorative filled triangle on the left side (2D Filling)
        glBegin(GL_TRIANGLES);
            glColor3f(1.0f, 0.8f, 0.2f); // Orange
            glVertex2f(bx - bw/2 + 15.0f, by);
            glVertex2f(bx - bw/2 + 23.0f, by + 5.0f);
            glVertex2f(bx - bw/2 + 23.0f, by - 5.0f);
        glEnd();

        // 6. Button Text
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(bx - btns[i].length() * 4.5f + 15.0f, by - 5.0f, btns[i], GLUT_BITMAP_HELVETICA_12);
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
    drawText(80, WIN_H - 420, "- Press F to trigger a Refract Wave that reflects bullets and damages nearby enemies.", GLUT_BITMAP_HELVETICA_18);

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
        // Shifted right by 70 pixels to align properly and prevent overlap with the stats box
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
        drawText(bx - shopTexts[i].length() * 4.0f, by - 6, shopTexts[i], GLUT_BITMAP_HELVETICA_12);

        if (!available[i] && reasonTexts[i].length() > 0) {
            glColor3f(1.0f, 0.3f, 0.3f);
            drawText(bx - shopTexts[i].length() * 4.0f + shopTexts[i].length() * 8.0f, by - 6, reasonTexts[i], GLUT_BITMAP_HELVETICA_12);
        }
    }
}

void Game::drawVictoryScreen() {
    // Title
    glColor3f(1.0f, 0.85f, 0.0f);
    drawText(WIN_W / 2 - 120, WIN_H / 2 + 140, "CONGRATULATIONS!", GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.2f, 0.9f, 1.0f);
    drawText(WIN_W / 2 - 70, WIN_H / 2 + 100, "YOU HAVE WON!", GLUT_BITMAP_HELVETICA_18);

    // Decorative lines
    glLineWidth(2.0f);
    glColor3f(1.0f, 0.85f, 0.0f);
    glBegin(GL_LINES);
        glVertex2f(WIN_W / 2 - 150, WIN_H / 2 + 80);
        glVertex2f(WIN_W / 2 + 150, WIN_H / 2 + 80);
    glEnd();

    // Stats
    glColor3f(1.0f, 1.0f, 1.0f);
    std::stringstream sc;
    sc << "FINAL SCORE: " << score;
    drawText(WIN_W / 2 - 60, WIN_H / 2 + 40, sc.str(), GLUT_BITMAP_HELVETICA_18);

    std::stringstream wv;
    wv << "WAVES SURVIVED: " << wave;
    drawText(WIN_W / 2 - 70, WIN_H / 2 + 10, wv.str(), GLUT_BITMAP_HELVETICA_12);

    std::stringstream cr;
    cr << "SCRAP COLLECTED: " << credits;
    drawText(WIN_W / 2 - 70, WIN_H / 2 - 15, cr.str(), GLUT_BITMAP_HELVETICA_12);

    // Decorative lines
    glLineWidth(2.0f);
    glColor3f(1.0f, 0.85f, 0.0f);
    glBegin(GL_LINES);
        glVertex2f(WIN_W / 2 - 150, WIN_H / 2 - 40);
        glVertex2f(WIN_W / 2 + 150, WIN_H / 2 - 40);
    glEnd();

    // Prompt to go back
    glColor3f(0.6f, 0.6f, 0.6f);
    drawText(WIN_W / 2 - 100, WIN_H / 2 - 80, "Press R to return to Main Menu", GLUT_BITMAP_HELVETICA_12);
}

void Game::drawLevelTransition() {
    // Big title
    glColor3f(0.6f, 0.1f, 0.8f);
    drawText(WIN_W / 2 - 50, WIN_H / 2 + 160, "LEVEL 2", GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.8f, 0.3f, 1.0f);
    drawText(WIN_W / 2 - 75, WIN_H / 2 + 120, "NEBULA ZONE", GLUT_BITMAP_HELVETICA_18);

    // Decorative line
    glLineWidth(2.0f);
    glColor3f(0.6f, 0.1f, 0.8f);
    glBegin(GL_LINES);
        glVertex2f(WIN_W / 2 - 150, WIN_H / 2 + 100);
        glVertex2f(WIN_W / 2 + 150, WIN_H / 2 + 100);
    glEnd();

    // Info
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(WIN_W / 2 - 130, WIN_H / 2 + 60, "New enemies. New environment.", GLUT_BITMAP_HELVETICA_12);
    drawText(WIN_W / 2 - 115, WIN_H / 2 + 35, "Your upgrades carry over.", GLUT_BITMAP_HELVETICA_12);

    // Stats carry-over summary
    glColor3f(0.2f, 0.9f, 1.0f);
    std::stringstream hp;
    hp << "Hull: " << player.hull << "/" << player.maxHull << "  Shields: " << player.shields << "/" << player.maxShields;
    drawText(WIN_W / 2 - 100, WIN_H / 2 - 10, hp.str(), GLUT_BITMAP_HELVETICA_12);

    std::stringstream wp;
    wp << "Weapon Level: " << player.weaponLevel << "  Scrap: " << credits;
    drawText(WIN_W / 2 - 90, WIN_H / 2 - 35, wp.str(), GLUT_BITMAP_HELVETICA_12);

    // Decorative line
    glLineWidth(2.0f);
    glColor3f(0.6f, 0.1f, 0.8f);
    glBegin(GL_LINES);
        glVertex2f(WIN_W / 2 - 150, WIN_H / 2 - 60);
        glVertex2f(WIN_W / 2 + 150, WIN_H / 2 - 60);
    glEnd();

    // Prompt
    glColor3f(1.0f, 0.85f, 0.0f);
    drawText(WIN_W / 2 - 115, WIN_H / 2 - 100, "Press ENTER to begin Level 2", GLUT_BITMAP_HELVETICA_12);
}

void Game::drawHUD() {
    glColor4f(0.02f, 0.02f, 0.1f, 0.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawRect(WIN_W/2, WIN_H - 25, WIN_W, 50);
    glDisable(GL_BLEND);

    glColor3f(1.0f, 1.0f, 1.0f);
    std::stringstream sw;
    sw << "Score: " << score << "   Wave: " << wave << "   Lvl: " << level;
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
        if (level == 2) {
            drawText(WIN_W/2 - 105, WIN_H/2 - 10, "Press R to retry Level 2", GLUT_BITMAP_HELVETICA_18);
        } else {
            drawText(WIN_W/2 - 90, WIN_H/2 - 10, "Press R to restart", GLUT_BITMAP_HELVETICA_18);
        }
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
        else if (state == PLAYING) {
            if (key == 'f' || key == 'F') {
                activateSuperPower();
            }
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
                spawnFloatingText(WIN_W / 2.0f - 50.0f, WIN_H / 2.0f, "WAVE 1", 0.2f, 0.9f, 1.0f, 0.6f);
            }
            else if (key == '2') {
                reset();
                player.shipType = 1;
                player.hull = 5; player.maxHull = 5;
                player.shields = 4; player.maxShields = 4;
                player.speed = 4.5f;
                state = PLAYING;
                spawnFloatingText(WIN_W / 2.0f - 50.0f, WIN_H / 2.0f, "WAVE 1", 0.2f, 0.9f, 1.0f, 0.6f);
            }
            else if (key == '3') {
                reset();
                player.shipType = 2;
                player.hull = 3; player.maxHull = 3;
                player.shields = 1; player.maxShields = 1;
                player.speed = 5.8f;
                state = PLAYING;
                spawnFloatingText(WIN_W / 2.0f - 50.0f, WIN_H / 2.0f, "WAVE 1", 0.2f, 0.9f, 1.0f, 0.6f);
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
                if (level == 2) {
                    // Retry Level 2 — keep ship type, restart at wave 6
                    int savedShipType = player.shipType;
                    float savedSpeed = player.speed;
                    int savedMaxHull = player.maxHull;
                    int savedMaxShields = player.maxShields;

                    wave = 5;
                    enemiesSpawned = 0;
                    enemySpawnTimer = 0;
                    baseShields = maxBaseShields;
                    bullets.clear();
                    enemyBullets.clear();
                    enemies.clear();
                    scraps.clear();
                    particles.clear();
                    floatingTexts.clear();

                    player.x = WIN_W / 2.0f;
                    player.y = 60.0f;
                    player.shipType = savedShipType;
                    player.speed = savedSpeed;
                    player.hull = savedMaxHull;
                    player.maxHull = savedMaxHull;
                    player.shields = savedMaxShields;
                    player.maxShields = savedMaxShields;
                    player.weaponLevel = 1;
                    player.hitFlashTimer = 0.0f;
                    credits = 0;
                    score = 0;

                    startNextWave();
                } else {
                    reset();
                    state = MAIN_MENU;
                }
            }
            return;
        }
        else if (state == GAME_WON) {
            if (key == 'r' || key == 'R') {
                reset();
                state = MAIN_MENU;
            }
            return;
        }
        else if (state == LEVEL_TRANSITION) {
            if (key == 13) { // Enter key
                // Transition to Level 2 — keep player stats, reset wave
                level = 2;
                wave = 5; // startNextWave will increment to 6 (first Level 2 wave)
                enemiesSpawned = 0;
                bullets.clear();
                enemyBullets.clear();
                enemies.clear();
                scraps.clear();
                floatingTexts.clear();
                player.shields = player.maxShields;
                baseShields = maxBaseShields; // Refill base shields
                startNextWave();
            }
            return;
        }

        if (state == PLAYING) {
            if (key == 'p' || key == 'P') {
                state = PAUSED;
            }
            else if (key == ' ') {
                float spawnY = player.y + player.h/2.0f;
                if (player.weaponLevel == 1) {
                    Bullet b = { player.x, spawnY, 0.0f, 9.0f, true };
                    bullets.push_back(b);
                    spawnMuzzleFlash(player.x, spawnY, 0.2f, 0.9f, 1.0f);
                }
                else if (player.weaponLevel == 2) {
                    Bullet b1 = { player.x - 8.0f, spawnY, 0.0f, 9.0f, true };
                    Bullet b2 = { player.x + 8.0f, spawnY, 0.0f, 9.0f, true };
                    bullets.push_back(b1);
                    bullets.push_back(b2);
                    spawnMuzzleFlash(player.x - 8.0f, spawnY, 0.2f, 0.9f, 1.0f);
                    spawnMuzzleFlash(player.x + 8.0f, spawnY, 0.2f, 0.9f, 1.0f);
                }
                else {
                    Bullet b1 = { player.x, spawnY, 0.0f, 9.0f, true };
                    Bullet b2 = { player.x - 8.0f, spawnY, -2.0f, 8.5f, true };
                    Bullet b3 = { player.x + 8.0f, spawnY, 2.0f, 8.5f, true };
                    bullets.push_back(b1);
                    bullets.push_back(b2);
                    bullets.push_back(b3);
                    spawnMuzzleFlash(player.x, spawnY, 0.2f, 0.9f, 1.0f);
                    spawnMuzzleFlash(player.x - 8.0f, spawnY, 0.2f, 0.9f, 1.0f);
                    spawnMuzzleFlash(player.x + 8.0f, spawnY, 0.2f, 0.9f, 1.0f);
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
