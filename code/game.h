#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>   
#include "Enemy.h"
#include "Tower.h"
#include "MapData.h"
#include "HighScore.h"
using namespace sf;

static const int COLS = 16;
static const int ROWS = 15;
static const int CELL = 52;
static const int MAP_W = COLS * CELL;    
static const int UI_W = 200;
static const int WIN_W = MAP_W + UI_W;    
static const int WIN_H = ROWS * CELL;    
static const int UI_X = MAP_W;
static const int PATH_LEN_MAX = MAX_PATH_WP;

static const int MAX_ENEMIES = 128;
static const int MAX_TOWERS  = 64;

enum class GameState {
    MAP_SELECT,    
    PREP,        
    PLAYING,       
    WAVE_CLEAR,     
    GAME_OVER, 
    WIN           
};

class Game {
    sf::RenderWindow window;
    sf::Clock clock;
    sf::Font font;
    Enemy** enemies;
    Tower** towers;
    int enemyCount;
    int towerCount;
    int selectedMap;
    MapDef currentMapDef;
    Vector2f path[PATH_LEN_MAX];
    int pathLen;
    bool pathCell[ROWS][COLS];

    int gold;
    int lives;
    int currentWave;
    int totalWaves;
    int enemiesToSpawn;
    float spawnTimer;
    float waveTimer;

    GameState state;
    int selectedTower;  
    int hoveredCol, hoveredRow;

    HighScore highScore;
    int lastScore;   

    SoundBuffer bufBgMusic;
    Music bgMusic;

    SoundBuffer bufCannon;
    SoundBuffer bufSniper;
    SoundBuffer bufMachine;
    SoundBuffer bufEnemyFiring;
    SoundBuffer bufEnemyLeave;
    SoundBuffer bufSlowBomb;
    SoundBuffer bufTowerPlace;
    SoundBuffer bufLose;
    SoundBuffer bufWon;

    Sound sndCannon;
    Sound sndSniper;
    Sound sndMachine;
    Sound sndEnemyFiring;
    Sound sndEnemyLeave;
    Sound sndSlowBomb;
    Sound sndTowerPlace;
    Sound sndLose;
    Sound sndWon;


    void buildPath();
    void spawnEnemy();
    void updateEnemies(float dt);
    void updateTowers(float dt);
    void enemiesAttackTowers(float dt);
    void cleanupDead();
    bool allEnemiesDone() const;
    void passSound(Sound* cannon, Sound* sniper,Sound* machine,Sound* slow,Sound* bomb, Sound* enemyFire);

    void drawBackground();
    void drawGrid();
    void drawPath();
    void drawEnemies();
    void drawTowers();
    void drawHUD();
    void drawTowerButtons();
    void drawOverlay();
    void drawMapSelect();
    void drawHighScoreTable(float x, float y);  

    void handleClick(int mx, int my);
    void handleMapSelectClick(int mx, int my);
    void placeTower(int col, int row);
    void recordScore();   

public:
    Game();
    ~Game();
    void run();
};