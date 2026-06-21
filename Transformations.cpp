/*=============================================================================
 * Transformations.cpp - Topic 3: Transformations
 *
 * This file demonstrates fundamental 2D transformation concepts:
 *
 *   1. OBJECT TRANSLATION (Movement):
 *      Particle::update() translates particles by adding velocity vectors
 *      to position: x += vx, y += vy. This is the most basic form of
 *      2D translation, equivalent to the transformation matrix:
 *        | 1  0  tx |   where tx = vx, ty = vy
 *        | 0  1  ty |
 *        | 0  0  1  |
 *
 *   2. DYNAMIC SCALING:
 *      Particle::draw() applies dynamic scaling to particle size based on
 *      remaining lifetime: sz = 5.0f * lifetime + 1.0f. As lifetime
 *      decreases, particles shrink, creating a fade-out effect.
 *      This demonstrates non-uniform scaling over time.
 *
 *   3. COMPOSITE TRANSFORMATION MATRICES:
 *      Game::draw() uses glPushMatrix/glPopMatrix with glTranslatef to
 *      apply screen shake. This is a composite transformation where the
 *      translation is applied to the entire scene temporarily.
 *
 *      Scrap::draw() uses glPushMatrix + glTranslatef + glRotatef to
 *      compose a translation-then-rotation matrix for spinning scrap items.
 *      The composite matrix is: M = T(x,y) * R(angle)
 *
 *   4. ENEMY MOVEMENT PATTERNS:
 *      Enemy::update() implements 8 different translation patterns using
 *      velocity vectors, sinusoidal oscillation, and player tracking.
 *      Each enemy type demonstrates a different approach to 2D translation.
 *=============================================================================*/

#include "ViewingClipping.h"
#include <GL/freeglut.h>
#include <cstdlib>
#include <cmath>
#include <sstream>
#include <algorithm>

/*=============================================================================
 * PARTICLE TRANSLATION
 *
 * Applies basic 2D translation to each particle by adding its velocity
 * vector (vx, vy) to its position (x, y). This is equivalent to:
 *   position_new = position_old + velocity
 *
 * The lifetime is decremented each frame, and when it reaches zero
 * the particle is flagged for removal (handled in Game::update()).
 *=============================================================================*/
void Particle::update() {
    x += vx;
    y += vy;
    lifetime -= 0.025f;
}

/*=============================================================================
 * PARTICLE RENDERING WITH DYNAMIC SCALING
 *
 * Demonstrates dynamic scaling: the particle size shrinks proportionally
 * to its remaining lifetime. The scaling formula:
 *   sz = 5.0f * lifetime + 1.0f
 *
 * When lifetime = 0.7 (fresh particle):  sz = 4.5 pixels
 * When lifetime = 0.1 (dying particle):  sz = 1.5 pixels
 * When lifetime = 0.0 (dead):            sz = 1.0 pixels
 *
 * This creates a smooth shrinking animation that simulates particles
 * dissipating over time. Alpha blending is also tied to lifetime
 * for a fade-out effect combined with the scaling.
 *=============================================================================*/
void Particle::draw() const {
    if (lifetime <= 0.0f) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, lifetime);
    float sz = 5.0f * lifetime + 1.0f;
    drawRect(x, y, sz, sz);
    glDisable(GL_BLEND);
}

/*=============================================================================
 * ENEMY TRANSLATION PATTERNS
 *
 * Each enemy type has a unique movement pattern demonstrating different
 * 2D translation approaches:
 *
 * Type 0 (Drone): Simple downward translation (y -= speed)
 *
 * Type 1 (Shooter): Descends to a Y threshold, then oscillates
 *   horizontally using sinusoidal translation: x += sin(y * 0.05) * 1.8
 *
 * Type 2 (Seeker): Descends while tracking player X position.
 *   Applies conditional horizontal translation toward the player.
 *
 * Type 3 (L1 Boss): Descends at half speed, then oscillates using
 *   time-based sin(moveTimer) for smooth horizontal movement.
 *   Position is boundary-clamped to keep boss on screen.
 *
 * Type 4 (Phantom): Zigzag descent using sin(moveTimer) * 3.5
 *   combined with constant downward translation.
 *
 * Type 5 (Sentinel): Descends to position, then orbits using
 *   cos(moveTimer) for X and sin(moveTimer) for Y — circular motion.
 *
 * Type 6 (Wraith): Fast player-tracking descent.
 *   Moves toward playerX at 1.8 units/frame.
 *
 * Type 7 (Nebula Overlord): Three-phase movement based on HP:
 *   Phase 1 (>60% HP): Slow sin oscillation
 *   Phase 2 (30-60% HP): Faster oscillation + vertical bobbing
 *   Phase 3 (<30% HP): Erratic random movement with cos/sin
 *   Position is clamped to a bounded region.
 *=============================================================================*/
