#include "Enemy.h"
#include "Tower.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp> 
#include <cmath>


extern sf::Sound* g_sndEnemyFire;
static float eDist(float ax, float ay, float bx, float by) {
    float dx = ax - bx, dy = ay - by;
    return std::sqrt(dx * dx + dy * dy);
}

Enemy::Enemy(float x, float y, int hp, float speed,int atkDmg, float atkRange, float atkRate,int gold, sf::Color color): Entity(x, y, hp),
    speed(speed), attackDamage(atkDmg),attackRange(atkRange), goldReward(gold),pathIndex(0), moveTimer(0.f),attackCooldown(0.f), attackRate(atkRate),
    slowFactor(1.f), slowTimer(0.f),towerCtx(nullptr), towerCtxCount(0)
{
    body.setSize({ 36.f, 36.f });
    body.setFillColor(color);
    body.setOutlineColor(sf::Color(255, 255, 255, 80));
    body.setOutlineThickness(1.5f);

    hpBarBg.setSize({ 36.f, 5.f });
    hpBarBg.setFillColor(sf::Color(60, 0, 0));

    hpBarFg.setSize({ 36.f, 5.f });
    hpBarFg.setFillColor(sf::Color(50, 220, 80));
}

Enemy::~Enemy() {}

void Enemy::updateHpBar() {
    float ratio = (maxHp > 0) ? (float)hp / maxHp : 0.f;
    hpBarFg.setSize({ 36.f * ratio, 5.f });
    sf::Uint8 r = (sf::Uint8)(255 * (1.f - ratio));
    sf::Uint8 g = (sf::Uint8)(220 * ratio);
    hpBarFg.setFillColor(sf::Color(r, g, 30));
}
void Enemy::setTowerContext(Tower** towers, int count) {
    towerCtx = towers;
    towerCtxCount = count;
}

bool Enemy::anyTowerInRange() const {
    if (!towerCtx) return false;
    for (int i = 0; i < towerCtxCount; ++i) {
        if (!towerCtx[i] || !towerCtx[i]->isAlive()) continue;
        if (eDist(x, y, towerCtx[i]->getX(), towerCtx[i]->getY()) <= attackRange)
            return true;
    }
    return false;
}

Tower* Enemy::nearestTower() const {
    if (!towerCtx) return nullptr;
    Tower* best = nullptr;
    float  bestD = attackRange + 1.f;
    for (int i = 0; i < towerCtxCount; ++i) {
        if (!towerCtx[i] || !towerCtx[i]->isAlive()) continue;
        float d = eDist(x, y, towerCtx[i]->getX(), towerCtx[i]->getY());
        if (d <= attackRange && d < bestD) { bestD = d; best = towerCtx[i]; }
    }
    return best;
}

void Enemy::applySlow(float factor, float duration) {
    if (factor < slowFactor) { slowFactor = factor; slowTimer = duration; }
}

float Enemy::getSpeed() const { return speed; }
int   Enemy::getAttackDamage() const { return attackDamage; }
float Enemy::getAttackRange() const { return attackRange; }
int   Enemy::getGoldReward() const { return goldReward; }
int   Enemy::getPathIndex() const { return pathIndex; }
void  Enemy::setPathIndex(int i) { pathIndex = i; }
void  Enemy::setPosition(float px, float py) { x = px; y = py; }

static void followPath(float& x, float& y, int& pathIndex,
    sf::Vector2f* path, int pathLen,
    float speed, float slowFactor, float dt)
{
    float remaining = speed * slowFactor * dt;
    while (remaining > 0.f && pathIndex + 1 < pathLen) {
        sf::Vector2f tgt = path[pathIndex + 1];
        float dx = tgt.x - x, dy = tgt.y - y;
        float d = std::sqrt(dx * dx + dy * dy);
        if (d <= remaining) {
            x = tgt.x; y = tgt.y; remaining -= d; ++pathIndex;
        }
        else {
            x += (dx / d) * remaining; y += (dy / d) * remaining; remaining = 0.f;
        }
    }
}

