#pragma once
#include "Entity.h"
#include <SFML/Graphics.hpp>
using namespace sf;
class Enemy;

class Tower : public Entity {
protected:
    int damage,cost;
    float range, fireRate, fireCooldown;
    

    RectangleShape body;
    CircleShape rangeCircle;
    RectangleShape hpBarBg;
    RectangleShape hpBarFg;
    CircleShape bullet;
    bool showBullet;
    float bulletTimer;

    void updateHpBar();
    void spawnBulletFlash(float tx, float ty);

public:
    Tower(float x, float y, int hp, int dmg, float range,float fireRate, int cost, sf::Color col);
    virtual ~Tower();
    virtual void attack(Enemy** enemies, int count, float dt) = 0;
    virtual void update(float dt) = 0;
    virtual void render(void* window) const = 0;
    void renderRange(sf::RenderWindow& win) const;
    Enemy* findTarget(Enemy** enemies, int count) const;
    float distanceTo(float ex, float ey) const;
    float getRange() const;
    int getDamage() const;
    int getCost() const;
};

class CannonTower : public Tower {
public:
    CannonTower(float x, float y);
    ~CannonTower();
    void attack(Enemy** enemies, int count, float dt) override;
    void update(float dt) override;
    void render(void* window) const override;
};

class SniperTower : public Tower {
public:
    SniperTower(float x, float y);
    ~SniperTower();
    void attack(Enemy** enemies, int count, float dt) override;
    void update(float dt) override;
    void render(void* window) const override;
};

class MachineGunTower : public Tower {
public:
    MachineGunTower(float x, float y);
    ~MachineGunTower();
    void attack(Enemy** enemies, int count, float dt) override;
    void update(float dt) override;
    void render(void* window) const override;
};

class SlowTower : public Tower {
    float pulseCooldown;
public:
    SlowTower(float x, float y);
    ~SlowTower();
    void attack(Enemy** enemies, int count, float dt) override;
    void update(float dt) override;
    void render(void* window) const override;
};

class BombTower : public Tower {
    float splashRadius;
    bool showSplash;
    float splashTimer;
    float splashX, splashY;
public:
    BombTower(float x, float y);
    ~BombTower();
    void attack(Enemy** enemies, int count, float dt) override;
    void update(float dt) override;
    void render(void* window) const override;
};