void Enemy::update(float playerX) {
    if (enemyType == 0) {
        // Drone: simple downward translation
        y -= speed;
    }
    else if (enemyType == 1) {
        // Shooter: descend then oscillate horizontally
        if (y > WIN_H - 160) {
            y -= speed;
        } else {
            x += sin(y * 0.05f) * 1.8f;
        }
        // Clamp within screen bounds
        if (x < w/2 + 5) x = w/2 + 5;
        if (x > WIN_W - w/2 - 5) x = WIN_W - w/2 - 5;
    }
    else if (enemyType == 2) {
        // Seeker: descend while tracking player position
        y -= speed;
        if (x < playerX) x += 1.0f;
        if (x > playerX) x -= 1.0f;
        // Clamp within screen bounds
        if (x < w/2 + 5) x = w/2 + 5;
        if (x > WIN_W - w/2 - 5) x = WIN_W - w/2 - 5;
    }
    else if (enemyType == 3) {
        // L1 Boss: descend slowly, then time-based oscillation
        if (y > WIN_H - 150) {
            y -= speed * 0.5f;
        } else {
            // Use a time-based counter for smooth oscillation instead of sin(y)
            moveTimer += 0.02f;
            x += sin(moveTimer) * 2.5f;
            // Clamp boss within screen bounds (boundary clamping)
            if (x < w/2 + 20) x = w/2 + 20;
            if (x > WIN_W - w/2 - 20) x = WIN_W - w/2 - 20;
        }
    }
    // --- Level 2 Enemies ---
    // Type 4: Phantom — Zigzag descent using sinusoidal translation
    else if (enemyType == 4) {
        y -= speed;
        moveTimer += 0.05f;
        x += sin(moveTimer) * 3.5f;
        // Clamp within screen bounds
        if (x < w/2 + 5) x = w/2 + 5;
        if (x > WIN_W - w/2 - 5) x = WIN_W - w/2 - 5;
    }
    // Type 5: Sentinel — Descends then orbits using cos/sin circular motion
    else if (enemyType == 5) {
        if (y > WIN_H - 200) {
            y -= speed;
        } else {
            moveTimer += 0.03f;
            x += cos(moveTimer) * 2.0f;
            y += sin(moveTimer) * 1.0f;
        }
        // Clamp within screen bounds
        if (x < w/2 + 5) x = w/2 + 5;
        if (x > WIN_W - w/2 - 5) x = WIN_W - w/2 - 5;
    }
    // Type 6: Wraith — Fast tracker that fades in/out
    else if (enemyType == 6) {
        y -= speed * 0.6f;
        if (x < playerX) x += 1.8f;
        if (x > playerX) x -= 1.8f;
        moveTimer += 0.03f; // Used for alpha fade in draw()
        // Clamp within screen bounds
        if (x < w/2 + 5) x = w/2 + 5;
        if (x > WIN_W - w/2 - 5) x = WIN_W - w/2 - 5;
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
            // Boundary clamping for boss (keeps within playable area)
            if (x < w/2 + 20) x = w/2 + 20;
            if (x > WIN_W - w/2 - 20) x = WIN_W - w/2 - 20;
            if (y < WIN_H - 250) y = WIN_H - 250;
            if (y > WIN_H - 80) y = WIN_H - 80;
        }
    }
}

/*=============================================================================
 * SCRAP RENDERING WITH COMPOSITE MATRIX TRANSFORMATION
 *
 * Demonstrates composite transformation using OpenGL matrix stack:
 *
 *   1. glPushMatrix() - Save current transformation state
 *   2. glTranslatef(x, y, 0) - Translate to scrap position
 *   3. glRotatef(angle, 0, 0, 1) - Rotate around Z axis
 *   4. Draw geometry at origin (which is now at (x,y) rotated)
 *   5. glPopMatrix() - Restore previous transformation state
 *
 * The composite matrix M = T(x,y) * R(angle) first rotates the
 * diamond shape, then translates it to the scrap's world position.
 *
 * The golden glow circle behind the scrap uses GL_TRIANGLE_FAN with
 * additive blending for a halo effect (see RotationVectors for the
 * trigonometric vertex generation used in circle rendering).
 *=============================================================================*/
void Scrap::draw() const {
    // Golden glow circle behind the scrap (additive blending)
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

    // --- Composite transformation: Translate + Rotate ---
    glPushMatrix();
    glTranslatef(x, y, 0.0f);                    // Step 1: Translate to position
    glRotatef(angle, 0.0f, 0.0f, 1.0f);          // Step 2: Rotate around Z axis
    
    // Draw rotating diamond at origin (transformation applied)
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
    glPopMatrix();  // Restore transformation state
}

/*=============================================================================
 * MASTER DRAW FUNCTION WITH SCREEN SHAKE TRANSFORMATION
 *
 * Demonstrates composite scene-level transformation:
 *   - glPushMatrix() saves the identity modelview matrix
 *   - When screenShakeTimer > 0, a random translation offset is applied
 *     to the ENTIRE scene using glTranslatef(shakeX, shakeY, 0)
 *   - All subsequent draw calls inherit this translation
 *   - glPopMatrix() restores the original matrix, removing the shake
 *
 * This is a powerful demonstration of how matrix stack operations
 * can apply transformations to composite objects (the entire game scene).
 *=============================================================================*/