static void drawHpBar(sf::RenderWindow* w,
    const sf::RectangleShape& bg,
    const sf::RectangleShape& fg,
    float x, float y, float offY)
{
    sf::RectangleShape b = bg; b.setPosition(x, y + offY); w->draw(b);
    sf::RectangleShape f = fg; f.setPosition(x, y + offY); w->draw(f);
}

BasicEnemy::BasicEnemy(float x, float y): Enemy(x, y, 80, 90.f, 2, 80.f, 1.0f, 10, sf::Color(220, 60, 60)) {}
BasicEnemy::~BasicEnemy() {}

void BasicEnemy::move(sf::Vector2f* path, int pathLen, float dt) {
    if (!alive) return;
    if (anyTowerInRange()) return;
    followPath(x, y, pathIndex, path, pathLen, speed, slowFactor, dt);
}

void BasicEnemy::attackTower(Tower* , float ) {
    if (!alive) return;
    Tower* t = nearestTower();
    if (!t) return;
    if (attackCooldown <= 0.f) {
        t->takeDamage(attackDamage);
        if (g_sndEnemyFire) g_sndEnemyFire->play();
        attackCooldown = attackRate;
    }
}

void BasicEnemy::update(float dt) {
    if (!alive) return;
    if (attackCooldown > 0.f) attackCooldown -= dt;
    if (slowTimer > 0.f) { slowTimer -= dt; if (slowTimer <= 0.f) slowFactor = 1.f; }
    updateHpBar();
}

void BasicEnemy::render(void* win) const {
    auto* w = static_cast<RenderWindow*>(win);
    RectangleShape b = body;
    b.setPosition(x, y);
    if (slowFactor < 1.f) b.setOutlineColor(Color(80, 160, 255, 220));
    w->draw(b);

    CircleShape dot(5.f);
    dot.setFillColor(Color(255, 220, 220));
    dot.setPosition(x + 13.f, y + 13.f);
    w->draw(dot);

    drawHpBar(w, hpBarBg, hpBarFg, x, y, -7.f);
}

TankEnemy::TankEnemy(float x, float y): Enemy(x, y, 300, 45.f, 10, 50.f, 1.5f, 30, Color(110, 60, 180)) {}
TankEnemy::~TankEnemy() {}

void TankEnemy::move(Vector2f* path, int pathLen, float dt) {
    if (!alive) return;
    if (anyTowerInRange()) return;
    followPath(x, y, pathIndex, path, pathLen, speed, slowFactor, dt);
}

void TankEnemy::attackTower(Tower* , float ) {
    if (!alive) return;
    Tower* t = nearestTower();
    if (!t) return;
    if (attackCooldown <= 0.f) {
        t->takeDamage(attackDamage);
        attackCooldown = attackRate;
    }
}

void TankEnemy::update(float dt) {
    if (!alive) return;
    if (attackCooldown > 0.f) attackCooldown -= dt;
    if (slowTimer > 0.f) { slowTimer -= dt; if (slowTimer <= 0.f) slowFactor = 1.f; }
    updateHpBar();
}

void TankEnemy::render(void* win) const {
    auto* w = static_cast<RenderWindow*>(win);
    RectangleShape b = body;
    b.setSize({ 40.f, 40.f });
    b.setPosition(x - 2.f, y - 2.f);
    b.setOutlineColor(slowFactor < 1.f? Color(80, 160, 255, 220): Color(200, 150, 255, 120));
    b.setOutlineThickness(2.5f);
    w->draw(b);

    CircleShape dot(5.f);
    dot.setFillColor(Color(220, 200, 255));
    dot.setPosition(x + 5.f, y + 13.f); w->draw(dot);
    dot.setPosition(x + 22.f, y + 13.f); w->draw(dot);

    RectangleShape bg = hpBarBg;
    bg.setSize({ 40.f, 6.f }); bg.setPosition(x - 2.f, y - 10.f); w->draw(bg);
    RectangleShape fg = hpBarFg;
    float ratio = (maxHp > 0) ? (float)hp / maxHp : 0.f;
    fg.setSize({ 40.f * ratio, 6.f });
    Uint8 r = (Uint8)(255 * (1.f - ratio));
    Uint8 g = (Uint8)(220 * ratio);
    fg.setFillColor(Color(r, g, 30));
    fg.setPosition(x - 2.f, y - 10.f);
    w->draw(fg);
}


