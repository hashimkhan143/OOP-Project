#include "Tower.h"
#include "Enemy.h"
#include <SFML/Audio.hpp>  
#include <SFML/Graphics.hpp>
#include <cmath>

extern sf::Sound* g_sndCannon;     
extern sf::Sound* g_sndSniper;     
extern sf::Sound* g_sndMachine; 
extern sf::Sound* g_sndSlowBomb; 

static float tDist(float ax, float ay, float bx, float by) {
    float dx = ax - bx, dy = ay - by;
    return sqrt(dx * dx + dy * dy);
}

Tower::Tower(float x, float y, int hp, int dmg, float range,float fireRate, int cost, sf::Color col): Entity(x, y, hp),
    damage(dmg), range(range),fireRate(fireRate), fireCooldown(0.f),cost(cost), showBullet(false), bulletTimer(0.f)
{
    body.setSize({ 44.f, 44.f });
    body.setFillColor(col);
    body.setOutlineColor(sf::Color(255, 255, 255));
    body.setOutlineThickness(2.f);

    rangeCircle.setRadius(range);
    rangeCircle.setFillColor(sf::Color(255, 255, 255, 18));
    rangeCircle.setOutlineColor(sf::Color(255, 255, 255, 60));
    rangeCircle.setOutlineThickness(1.f);

    hpBarBg.setSize({ 44.f, 5.f });
    hpBarBg.setFillColor(sf::Color(40, 0, 0));

    hpBarFg.setSize({ 44.f, 5.f });
    hpBarFg.setFillColor(sf::Color(50, 220, 80));

    bullet.setRadius(6.f);
    bullet.setFillColor(sf::Color(255, 240, 80, 200));
}
Tower::~Tower() {}

void Tower::updateHpBar() {
    float ratio = (maxHp > 0) ? (float)hp / maxHp : 0.f;
    hpBarFg.setSize({ 44.f * ratio, 5.f });
    Uint8 r = (Uint8)(255 * (1.f - ratio));
    Uint8 g = (Uint8)(220 * ratio);
    hpBarFg.setFillColor(Color(r, g, 30));
}

void Tower::spawnBulletFlash(float tx, float ty) {
    bullet.setPosition(tx, ty);
    showBullet = true;
    bulletTimer = 0.12f;
}

void Tower::renderRange(RenderWindow& win) const {
    CircleShape rc = rangeCircle;
    rc.setPosition(x + 22.f - range, y + 22.f - range);
    win.draw(rc);
}

float Tower::distanceTo(float ex, float ey) const {
    return tDist(x + 22.f, y + 22.f, ex + 18.f, ey + 18.f);
}

Enemy* Tower::findTarget(Enemy** enemies, int count) const {
    for (int i = 0; i < count; ++i) {
        if (enemies[i] && enemies[i]->isAlive())
            if (distanceTo(enemies[i]->getX(), enemies[i]->getY()) <= range)
                return enemies[i];
    }
    return nullptr;
}

int Tower::getDamage() const 
{ 
    return damage;
}
float Tower::getRange() const
{
    return range; 
}
int Tower::getCost() const 
{
    return cost; 
}

static void drawTowerHpBar(RenderWindow* w,const RectangleShape& bg,const RectangleShape& fg,float x, float y)
{
    RectangleShape b = bg; b.setPosition(x, y - 8.f); w->draw(b);
    RectangleShape f = fg; f.setPosition(x, y - 8.f); w->draw(f);
}

CannonTower::CannonTower(float x, float y): Tower(x, y, 150, 70, 140.f, 0.6f, 100, sf::Color(200, 120, 30)) {}
CannonTower::~CannonTower() {}