void Game::draw() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Push matrix and apply screen shake translation to entire scene
    glPushMatrix();
    if (screenShakeTimer > 0) {
        float shakeX = (rand() % 10 - 5) * 1.5f;
        float shakeY = (rand() % 10 - 5) * 1.5f;
        glTranslatef(shakeX, shakeY, 0.0f);  // Composite translation
    }

    // Draw background based on current level (staging - see AnimationPrinciples)
    if (level == 2 && state != MAIN_MENU && state != GUIDELINES && state != SHIP_SELECT) {
        nebulaField.draw();
    } else {
        starField.draw();
    }

    // Draw state-specific screens
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
        // Super power pulse ring (uses rotation vectors - see RotationVectors)
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

    // Render particles (transformed positions from Particle::update)
    for (const auto& p : particles) {
        p.draw();
    }

    // Render floating text
    for (const auto& ft : floatingTexts) {
        ft.draw();
    }

    // Base hit flash — red border overlay (animation feedback)
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

    glPopMatrix();  // Restore matrix (removes screen shake)
    glutSwapBuffers();
}

/*=============================================================================
 * MASTER UPDATE LOOP
 *
 * Manages all per-frame updates including:
 *   - Timer decrements (screen shake, base flash, super power cooldown)
 *   - Player, bullet, enemy, and scrap position translation updates
 *   - Enemy spawn timing and wave progression
 *   - Scrap magnetic attraction (translation toward player)
 *   - Dead entity cleanup using std::remove_if
 *
 * The scrap attraction demonstrates dynamic translation: when a scrap
 * item is within 100 units of the player, its position is translated
 * toward the player at 3.5 units/frame using normalized direction vectors.
 *=============================================================================*/
void Game::update() {
    if (screenShakeTimer > 0) screenShakeTimer -= 0.016f;
    if (baseFlashTimer > 0) baseFlashTimer -= 0.016f;
    
    float prevCooldown = superPowerCooldownTimer;
    if (superPowerCooldownTimer > 0.0f) superPowerCooldownTimer -= 1.0f;
    if (superPowerCooldownTimer < 0.0f) superPowerCooldownTimer = 0.0f;
    
    // Notify when super power becomes ready (spawns radial particle burst)
    if (state == PLAYING && prevCooldown > 0.0f && superPowerCooldownTimer == 0.0f && !superPowerReadyNotified) {
        superPowerReadyNotified = true;
        spawnFloatingText(player.x - 80.0f, player.y + 40.0f, "SUPER POWER READY!", 0.2f, 0.9f, 1.0f, 1.5f);
        
        // Spawn cyan particle burst around player (uses rotation vectors)
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
        // Update player position (translation + boundary clamping)
        player.update(keyLeft, keyRight, keyUp, keyDown);

        // Update bullet positions (translation + off-screen clipping)
        for (auto& b : bullets) {
            b.update();
        }
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
            [](const Bullet& b){ return !b.alive; }), bullets.end());

        // Update enemy bullet positions
        for (auto& eb : enemyBullets) {
            eb.update();
        }
        enemyBullets.erase(std::remove_if(enemyBullets.begin(), enemyBullets.end(),
            [](const EnemyBullet& eb){ return !eb.alive; }), enemyBullets.end());

        // Update enemy positions (various translation patterns)
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

            // Enemy leaked past bottom — damages base shields
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

        // Enemy spawn timing (see AnimationPrinciples for spawnEnemy())
        enemySpawnTimer++;
        int spawnInterval = std::max(25, 75 - wave * 4);
        if (enemiesSpawned < maxEnemies && enemySpawnTimer >= spawnInterval) {
            spawnEnemy();
            enemySpawnTimer = 0;
        }

        // Wave completion check
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

        // Scrap magnetic attraction (dynamic translation toward player)
        for (auto& s : scraps) {
            s.update();
            float dx = player.x - s.x;
            float dy = player.y - s.y;
            float dist = sqrt(dx*dx + dy*dy);
            if (dist < 100.0f) {
                // Translate scrap toward player using normalized direction
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

    // Update particle positions (translation)
    for (auto& p : particles) {
        p.update();
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(),
        [](const Particle& p){ return p.lifetime <= 0.0f; }), particles.end());

    // Update floating text positions
    for (auto& ft : floatingTexts) {
        ft.update();
    }
    floatingTexts.erase(std::remove_if(floatingTexts.begin(), floatingTexts.end(),
        [](const FloatingText& ft){ return !ft.alive; }), floatingTexts.end());

    // Update background animations (parallax staging)
    starField.update();
    nebulaField.update();
}

/*=============================================================================
 * WAVE MANAGEMENT
 *
 * Handles the transition between waves by incrementing wave counter,
 * resetting spawn counts, and adjusting difficulty (maxEnemies).
 *=============================================================================*/
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

/*=============================================================================
 * ENEMY BULLET SPAWN WITH POSITION OFFSETS
 *
 * Spawns enemy bullets at calculated offset positions relative to
 * the enemy's current position. Different enemy types use different
 * offset patterns (single, triple, spread, phase-based).
 *=============================================================================*/
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
