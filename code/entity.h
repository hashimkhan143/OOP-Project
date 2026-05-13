#pragma once
class Entity {
protected:
    float x, y;  
    int hp;
    int maxHp;
    bool alive;
public:
    Entity(float x, float y, int hp);
    virtual ~Entity();
    virtual void update(float dt) = 0;
    virtual void render(void* window) const = 0;
    bool operator>(const Entity& o) const;
    bool operator<(const Entity& o) const;
    void takeDamage(int dmg);
    void heal(int amount);         
    float getX() const;
    float getY() const;
    int getHp() const;
    int getMaxHp() const;
    bool isAlive() const;
    void setAlive(bool v);
};