void CannonTower::attack(Enemy** enemies, int count, float dt) {
    if (!alive)
        return;

    fireCooldown -= dt;

    if (fireCooldown > 0.f) 
        return;

    Enemy* target = nullptr;
    int    bestHp = -1;
    for (int i = 0; i < count; ++i) {
        if (!enemies[i] || !enemies[i]->isAlive()) 
            continue;
        if (distanceTo(enemies[i]->getX(), enemies[i]->getY()) <= range)
            if (enemies[i]->getHp() > bestHp) 
            {
                bestHp = enemies[i]->getHp(); target = enemies[i]; 
            }
    }
    if (target) 
    {
        target->takeDamage(damage);
        spawnBulletFlash(target->getX() + 8.f, target->getY() + 8.f);
        if (g_sndCannon) g_sndCannon->play();
        fireCooldown = 1.f / fireRate;
    }
}

void CannonTower::update(float dt) {
    if (!alive)
        return;
    if (bulletTimer > 0.f) { bulletTimer -= dt; if (bulletTimer <= 0.f) showBullet = false; }
    updateHpBar();
}

void CannonTower::render(void* win) const 
{
    auto* w = static_cast<RenderWindow*>(win);
    sf::RectangleShape b = body; b.setPosition(x, y); w->draw(b);

    sf::RectangleShape barrel({ 20.f, 8.f });
    barrel.setFillColor(Color(140, 80, 20));
    barrel.setPosition(x + 22.f, y + 18.f);
    w->draw(barrel);

    sf::CircleShape center(8.f);
    center.setFillColor(Color(255, 180, 60));
    center.setPosition(x + 14.f, y + 14.f);
    w->draw(center);

    drawTowerHpBar(w, hpBarBg, hpBarFg, x, y);
    if (showBullet)
        w->draw(bullet);
}

SniperTower::SniperTower(float x, float y)
    : Tower(x, y, 50, 80, 200.f, 0.5f, 120, sf::Color(40, 140, 200)) {
}
SniperTower::~SniperTower() {}

void SniperTower::attack(Enemy** enemies, int count, float dt) {
    if (!alive) return;
    fireCooldown -= dt;
    if (fireCooldown > 0.f) return;

    Enemy* target = nullptr;
    int    bestIdx = -1;
    for (int i = 0; i < count; ++i) {
        if (!enemies[i] || !enemies[i]->isAlive()) continue;
        if (distanceTo(enemies[i]->getX(), enemies[i]->getY()) <= range)
            if (enemies[i]->getPathIndex() > bestIdx) { bestIdx = enemies[i]->getPathIndex(); target = enemies[i]; }
    }
    if (target) {
        target->takeDamage(damage);
        spawnBulletFlash(target->getX() + 8.f, target->getY() + 8.f);
        if (g_sndSniper) g_sndSniper->play();
        fireCooldown = 1.f / fireRate;
    }
}

void SniperTower::update(float dt) {
    if (!alive)
        return;
    if (bulletTimer > 0.f) { bulletTimer -= dt; if (bulletTimer <= 0.f) showBullet = false; }
    updateHpBar();
}

void SniperTower::render(void* win) const {
    auto* w = static_cast<RenderWindow*>(win);
    RectangleShape b = body;
    b.setPosition(x, y);
    w->draw(b);

    CircleShape scope(14.f);
    scope.setFillColor(Color(20, 80, 140));
    scope.setOutlineColor(Color(100, 200, 255));
    scope.setOutlineThickness(2.f);
    scope.setPosition(x + 8.f, y + 8.f);
    w->draw(scope);

    CircleShape inner(5.f);
    inner.setFillColor(Color(180, 240, 255));
    inner.setPosition(x + 17.f, y + 17.f);
    w->draw(inner);

    RectangleShape barrel({ 22.f, 5.f });
    barrel.setFillColor(Color(20, 100, 160));
    barrel.setPosition(x + 22.f, y + 20.f);
    w->draw(barrel);

    drawTowerHpBar(w, hpBarBg, hpBarFg, x, y);
    if (showBullet) 
    {
        CircleShape bl = bullet;
        bl.setFillColor(Color(120, 220, 255, 200));
        w->draw(bl);
    }
}

MachineGunTower::MachineGunTower(float x, float y)
    : Tower(x, y, 120, 15, 130.f, 4.f, 150, Color(160, 30, 30)) {
}
MachineGunTower::~MachineGunTower() {}

