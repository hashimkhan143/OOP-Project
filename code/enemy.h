#pragma once
#include "Entity.h"
#include <SFML/Graphics.hpp>
using namespace sf;

class Tower;  
class Enemy : public Entity {
protected:
    float speed, attackRange, moveTimer;
    float attackCooldown;
    float attackRate, slowFactor, slowTimer;
    int attackDamage,goldReward,pathIndex;
        
    Tower** towerCtx;
    int towerCtxCount;

    RectangleShape body,hpBarBg,hpBarFg;
    CircleShape icon;

    void updateHpBar();
    bool anyTowerInRange() const;
    Tower* nearestTower() const;

public:
    Enemy(float x, float y, int hp, float speed,int atkDmg, float atkRange, float atkRate,int gold, Color color);
    virtual ~Enemy();

    virtual void move(Vector2f* path, int pathLen, float dt) = 0;
    virtual void attackTower(Tower* tower, float dt) = 0;
    virtual void update(float dt) = 0;
    virtual void render(void* window) const = 0;
    void setTowerContext(Tower** towers, int count);

    void applySlow(float factor, float duration);

    float getSpeed() const;
    int getAttackDamage() const;
    float getAttackRange()const;
    int getGoldReward()const;
    int getPathIndex() const;
    void setPathIndex(int i);
    void setPosition(float px, float py);
};

class BasicEnemy : public Enemy {
public:
    BasicEnemy(float x, float y);
    ~BasicEnemy();
    void move(Vector2f* path, int pathLen, float dt) override;
    void attackTower(Tower* tower, float dt) override;
    void update(float dt) override;
    void render(void* window) const override;
};
class TankEnemy : public Enemy {
public:
    TankEnemy(float x, float y);
    ~TankEnemy();
    void move(Vector2f* path, int pathLen, float dt) override;
    void attackTower(Tower* tower, float dt) override;
    void update(float dt) override;
    void render(void* window) const override;
};

class FastEnemy : public Enemy {
public:
    FastEnemy(float x, float y);
    ~FastEnemy();
    void move(Vector2f* path, int pathLen, float dt) override;
    void attackTower(Tower* tower, float dt) override;
    void update(float dt) override;
    void render(void* window) const override;
};

class FlyingEnemy : public Enemy {
    float targetX, targetY;
public:
    FlyingEnemy(float x, float y, float exitX, float exitY);
    ~FlyingEnemy();
    void move(Vector2f* path, int pathLen, float dt) override;
    void attackTower(Tower* tower, float dt) override;
    void update(float dt) override;
    void render(void* window) const override;
};

class HealerEnemy : public Enemy {
    float   healTimer;
    float   healRadius;
    Enemy** allEnemies;
    int* enemyCount;
public:
    HealerEnemy(float x, float y, Enemy** enemies, int* count);
    ~HealerEnemy();
    void move(Vector2f* path, int pathLen, float dt) override;
    void attackTower(Tower* tower, float dt) override;
    void update(float dt) override;
    void render(void* window) const override;
    void healNearby();
};