#include "Entity.h"
Entity::Entity(float x, float y, int hp): x(x), y(y), hp(hp), maxHp(hp), alive(true) {}
Entity::~Entity() {}
bool Entity::operator>(const Entity& o) const 
 {
    return hp > o.hp; 
 }
bool Entity::operator<(const Entity& o) const 
{
    return hp < o.hp; 
}
void Entity::takeDamage(int dmg) 
{
    if (dmg <= 0) return;
    hp -= dmg;
    if (hp <= 0) 
    {
        hp = 0; 
        alive = false;
    }
}
void Entity::heal(int amount) 
{
    if (!alive || amount <= 0) 
        return;
    hp += amount;
    if (hp > maxHp) 
        hp = maxHp;
}
float Entity::getX() const 
{ return x; }
float Entity::getY() const 
{ return y; }
int   Entity::getHp() const 
{ return hp; }
int   Entity::getMaxHp() const 
{ return maxHp; }
bool  Entity::isAlive() const 
{ return alive; }
void  Entity::setAlive(bool v) 
{ alive = v; }