void MachineGunTower::attack(Enemy** enemies, int count, float dt) {
    if (!alive)
        return;
    fireCooldown -= dt;
    if (fireCooldown > 0.f) 
        return;

    Enemy* target = nullptr;
    int    bestIdx = -1;
    for (int i = 0; i < count; ++i) {
        if (!enemies[i] || !enemies[i]->isAlive()) 
            continue;
        if (distanceTo(enemies[i]->getX(), enemies[i]->getY()) <= range)
            if (enemies[i]->getPathIndex() > bestIdx) { bestIdx = enemies[i]->getPathIndex(); target = enemies[i]; }
    }
    if (target)
    {
        target->takeDamage(damage);
        spawnBulletFlash(target->getX() + 8.f, target->getY() + 8.f);
        g_sndMachine->play();
        fireCooldown = 1.f / fireRate;
    }
}

void MachineGunTower::update(float dt) {
    if (!alive)
        return;
    if (bulletTimer > 0.f)
    { 
        bulletTimer -= dt;
        if (bulletTimer <= 0.f) 
            showBullet = false;
    }
    updateHpBar();
}

void MachineGunTower::render(void* win) const
{
    auto* w = static_cast<RenderWindow*>(win);
    RectangleShape b = body;
    b.setPosition(x, y); 
    w->draw(b);
    for (int i = 0; i < 3; ++i) 
    {
        RectangleShape barrel({ 18.f, 4.f });
        barrel.setFillColor(Color(80, 20, 20));
        barrel.setPosition(x + 24.f, y + 6.f + i * 11.f);
        w->draw(barrel);
    }

    CircleShape hub(7.f);
    hub.setFillColor(Color(230, 80, 80));
    hub.setPosition(x + 15.f, y + 15.f);
    w->draw(hub);

    drawTowerHpBar(w, hpBarBg, hpBarFg, x, y);
    if (showBullet) {
        CircleShape bl = bullet;
        bl.setRadius(3.f);
        bl.setFillColor(Color(255, 100, 100, 220));
        w->draw(bl);
    }
}

SlowTower::SlowTower(float x, float y)
    : Tower(x, y, 80, 0, 110.f, 0.5f, 130, Color(30, 80, 200)),
    pulseCooldown(2.f) {
}
SlowTower::~SlowTower() {}

void SlowTower::attack(Enemy** enemies, int count, float dt) {
    if (!alive) 
        return;
    pulseCooldown -= dt;
    if (pulseCooldown > 0.f)
        return;

    for (int i = 0; i < count; ++i) {
        if (!enemies[i] || !enemies[i]->isAlive()) continue;
        if (distanceTo(enemies[i]->getX(), enemies[i]->getY()) <= range)
            enemies[i]->applySlow(0.5f, 2.f);
    }
    spawnBulletFlash(x + 22.f, y + 22.f);
    if (g_sndSlowBomb) g_sndSlowBomb->play();
    pulseCooldown = 2.f;
}

void SlowTower::update(float dt) {
    if (!alive)
        return;
    if (bulletTimer > 0.f) 
    { 
        bulletTimer -= dt;
        if (bulletTimer <= 0.f)
            showBullet = false;
    }
    updateHpBar();
}

void SlowTower::render(void* win) const {
    auto* w = static_cast<sf::RenderWindow*>(win);
    RectangleShape b = body; b.setPosition(x, y); w->draw(b);
    RectangleShape spoke({ 20.f, 4.f });
    spoke.setFillColor(sf::Color(150, 200, 255));
    spoke.setOrigin(10.f, 2.f);
    for (int angle = 0; angle < 180; angle += 45) {
        spoke.setRotation((float)angle);
        spoke.setPosition(x + 22.f, y + 22.f);
        w->draw(spoke);
    }

    CircleShape core(6.f);
    core.setFillColor(sf::Color(200, 230, 255));
    core.setPosition(x + 16.f, y + 16.f);
    w->draw(core);

    if (showBullet) {
        sf::CircleShape ring(range);
        ring.setFillColor(sf::Color(100, 150, 255, 35));
        ring.setOutlineColor(sf::Color(150, 200, 255, 100));
        ring.setOutlineThickness(2.f);
        ring.setPosition(x + 22.f - range, y + 22.f - range);
        w->draw(ring);
    }

    drawTowerHpBar(w, hpBarBg, hpBarFg, x, y);
}

