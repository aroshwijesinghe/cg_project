#ifndef ENTITIES_H
#define ENTITIES_H

#include <vector>
#include <string>

const int WIN_W = 800;
const int WIN_H = 600;

enum GameState { MAIN_MENU, GUIDELINES, SHIP_SELECT, PLAYING, UPGRADE_SHOP, GAME_OVER, GAME_WON, PAUSED };

void drawRect(float cx, float cy, float w, float h);
void drawTriangle(float cx, float cy, float w, float h);

bool aabb(float ax, float ay, float aw, float ah,
          float bx, float by, float bw, float bh);

struct Particle {
    float x, y;
    float vx, vy;
    float r, g, b;
    float lifetime;
    void update();
    void draw() const;
};

struct Star {
    float x, y, speed;
};

class StarField {
public:
    void init();
    void update();
    void draw() const;
private:
    std::vector<Star> stars;
};

struct Bullet {
    float x, y;
    float vx, vy;
    bool  alive;

    void update();
    void draw() const;
};

struct EnemyBullet {
    float x, y;
    float vy;
    bool  alive;

    void update();
    void draw() const;
};

struct Scrap {
    float x, y;
    float speed;
    float angle;
    bool  alive;

    void update();
    void draw() const;
};

struct Enemy {
    float x, y;
    float w, h;
    float speed;
    bool  alive;
    int   enemyType;
    int   hp;
    int   maxHp;
    int   shootCooldown;
    float moveTimer;  // Used for boss oscillation

    void update(float playerX);
    void draw() const;
};

struct Player {
    float x, y;
    float w, h;
    float speed;
    int   shipType;
    int   hull;
    int   maxHull;
    int   shields;
    int   maxShields;
    int   weaponLevel;
    float hitFlashTimer;

    void update(bool left, bool right, bool up, bool down);
    void draw() const;
};

struct FloatingText {
    float x, y;
    std::string text;
    float vx, vy;
    float r, g, b;
    float alpha;
    bool alive;

    void update();
    void draw() const;
};

#endif
