#ifndef GAME_H
#define GAME_H

#include "Entities.h"
#include <vector>
#include <string>

class Game {
public:
    Game();
    void init();
    void reset();
    void update();
    void draw();
    void handleInput(unsigned char key, bool pressed);
    void handleSpecialInput(int key, bool pressed);

    GameState getState() const { return state; }

private:
    GameState state;
    int score;
    int credits;
    int wave;
    int enemySpawnTimer;
    int enemiesSpawned;
    int maxEnemies;
    int baseShields;
    int maxBaseShields;
    int level;

    Player player;
    std::vector<Bullet> bullets;
    std::vector<EnemyBullet> enemyBullets;
    std::vector<Enemy> enemies;
    std::vector<Scrap> scraps;
    std::vector<Particle> particles;
    std::vector<FloatingText> floatingTexts;
    StarField starField;
    NebulaField nebulaField;

    bool keyLeft, keyRight, keyUp, keyDown;
    float screenShakeTimer;
    float baseFlashTimer;
    float superPowerCooldownTimer;
    float superPowerPulseTimer;
    float superPowerCenterX;
    float superPowerCenterY;
    bool superPowerReadyNotified;
    static constexpr float SUPER_POWER_COOLDOWN_MAX = 360.0f;
    static constexpr float SUPER_POWER_PULSE_MAX = 0.45f;

    void spawnEnemy();
    void spawnEnemyBullet(const Enemy& e);
    void spawnExplosion(float x, float y, float r, float g, float b);
    void spawnMuzzleFlash(float x, float y, float r, float g, float b);
    void spawnFloatingText(float x, float y, const std::string& text, float r, float g, float b, float vy = 1.2f);
    void activateSuperPower();
    void checkCollisions();
    void startNextWave();

    void drawMainMenu();
    void drawGuidelines();
    void drawShipSelect();
    void drawUpgradeShop();
    void drawHUD();
    void drawVictoryScreen();
    void drawLevelTransition();
    void drawText(float x, float y, const std::string& s, void* font);
};

#endif