BombTower::BombTower(float x, float y)
    : Tower(x, y, 130, 55, 150.f, 0.4f, 180, sf::Color(60, 60, 60)),
    splashRadius(65.f), showSplash(false),
    splashTimer(0.f), splashX(0.f), splashY(0.f) {
}
BombTower::~BombTower() {}

void BombTower::attack(Enemy** enemies, int count, float dt) {
    if (!alive) return;
    fireCooldown -= dt;
    if (fireCooldown > 0.f) return;
    Enemy* target = nullptr;
    float  minD = range + 1.f;
    for (int i = 0; i < count; ++i) {
        if (!enemies[i] || !enemies[i]->isAlive()) continue;
        float d = distanceTo(enemies[i]->getX(), enemies[i]->getY());
        if (d <= range && d < minD) { minD = d; target = enemies[i]; }
    }

    if (target) {
        float tx = target->getX() + 18.f;
        float ty = target->getY() + 18.f;
        for (int i = 0; i < count; ++i) {
            if (!enemies[i] || !enemies[i]->isAlive()) 
                continue;
            float ex = enemies[i]->getX() + 18.f;
            float ey = enemies[i]->getY() + 18.f;
            float d = tDist(tx, ty, ex, ey);
            if (d <= splashRadius) {
                int dmg = (int)(damage * (1.f - 0.5f * d / splashRadius));
                if (dmg < 1) dmg = 1;
                enemies[i]->takeDamage(dmg);
            }
        }

        splashX = tx; splashY = ty;
        showSplash = true;
        splashTimer = 0.25f;
        spawnBulletFlash(tx - 6.f, ty - 6.f);
        if (g_sndSlowBomb) g_sndSlowBomb->play();
        fireCooldown = 1.f / fireRate;
    }
}

void BombTower::update(float dt) {
    if (!alive) return;
    if (bulletTimer > 0.f) { bulletTimer -= dt; if (bulletTimer <= 0.f) showBullet = false; }
    if (splashTimer > 0.f) { splashTimer -= dt; if (splashTimer <= 0.f) showSplash = false; }
    updateHpBar();
}

void BombTower::render(void* win) const {
    auto* w = static_cast<sf::RenderWindow*>(win);
    sf::RectangleShape b = body; b.setPosition(x, y); w->draw(b);

    sf::CircleShape bomb(14.f);
    bomb.setFillColor(sf::Color(20, 20, 20));
    bomb.setOutlineColor(sf::Color(120, 120, 120));
    bomb.setOutlineThickness(2.f);
    bomb.setPosition(x + 8.f, y + 10.f);
    w->draw(bomb);

    sf::RectangleShape fuse({ 3.f, 10.f });
    fuse.setFillColor(sf::Color(180, 140, 40));
    fuse.setPosition(x + 22.f, y + 3.f);
    fuse.setRotation(20.f);
    w->draw(fuse);

    sf::CircleShape spark(4.f);
    spark.setFillColor(sf::Color(255, 200, 50));
    spark.setPosition(x + 25.f, y + 2.f);
    w->draw(spark);

    if (showSplash) {
        float alpha = (float)(200 * (splashTimer / 0.25f));
        sf::CircleShape ex(splashRadius);
        ex.setFillColor(sf::Color(255, 120, 20, (sf::Uint8)alpha));
        ex.setOutlineColor(sf::Color(255, 200, 50, (sf::Uint8)alpha));
        ex.setOutlineThickness(2.f);
        ex.setPosition(splashX - splashRadius, splashY - splashRadius);
        w->draw(ex);
    }

    drawTowerHpBar(w, hpBarBg, hpBarFg, x, y);
    if (showBullet) w->draw(bullet);
}