FastEnemy::FastEnemy(float x, float y): Enemy(x, y, 40, 200.f, 8, 40.f, 0.7f, 15, Color(0, 220, 220)) {}
FastEnemy::~FastEnemy() {}

void FastEnemy::move(Vector2f* path, int pathLen, float dt) {
    if (!alive) return;
    followPath(x, y, pathIndex, path, pathLen, speed, slowFactor, dt);
}

void FastEnemy::attackTower(Tower* , float ) {
    if (!alive) return;
    Tower* t = nearestTower();
    if (!t) return;
    if (attackCooldown <= 0.f) {
        t->takeDamage(attackDamage);
        attackCooldown = attackRate;
    }
}

void FastEnemy::update(float dt) {
    if (!alive) return;
    if (attackCooldown > 0.f) attackCooldown -= dt;
    if (slowTimer > 0.f) { slowTimer -= dt; if (slowTimer <= 0.f) slowFactor = 1.f; }
    updateHpBar();
}

void FastEnemy::render(void* win) const {
    auto* w = static_cast<sf::RenderWindow*>(win);
    sf::RectangleShape b = body;
    b.setSize({ 28.f, 28.f });
    b.setPosition(x + 4.f, y + 4.f);
    b.setFillColor(sf::Color(0, 220, 220));
    b.setOutlineColor(slowFactor < 1.f
        ? sf::Color(80, 160, 255, 220)
        : sf::Color(150, 255, 255, 180));
    b.setOutlineThickness(1.5f);
    w->draw(b);

    sf::CircleShape tri(6.f, 3);
    tri.setFillColor(sf::Color(200, 255, 255));
    tri.setPosition(x + 11.f, y + 11.f);
    w->draw(tri);

    sf::RectangleShape bg = hpBarBg;
    bg.setSize({ 28.f, 4.f }); bg.setPosition(x + 4.f, y - 6.f); w->draw(bg);
    sf::RectangleShape fg = hpBarFg;
    float ratio = (maxHp > 0) ? (float)hp / maxHp : 0.f;
    fg.setSize({ 28.f * ratio, 4.f });
    sf::Uint8 r = (sf::Uint8)(255 * (1.f - ratio));
    sf::Uint8 g = (sf::Uint8)(220 * ratio);
    fg.setFillColor(sf::Color(r, g, 30));
    fg.setPosition(x + 4.f, y - 6.f);
    w->draw(fg);
}

FlyingEnemy::FlyingEnemy(float x, float y, float exitX, float exitY): Enemy(x, y, 120, 110.f, 60, 45.f, 1.2f, 20, sf::Color(255, 200, 0)),targetX(exitX), targetY(exitY)
{
    (void)true;
}
FlyingEnemy::~FlyingEnemy() {}

void FlyingEnemy::move(sf::Vector2f*, int , float dt) {
    if (!alive) return;
    if (anyTowerInRange()) return;  

    float dx = targetX - x, dy = targetY - y;
    float d = std::sqrt(dx * dx + dy * dy);
    if (d < 4.f) { pathIndex = 9999; return; }   

    float step = speed * slowFactor * dt;
    if (step >= d) { x = targetX; y = targetY; pathIndex = 9999; }
    else { x += (dx / d) * step; y += (dy / d) * step; }
}

void FlyingEnemy::attackTower(Tower* , float ) {
    if (!alive) return;
    Tower* t = nearestTower();
    if (!t) return;
    if (attackCooldown <= 0.f) {
        t->takeDamage(attackDamage);
        attackCooldown = attackRate;
    }
}

void FlyingEnemy::update(float dt) {
    if (!alive) return;
    if (attackCooldown > 0.f) attackCooldown -= dt;
    if (slowTimer > 0.f) { slowTimer -= dt; if (slowTimer <= 0.f) slowFactor = 1.f; }
    updateHpBar();
}

