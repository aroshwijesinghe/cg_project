#include <iostream>
#include <cassert>
#include "Entities.h"

// Helper to print test status
void runTest(const char* testName, void (*testFunc)()) {
    std::cout << "[RUNNING] " << testName << "..." << std::endl;
    testFunc();
    std::cout << "[SUCCESS] " << testName << " passed!" << std::endl;
}

// 1. Collision detection (AABB) tests
void test_collision_detection() {
    // Overlapping boxes
    assert(aabb(100.0f, 100.0f, 20.0f, 20.0f, 105.0f, 105.0f, 20.0f, 20.0f) == true);
    
    // Non-overlapping boxes
    assert(aabb(100.0f, 100.0f, 20.0f, 20.0f, 200.0f, 200.0f, 20.0f, 20.0f) == false);
    
    // Edge contact (no overlap)
    assert(aabb(100.0f, 100.0f, 20.0f, 20.0f, 120.0f, 100.0f, 20.0f, 20.0f) == false);
}

// 2. Player movement and window boundary checks
void test_player_boundaries() {
    Player player;
    player.x = 400.0f;
    player.y = 300.0f;
    player.w = 40.0f;
    player.h = 30.0f;
    player.speed = 10.0f;

    // Move right normally
    player.update(false, true, false, false);
    assert(player.x == 410.0f);

    // Try to move beyond the right screen boundary (WIN_W = 800)
    player.x = 790.0f;
    player.update(false, true, false, false);
    assert(player.x == 780.0f); // Clamped at WIN_W - w/2 (800 - 20)

    // Try to move beyond the left screen boundary (0)
    player.x = 10.0f;
    player.update(true, false, false, false);
    assert(player.x == 20.0f); // Clamped at w/2 (40/2 = 20)

    // Try to move beyond the top screen boundary (WIN_H = 600)
    player.y = 590.0f;
    player.update(false, false, true, false);
    assert(player.y == 585.0f); // Clamped at WIN_H - h/2 (600 - 15)
}

// 3. Particle system updates
void test_particle_updates() {
    Particle p;
    p.x = 100.0f;
    p.y = 100.0f;
    p.vx = 2.0f;
    p.vy = -1.0f;
    p.lifetime = 1.0f;

    p.update();
    assert(p.x == 102.0f);
    assert(p.y == 99.0f);
    assert(p.lifetime < 1.0f); // Lifetime should decrease
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "     RUNNING GAME ENGINE UNIT TESTS     " << std::endl;
    std::cout << "========================================" << std::endl;

    runTest("Collision Detection (AABB)", test_collision_detection);
    runTest("Player Boundaries & Movement", test_player_boundaries);
    runTest("Particle System Updates", test_particle_updates);

    std::cout << "========================================" << std::endl;
    std::cout << "      ALL TESTS PASSED SUCCESSFULLY!    " << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