void FlyingEnemy::render(void* win) const {
    auto* w = static_cast<sf::RenderWindow*>(win);

    sf::RectangleShape b({ 26.f, 26.f });
    b.setFillColor(sf::Color(255, 200, 0));
    b.setOutlineColor(slowFactor < 1.f? sf::Color(80, 160, 255, 220): sf::Color(255, 255, 160, 200));
    b.setOutlineThickness(2.f);
    b.setOrigin(13.f, 13.f);
    b.setRotation(45.f);
    b.setPosition(x + 18.f, y + 18.f);
    w->draw(b);

    sf::CircleShape wing(5.f, 3);
    wing.setFillColor(sf::Color(255, 240, 100));
    wing.setPosition(x + 4.f, y + 14.f); w->draw(wing);
    wing.setPosition(x + 27.f, y + 14.f); w->draw(wing);

    drawHpBar(w, hpBarBg, hpBarFg, x, y, -8.f);
}

HealerEnemy::HealerEnemy(float x, float y, Enemy** enemies, int* count): Enemy(x, y, 100, 70.f, 10, 42.f, 1.3f, 25, sf::Color(80, 200, 120)),healTimer(2.f), healRadius(100.f),allEnemies(enemies), enemyCount(count) {}
HealerEnemy::~HealerEnemy() {}

void HealerEnemy::move(sf::Vector2f* path, int pathLen, float dt) {
    if (!alive) return;
    if (anyTowerInRange()) return;
    followPath(x, y, pathIndex, path, pathLen, speed, slowFactor, dt);
}

void HealerEnemy::healNearby() {
    if (!allEnemies || !enemyCount) return;
    for (int i = 0; i < *enemyCount; ++i) {
        if (!allEnemies[i] || allEnemies[i] == this) continue;
        if (!allEnemies[i]->isAlive()) continue;
        float dx = allEnemies[i]->getX() - x;
        float dy = allEnemies[i]->getY() - y;
        if (std::sqrt(dx * dx + dy * dy) <= healRadius)
            allEnemies[i]->heal(15);
    }
}

void HealerEnemy::attackTower(Tower* , float ) {
    if (!alive) return;
    Tower* t = nearestTower();
    if (!t) return;
    if (attackCooldown <= 0.f) {
        t->takeDamage(attackDamage);
        attackCooldown = attackRate;
    }
}

void HealerEnemy::update(float dt) {
    if (!alive) return;
    if (attackCooldown > 0.f) attackCooldown -= dt;
    if (slowTimer > 0.f) { slowTimer -= dt; if (slowTimer <= 0.f) slowFactor = 1.f; }
    healTimer -= dt;
    if (healTimer <= 0.f) { healNearby(); healTimer = 2.f; }
    updateHpBar();
}

void HealerEnemy::render(void* win) const {
    auto* w = static_cast<sf::RenderWindow*>(win);

    sf::RectangleShape b = body;
    b.setFillColor(sf::Color(80, 200, 120));
    b.setOutlineColor(slowFactor < 1.f? sf::Color(80, 160, 255, 220): sf::Color(150, 255, 180, 200));
    b.setOutlineThickness(2.f);
    b.setPosition(x, y);
    w->draw(b);

    // Green cross (healer symbol)
    sf::RectangleShape crossH({ 18.f, 6.f });
    crossH.setFillColor(sf::Color(200, 255, 210));
    crossH.setPosition(x + 9.f, y + 15.f);
    w->draw(crossH);
    sf::RectangleShape crossV({ 6.f, 18.f });
    crossV.setFillColor(sf::Color(200, 255, 210));
    crossV.setPosition(x + 15.f, y + 9.f);
    w->draw(crossV);

    sf::CircleShape ring(healRadius);
    ring.setFillColor(sf::Color(80, 220, 120, 12));
    ring.setOutlineColor(sf::Color(80, 220, 120, 35));
    ring.setOutlineThickness(1.f);
    ring.setPosition(x + 18.f - healRadius, y + 18.f - healRadius);
    w->draw(ring);

    drawHpBar(w, hpBarBg, hpBarFg, x, y, -7.f);
}