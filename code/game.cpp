#include "Game.h"
#include <cstring>
#include <cstdio>
#include <cmath>
using namespace sf;
using namespace std;

Sound* g_sndCannon = nullptr;  
Sound* g_sndSniper = nullptr;
Sound* g_sndMachine = nullptr;  
Sound* g_sndSlowBomb = nullptr;  
Sound* g_sndEnemyFire = nullptr;

Game::Game(): window(VideoMode(WIN_W, WIN_H), "Tower Defense", Style::Titlebar | Style::Close),
   enemyCount(0), towerCount(0),gold(250), lives(15),currentWave(0), totalWaves(7),enemiesToSpawn(0), spawnTimer(0.f),
   waveTimer(3.f),state(GameState::MAP_SELECT),selectedTower(0),hoveredCol(-1), hoveredRow(-1),selectedMap(0), pathLen(0),lastScore(0)
{

    window.setFramerateLimit(80);

    enemies = new Enemy * [MAX_ENEMIES];
    towers = new Tower * [MAX_TOWERS];
    for (int i = 0; i < MAX_ENEMIES; ++i) 
        enemies[i] = nullptr;
    for (int i = 0; i < MAX_TOWERS; ++i) 
        towers[i] = nullptr;

    font.loadFromFile("C:/Windows/Fonts/arial.ttf");
    memset(pathCell, 0, sizeof(pathCell));
  
    bufCannon.loadFromFile("sound/cannon.wav");
    bufSniper.loadFromFile("sound/sniperfire.wav");
    bufMachine.loadFromFile("sound/machine.wav");
    bufEnemyFiring.loadFromFile("sound/Enemyfiring.wav");
    bufEnemyLeave.loadFromFile("sound/enemyLeave.wav");
    bufSlowBomb.loadFromFile("sound/faaaa.wav");
    bufTowerPlace.loadFromFile("sound/Towerplacing.wav");
    bufLose.loadFromFile("sound/lose.wav");
    bufWon.loadFromFile("sound/Won.wav");

    sndCannon.setBuffer(bufCannon);
    sndSniper.setBuffer(bufSniper);
    sndMachine.setBuffer(bufMachine);
    sndEnemyFiring.setBuffer(bufEnemyFiring);
    sndEnemyLeave.setBuffer(bufEnemyLeave);
    sndSlowBomb.setBuffer(bufSlowBomb);
    sndTowerPlace.setBuffer(bufTowerPlace);
    sndLose.setBuffer(bufLose);
    sndWon.setBuffer(bufWon);

    sndMachine.setVolume(60.f);
    sndCannon.setVolume(85.f);
    sndSniper.setVolume(80.f);

    bgMusic.openFromFile("sound/bgmusic.wav");
    bgMusic.setLoop(true);
    bgMusic.setVolume(30.f);
    bgMusic.play();
}

Game::~Game()
{
    for (int i = 0; i < MAX_ENEMIES; ++i) 
    {
        delete enemies[i]; 
        enemies[i] = nullptr; 
    }
    delete[] enemies;
    for (int i = 0; i < MAX_TOWERS; ++i) 
    {
        delete towers[i]; 
        towers[i] = nullptr; 
    }
    delete[] towers;
}

//  loads the selected map into runtime arrays
void Game::buildPath() {
    memset(pathCell, 0, sizeof(pathCell));

    pathLen = currentMapDef.pathLen;
    const int* wc = currentMapDef.wpCol;
    const int* wr = currentMapDef.wpRow;

    for (int i = 0; i < pathLen; ++i) 
    {
        path[i] = sf::Vector2f((float)(wc[i] * CELL) + 4.f,
            (float)(wr[i] * CELL) + 4.f);
        if (wr[i] >= 0 && wr[i] < ROWS && wc[i] >= 0 && wc[i] < COLS)
            pathCell[wr[i]][wc[i]] = true;

        // Marks every cell between consecutive waypoints as path
        if (i + 1 < pathLen) 
        {
            int dc = wc[i + 1] - wc[i], dr = wr[i + 1] - wr[i];
            int steps = (abs(dc) > abs(dr)) ? abs(dc) : abs(dr);
            for (int s = 1; s < steps; ++s) 
            {
                int mc = wc[i] + (dc > 0 ? s : (dc < 0 ? -s : 0));
                int mr = wr[i] + (dr > 0 ? s : (dr < 0 ? -s : 0));
                if (mr >= 0 && mr < ROWS && mc >= 0 && mc < COLS)
                    pathCell[mr][mc] = true;
            }
        }
    }
}

void Game::spawnEnemy() {
    if (enemyCount >= MAX_ENEMIES) return;

    float sx = path[0].x, sy = path[0].y;
    float ex = path[pathLen - 1].x, ey = path[pathLen - 1].y;

    int roll = enemyCount % 10;
    Enemy* e = nullptr;

    switch (currentWave) {
    case 1: 
        e = new BasicEnemy(sx, sy);
        break;
    case 2:
        e = (roll < 6) ? (Enemy*)new BasicEnemy(sx, sy): (Enemy*)new FastEnemy(sx, sy);
        break;
    case 3:
        e = (roll < 5) ? (Enemy*)new BasicEnemy(sx, sy): (Enemy*)new TankEnemy(sx, sy);
        break;
    case 4:
        e = (roll < 5) ? (Enemy*)new FastEnemy(sx, sy): (Enemy*)new FlyingEnemy(sx, sy, ex, ey);
        break;
    case 5:
        if (roll < 3) 
            e = new TankEnemy(sx, sy);
        else if (roll < 7) 
            e = new BasicEnemy(sx, sy);
        else              
            e = new HealerEnemy(sx, sy, enemies, &enemyCount);
        break;
    case 6:
        if (roll < 2) 
            e = new TankEnemy(sx, sy);
        else if (roll < 4) 
            e = new FastEnemy(sx, sy);
        else if (roll < 6) 
            e = new FlyingEnemy(sx, sy, ex, ey);
        else if (roll < 8) 
            e = new HealerEnemy(sx, sy, enemies, &enemyCount);
        else               
            e = new BasicEnemy(sx, sy);
        break;
    default: // wave 7
        if (roll < 3)
            e = new TankEnemy(sx, sy);
        else if (roll < 5) 
            e = new FlyingEnemy(sx, sy, ex, ey);
        else if (roll < 7) 
            e = new HealerEnemy(sx, sy, enemies, &enemyCount);
        else               
            e = new FastEnemy(sx, sy);
        break;
    }

    if (e) enemies[enemyCount++] = e;
}

 void Game::recordScore() 
 {
    lastScore = HighScore::calcScore(currentWave - 1, gold, lives);
    highScore.addScore("Player", lastScore);
    bgMusic.stop();                            
    if (state == GameState::GAME_OVER) 
        sndLose.play();  
    if (state == GameState::WIN)       
        sndWon.play();    
 }

void Game::updateEnemies(float dt)
{
    for (int i = 0; i < enemyCount; ++i)
    {
        if (!enemies[i] || !enemies[i]->isAlive()) 
            continue;
        enemies[i]->setTowerContext(towers, towerCount);
        enemies[i]->move(path, pathLen, dt);
        enemies[i]->update(dt);

        bool atExit = (enemies[i]->getPathIndex() >= pathLen - 1) ||(enemies[i]->getPathIndex() >= 9999);
        if (atExit)
        {
            --lives;
            enemies[i]->setAlive(false);
            sndEnemyLeave.play();
            if (lives <= 0)
            {
                state = GameState::GAME_OVER;
                recordScore();
            }
        }
    }
}
void Game::updateTowers(float dt) 
{
    for (int j = 0; j < towerCount; ++j) 
    {
        if (!towers[j] || !towers[j]->isAlive()) 
            continue;
        towers[j]->update(dt);
        towers[j]->attack(enemies, enemyCount, dt);
    }
}
void Game::enemiesAttackTowers(float dt) 
{
    for (int i = 0; i < enemyCount; ++i) 
    {
        if (!enemies[i] || !enemies[i]->isAlive()) 
            continue;
        enemies[i]->attackTower(nullptr, dt);
    }
}

void Game::cleanupDead()
{
    for (int i = 0; i < enemyCount; ++i) 
    {
        if (!enemies[i]) 
            continue;
        if (!enemies[i]->isAlive()) 
        {
            if (enemies[i]->getHp() == 0)
                gold += enemies[i]->getGoldReward();
            delete enemies[i]; 
            enemies[i] = nullptr;
        }
    }
    int w = 0;
    for (int r = 0; r < MAX_ENEMIES; ++r)
        if (enemies[r]) 
            enemies[w++] = enemies[r];
    enemyCount = w;
    for (int i = w; i < MAX_ENEMIES; ++i) 
        enemies[i] = nullptr;

    for (int j = 0; j < towerCount; ++j) 
    {
        if (towers[j] && !towers[j]->isAlive()) 
        {
            delete towers[j]; 
            towers[j] = nullptr;
        }
    }
    w = 0;
    for (int r = 0; r < MAX_TOWERS; ++r)
        if (towers[r]) towers[w++] = towers[r];
    towerCount = w;
    for (int i = w; i < MAX_TOWERS; ++i) towers[i] = nullptr;
}

bool Game::allEnemiesDone() const {
    if (enemiesToSpawn > 0)
        return false;
    for (int i = 0; i < enemyCount; ++i)
        if (enemies[i] && enemies[i]->isAlive()) 
            return false;
    return true;
}

void Game::drawMapSelect() {
    sf::RectangleShape bg({ (float)WIN_W, (float)WIN_H });
    bg.setFillColor(sf::Color(10, 12, 20));
    window.draw(bg);
    sf::Text title;
    title.setFont(font);
    title.setString("TOWER DEFENSE");
    title.setCharacterSize(42);
    title.setFillColor(sf::Color(100, 200, 255));
    title.setPosition(WIN_W / 2.f - 190.f, 28.f);
    window.draw(title);

    sf::Text sub;
    sub.setFont(font);
    sub.setString("Select a Map");
    sub.setCharacterSize(20);
    sub.setFillColor(sf::Color(160, 180, 200));
    sub.setPosition(WIN_W / 2.f - 80.f, 84.f);
    window.draw(sub);
    float cardW = 240.f, cardH = 340.f;
    float gap = 36.f;
    float totalW = 3 * cardW + 2 * gap;
    float startX = (WIN_W - totalW) / 2.f;
    float cardY = 120.f;

    const char* mapNames[] = { "Forest Path", "Desert Road", "Mountain Trail" };
    const char* mapDescs[] = {
        "Simple L-shaped path.\n\nEnemies turn once.\nGood for beginners.\n\nMap 1 of 3",
        "S-shaped road.\n\nEnemies turn twice.\nMedium difficulty.\n\nMap 2 of 3",
        "Zigzag trail.\n\nEnemies turn 3 times.\nMore coverage needed.\n\nMap 3 of 3"
    };
    sf::Color cardColors[] = {
        sf::Color(45, 75, 55),
        sf::Color(90, 58, 28),
        sf::Color(40, 50, 90)
    };
    sf::Color borderColors[] = {
        sf::Color(80, 180, 100),
        sf::Color(220, 140, 60),
        sf::Color(100, 130, 220)
    };

    for (int i = 0; i < 3; ++i) {
        float cx = startX + i * (cardW + gap);
        bool  sel = (selectedMap == i);
        sf::RectangleShape card({ cardW, cardH });
        card.setPosition(cx, cardY);
        card.setFillColor(sf::Color(
            cardColors[i].r / (sel ? 1 : 2),
            cardColors[i].g / (sel ? 1 : 2),
            cardColors[i].b / (sel ? 1 : 2), 220));
        card.setOutlineColor(sel ? borderColors[i] : sf::Color(60, 60, 80));
        card.setOutlineThickness(sel ? 3.f : 1.5f);
        window.draw(card);

        
        char nbuf[4]; std::snprintf(nbuf, sizeof(nbuf), "%d", i + 1);
        sf::CircleShape badge(24.f);
        badge.setFillColor(borderColors[i]);
        badge.setPosition(cx + cardW / 2.f - 24.f, cardY + 14.f);
        window.draw(badge);

        sf::Text numT; numT.setFont(font); numT.setString(nbuf);
        numT.setCharacterSize(26); numT.setFillColor(sf::Color::White);
        numT.setPosition(cx + cardW / 2.f - 9.f, cardY + 18.f);
        window.draw(numT);

        sf::Text nameT; nameT.setFont(font); nameT.setString(mapNames[i]);
        nameT.setCharacterSize(17);
        nameT.setFillColor(sel ? sf::Color(240, 240, 160) : sf::Color(200, 200, 200));
        nameT.setPosition(cx + 14.f, cardY + 72.f);
        window.draw(nameT);

        MapDef md = getMap(i);//small map preview
        float scale = 3.6f;
        float ox = cx + 18.f, oy = cardY + 105.f;
        for (int p = 0; p + 1 < md.pathLen; ++p)
        {
            float ax = ox + md.wpCol[p] * scale;
            float ay = oy + md.wpRow[p] * scale;
            float bx = ox + md.wpCol[p + 1] * scale;
            float by = oy + md.wpRow[p + 1] * scale;
            float dx = bx - ax, dy = by - ay;
            float len = sqrt(dx * dx + dy * dy);
            if (len < 0.1f) 
                continue;
            RectangleShape line({ len, 4.f });
            line.setFillColor(sf::Color(
                (sf::Uint8)std::min(255, cardColors[i].r + 100),
                (sf::Uint8)std::min(255, cardColors[i].g + 100),
                (sf::Uint8)std::min(255, cardColors[i].b + 100), 210));
            line.setPosition(ax, ay);
            line.setRotation(std::atan2(dy, dx) * 180.f / 3.14159f);
            window.draw(line);
        }
       
        sf::CircleShape entryDot(5.f);
        entryDot.setFillColor(sf::Color(80, 220, 100));
        entryDot.setPosition(ox + md.wpCol[0] * scale - 5.f,
            oy + md.wpRow[0] * scale - 5.f);
        window.draw(entryDot);
        sf::CircleShape exitDot(5.f);
        exitDot.setFillColor(sf::Color(220, 60, 60));
        exitDot.setPosition(ox + md.wpCol[md.pathLen - 1] * scale - 5.f,
            oy + md.wpRow[md.pathLen - 1] * scale - 5.f);
        window.draw(exitDot);

        sf::Text descT; descT.setFont(font); descT.setString(mapDescs[i]);
        descT.setCharacterSize(12);
        descT.setFillColor(sf::Color(160, 160, 160));
        descT.setPosition(cx + 14.f, cardY + 230.f);
        window.draw(descT);

        if (sel) 
        {
            RectangleShape selTag({ 110.f, 24.f });
            selTag.setFillColor(borderColors[i]);
            selTag.setPosition(cx + cardW / 2.f - 55.f, cardY + cardH - 32.f);
            window.draw(selTag);
            Text selT; selT.setFont(font); selT.setString("SELECTED");
            selT.setCharacterSize(13); selT.setFillColor(sf::Color::White);
            selT.setPosition(cx + cardW / 2.f - 43.f, cardY + cardH - 28.f);
            window.draw(selT);
        }
    }

    // start game button
    float btnW = 220.f, btnH = 52.f;
    float btnX = WIN_W / 2.f - btnW / 2.f;
    float btnY = cardY + cardH + 24.f;
    RectangleShape startBtn({ btnW, btnH });
    startBtn.setPosition(btnX, btnY);
    startBtn.setFillColor(sf::Color(30, 150, 60, 230));
    startBtn.setOutlineColor(sf::Color(80, 255, 120));
    startBtn.setOutlineThickness(2.5f);
    window.draw(startBtn);

    Text startT; 
    startT.setFont(font);
    startT.setString("  START GAME  [Enter]");
    startT.setCharacterSize(17);
    startT.setFillColor(sf::Color(180, 255, 180));
    startT.setPosition(btnX + 14.f, btnY + 13.f);
    window.draw(startT);

    // high score preview on map select screen 
    float hsX = 20.f, hsY = btnY + btnH + 18.f;
    Text hsTitle; hsTitle.setFont(font);
    hsTitle.setString("TOP SCORES:");
    hsTitle.setCharacterSize(13);
    hsTitle.setFillColor(sf::Color(200, 200, 100));
    hsTitle.setPosition(hsX, hsY);
    window.draw(hsTitle);

    for (int i = 0; i < highScore.getCount() && i < 3; ++i) 
    {
        char buf[48];
        snprintf(buf, sizeof(buf), "%d. %-10s %d",
            i + 1, highScore.getName(i), highScore.getScore(i));
        Text ht; ht.setFont(font); ht.setString(buf);
        ht.setCharacterSize(12);
        ht.setFillColor(sf::Color(180, 180, 140));
        ht.setPosition(hsX, hsY + 18.f + i * 16.f);
        window.draw(ht);
    }

    // Keyboard hint
    Text hint; hint.setFont(font);
    hint.setString("1/2/3 or click a card to select  |  Enter to start");
    hint.setCharacterSize(12);
    hint.setFillColor(sf::Color(100, 100, 120));
    hint.setPosition(WIN_W / 2.f - 230.f, (float)WIN_H - 22.f);
    window.draw(hint);
}
void Game::handleMapSelectClick(int mx, int my) 
{
    float cardW = 240.f, cardH = 340.f;
    float gap = 36.f;
    float totalW = 3 * cardW + 2 * gap;
    float startX = (WIN_W - totalW) / 2.f;
    float cardY = 120.f;

    for (int i = 0; i < 3; ++i) {
        float cx = startX + i * (cardW + gap);
        if (mx >= (int)cx && mx <= (int)(cx + cardW) && my >= (int)cardY && my <= (int)(cardY + cardH)) 
        {
            selectedMap = i;
            return;
        }
    }

    float btnW = 220.f, btnH = 52.f;
    float btnX = WIN_W / 2.f - btnW / 2.f;
    float btnY = cardY + cardH + 24.f;
    if (mx >= (int)btnX && mx <= (int)(btnX + btnW) &&
        my >= (int)btnY && my <= (int)(btnY + btnH)) 
    {
        currentMapDef = getMap(selectedMap);
        buildPath();
        state = GameState::PREP;
    }
}

void Game::drawBackground() {
    const MapTheme& t = currentMapDef.theme;

    RectangleShape mapBg({ (float)MAP_W, (float)WIN_H });
    mapBg.setFillColor(t.bgMap);
    window.draw(mapBg);

    RectangleShape panel({ (float)UI_W, (float)WIN_H });
    panel.setPosition((float)MAP_W, 0.f);
    panel.setFillColor(t.bgPanel);
    window.draw(panel);

    RectangleShape border({ 2.f, (float)WIN_H });
    border.setPosition((float)MAP_W, 0.f);
    border.setFillColor(t.borderLine);
    window.draw(border);
}

void Game::drawGrid() {
    const MapTheme& t = currentMapDef.theme;
    for (int c = 0; c <= COLS; ++c) {
        RectangleShape line({ 1.f, (float)WIN_H });
        line.setPosition((float)(c * CELL), 0.f);
        line.setFillColor(t.gridLine);
        window.draw(line);
    }
    for (int r = 0; r <= ROWS; ++r) {
        RectangleShape line({ (float)MAP_W, 1.f });
        line.setPosition(0.f, (float)(r * CELL));
        line.setFillColor(t.gridLine);
        window.draw(line);
    }

    if (hoveredCol >= 0 && hoveredRow >= 0 && selectedTower != 0) 
    {
        bool blocked = pathCell[hoveredRow][hoveredCol];
        RectangleShape hover({ (float)CELL - 2.f, (float)CELL - 2.f });
        hover.setPosition((float)(hoveredCol * CELL + 1), (float)(hoveredRow * CELL + 1));
        hover.setFillColor(blocked? Color(200, 40, 40, 60): Color(80, 200, 120, 60));
        hover.setOutlineColor(blocked? Color(255, 60, 60, 180): Color(100, 255, 140, 180));
        hover.setOutlineThickness(1.5f);
        window.draw(hover);
    }
}

void Game::drawPath() {
    const MapTheme& th = currentMapDef.theme;

    for (int i = 0; i + 1 < pathLen; ++i) 
    {
        Vector2f a = path[i], b = path[i + 1];
        float dx = b.x - a.x, dy = b.y - a.y;
        float len = sqrt(dx * dx + dy * dy);
        int steps = (int)(len / 8.f) + 1;
        for (int s = 0; s <= steps; ++s) 
        {
            float t = (float)s / steps;
            float px = a.x + dx * t, py = a.y + dy * t;
            RectangleShape tile({ (float)CELL - 4.f, (float)CELL - 4.f });
            tile.setPosition((float)(((int)(px / CELL)) * CELL + 2),(float)(((int)(py / CELL)) * CELL + 2));
            tile.setFillColor(th.pathTile);
            window.draw(tile);
        }
    }
    CircleShape entry(10.f);
    entry.setFillColor(sf::Color(80, 220, 100));
    entry.setPosition(path[0].x + CELL / 2.f - 10.f, path[0].y + CELL / 2.f - 10.f);
    window.draw(entry);
                                                          
    CircleShape exitM(10.f);
    exitM.setFillColor(sf::Color(220, 60, 60));
    exitM.setPosition(path[pathLen - 1].x + CELL / 2.f - 10.f,
        path[pathLen - 1].y + CELL / 2.f - 10.f);
    window.draw(exitM);
}

void Game::drawEnemies() 
{
    for (int i = 0; i < enemyCount; ++i)
        if (enemies[i] && enemies[i]->isAlive())
            enemies[i]->render(&window);
}

void Game::drawTowers() {
    for (int j = 0; j < towerCount; ++j) 
    {
        if (!towers[j] || !towers[j]->isAlive()) 
            continue;
        towers[j]->renderRange(window);
        towers[j]->render(&window);
    }
}
void Game::drawHUD() {
    char  buf[64];
    float ux = (float)UI_X + 10.f;
    float uy = 18.f;
    float lineH = 28.f;

    auto drawText = [&](const char* str, float x, float y,unsigned size, Color col)
        {
            Text t; t.setFont(font); t.setString(str);
            t.setCharacterSize(size); t.setFillColor(col);
            t.setPosition(x, y); window.draw(t);
        };

    drawText("TOWER DEFENSE", ux, uy, 16, sf::Color(100, 200, 255)); uy += 20.f;
    drawText(currentMapDef.theme.name, ux, uy, 11, sf::Color(180, 160, 120)); uy += 22.f;

    std::snprintf(buf, sizeof(buf), "Wave: %d / %d", currentWave, totalWaves);
    drawText(buf, ux, uy, 15, sf::Color(220, 220, 100)); uy += lineH;

    std::snprintf(buf, sizeof(buf), "Gold:  %d", gold);
    drawText(buf, ux, uy, 15, sf::Color(255, 210, 50)); uy += lineH;

    std::snprintf(buf, sizeof(buf), "Lives: %d", lives);
    drawText(buf, ux, uy, 15,
        lives <= 3 ? sf::Color(255, 80, 80) : sf::Color(100, 255, 100));
    uy += lineH;

    int alive = 0;
    for (int i = 0; i < enemyCount; ++i)
        if (enemies[i] && enemies[i]->isAlive()) ++alive;
    std::snprintf(buf, sizeof(buf), "Enemies: %d+%d", alive, enemiesToSpawn);
    drawText(buf, ux, uy, 12, sf::Color(200, 160, 160)); uy += lineH + 4.f;

    sf::RectangleShape div({ (float)(UI_W - 20), 1.f });
    div.setPosition(ux, uy); div.setFillColor(sf::Color(60, 80, 100));
    window.draw(div); uy += 8.f;

    drawText("SELECT TOWER (key/click)", ux, uy, 10, sf::Color(140, 170, 200));
    uy += 18.f;

    drawTowerButtons();

    if (state == GameState::PREP) {
        drawText("Place towers then", ux, (float)WIN_H - 96.f, 12, sf::Color(180, 255, 180));
        drawText("click START WAVE.", ux, (float)WIN_H - 78.f, 12, sf::Color(180, 255, 180));
    }
}

void Game::drawTowerButtons() {
    struct TBtn { char key; const char* label; int cost; Color col; };
    static const TBtn btns[] = {
        { 'C', "[C] Cannon $100\nHigh DMG / Slow", 100, Color(200,120,30) },
        { 'S', "[S] Sniper $120\nLong range / Precise", 120, Color(40,140,200) },
        { 'M', "[M] MachGun $150\nRapid fire / Swarms", 150, Color(160,30,30) },
        { 'W', "[W] SlowFld $130\nAoE slow pulse", 130, Color(30,80,200) },
        { 'B', "[B] Bomb $180\nSplash AoE damage", 180, Color(60,60,60)},
    };

    float bx = (float)UI_X + 10.f;
    float by = 210.f;
    float bw = (float)(UI_W - 20);
    float bh = 46.f;
    float gap = 6.f;

    for (int i = 0; i < 5; ++i) {
        RectangleShape btn({ bw, bh });
        btn.setPosition(bx, by);
        bool sel = (selectedTower == btns[i].key);
        bool can = (gold >= btns[i].cost);
        btn.setFillColor(sel? Color(btns[i].col.r, btns[i].col.g, btns[i].col.b, 220):Color(btns[i].col.r / 3, btns[i].col.g / 3, btns[i].col.b / 3, 180));
        btn.setOutlineColor(can ? btns[i].col : Color(80, 80, 80));
        btn.setOutlineThickness(sel ? 2.5f : 1.f);
        window.draw(btn);

        Text t; t.setFont(font); t.setString(btns[i].label);
        t.setCharacterSize(11);
        t.setFillColor(can ? Color(240, 220, 180) : Color(120, 100, 80));
        t.setPosition(bx + 6.f, by + 4.f);
        window.draw(t);

        by += bh + gap;
    }

    // START WAVE button
    by += 6.f;
    if (state == GameState::PREP || state == GameState::WAVE_CLEAR) {
        RectangleShape btn({ bw, 40.f });
        btn.setPosition(bx, by);
        btn.setFillColor(Color(30, 150, 60, 220));
        btn.setOutlineColor(Color(80, 255, 120));
        btn.setOutlineThickness(2.f);
        window.draw(btn);

        Text t; t.setFont(font);
        t.setString("  START WAVE [Space]");
        t.setCharacterSize(13);
        t.setFillColor(Color(180, 255, 180));
        t.setPosition(bx + 6.f, by + 10.f);
        window.draw(t);
    }
}

void Game::drawHighScoreTable(float x, float y) {
    Text title; title.setFont(font);
    title.setString("HIGH SCORES");
    title.setCharacterSize(20);
    title.setFillColor(Color(255, 220, 50));
    title.setPosition(x, y);
    window.draw(title);

    for (int i = 0; i < highScore.getCount(); ++i) {
        char buf[48];
        std::snprintf(buf, sizeof(buf), "%d. %-10s %d",
            i + 1, highScore.getName(i), highScore.getScore(i));
        Text ht; ht.setFont(font); ht.setString(buf);
        // Highlight the most recently added score
        bool isNew = (highScore.getScore(i) == lastScore);
        ht.setCharacterSize(15);
        ht.setFillColor(isNew ? Color(100, 255, 100) : Color(200, 200, 200));
        ht.setPosition(x, y + 28.f + i * 22.f);
        window.draw(ht);
    }
}

void Game::drawOverlay() {
    if (state != GameState::GAME_OVER && state != GameState::WIN) return;

    RectangleShape dim({ (float)WIN_W, (float)WIN_H });
    dim.setFillColor(Color(0, 0, 0, 170));
    window.draw(dim);

    // Big title game over
    Text title; title.setFont(font); title.setCharacterSize(52);
    if (state == GameState::GAME_OVER) 
    {
        title.setString("GAME OVER");
        title.setFillColor(Color(255, 60, 60));
        title.setPosition(195.f, 80.f);
    }
    else 
    {
        title.setString("YOU WIN!");
        title.setFillColor(Color(80, 255, 120));
        title.setPosition(240.f, 80.f);
    }
    window.draw(title);

    char buf[64];
    snprintf(buf, sizeof(buf),
        "Score: %d   (Wave %d  Gold %d  Lives %d)",
        lastScore, currentWave - 1, gold, lives);
    Text scoreLine; scoreLine.setFont(font);
    scoreLine.setString(buf);
    scoreLine.setCharacterSize(16);
    scoreLine.setFillColor(Color(220, 220, 100));
    scoreLine.setPosition(140.f, 152.f);
    window.draw(scoreLine);

    Text formula; formula.setFont(font);
    formula.setString("Score = (wavesCompleted x 200) + goldLeft + (livesLeft x 100)");
    formula.setCharacterSize(12);
    formula.setFillColor(Color(150, 150, 150));
    formula.setPosition(140.f, 176.f);
    window.draw(formula);

    drawHighScoreTable(160.f, 215.f);

    Text hint; hint.setFont(font);
    hint.setString("Press  R  to return to map select   |   Esc to quit");
    hint.setCharacterSize(15);
    hint.setFillColor(Color(160, 160, 160));
    hint.setPosition(190.f, 430.f);
    window.draw(hint);
}

void Game::handleClick(int mx, int my)
{
    if (mx >= MAP_W) {
        float bx = (float)UI_X + 10.f;
        float bw = (float)(UI_W - 20);
        char keys[] = { 'C','S','M','W','B' };
        for (int i = 0; i < 5; ++i) {
            float by = 210.f + i * 52.f;
            if (mx >= (int)bx && mx <= (int)(bx + bw) &&
                my >= (int)by && my <= (int)(by + 46.f)) {
                selectedTower = (selectedTower == keys[i]) ? 0 : keys[i];
                return;
            }
        }
 //start game button
        float startY = 210.f + 5 * 52.f + 6.f;
        if (mx >= (int)bx && mx <= (int)(bx + bw) &&
            my >= (int)startY && my <= (int)(startY + 40.f)) {
            if (state == GameState::PREP || state == GameState::WAVE_CLEAR) {
                ++currentWave;
                if (currentWave > totalWaves) {
                    state = GameState::WIN;
                    recordScore();
                    return;
                }
                enemiesToSpawn = 5 + (currentWave - 1) * 3;
                spawnTimer = 0.f;
                state = GameState::PLAYING;
            }
        }
        return;
    }
    if (selectedTower != 0 && my < WIN_H) {
        int col = mx / CELL, row = my / CELL;
        if (col < COLS && row < ROWS) placeTower(col, row);
    }
}

void Game::placeTower(int col, int row) 
{
    if (pathCell[row][col]) return;     // cant place on path
    if (towerCount >= MAX_TOWERS) return;

    float cx = (float)(col * CELL) + 2.f;
    float cy = (float)(row * CELL) + 2.f;

    //cant place towers on same cell
    for (int j = 0; j < towerCount; ++j) 
    {
        if (!towers[j]) continue;
        if ((int)(towers[j]->getX() / CELL) == col &&
            (int)(towers[j]->getY() / CELL) == row) return;
    }

    Tower* t = nullptr;
    if (selectedTower == 'C' && gold >= 100) { t = new CannonTower(cx, cy); gold -= 100; }
    else if (selectedTower == 'S' && gold >= 120) { t = new SniperTower(cx, cy); gold -= 120; }
    else if (selectedTower == 'M' && gold >= 150) { t = new MachineGunTower(cx, cy); gold -= 150; }
    else if (selectedTower == 'W' && gold >= 130) { t = new SlowTower(cx, cy); gold -= 130; }
    else if (selectedTower == 'B' && gold >= 180) { t = new BombTower(cx, cy); gold -= 180; }

    if (t)
    {
        towers[towerCount++] = t;
        sndTowerPlace.play();     
    }
}

void Game::run() {
    g_sndCannon = &sndCannon;        
    g_sndSniper = &sndSniper;       
    g_sndMachine = &sndMachine;       
    g_sndSlowBomb = &sndSlowBomb;      
    g_sndEnemyFire = &sndEnemyFiring; 

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;  

        Event event;
        while (window.pollEvent(event)) {

            if (event.type == Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                auto kc = event.key.code;

                if (kc == Keyboard::Escape) window.close();

                if (kc == Keyboard::R &&
                    (state == GameState::GAME_OVER || state == GameState::WIN))
                {
                    for (int i = 0; i < MAX_ENEMIES; ++i) { delete enemies[i]; enemies[i] = nullptr; }
                    for (int i = 0; i < MAX_TOWERS; ++i) { delete towers[i];  towers[i] = nullptr; }
                    enemyCount = 0; towerCount = 0;
                    gold = 250; lives = 15;
                    currentWave = 0; enemiesToSpawn = 0; spawnTimer = 0.f;
                    selectedTower = 0; lastScore = 0;
                    state = GameState::MAP_SELECT;
                    continue;
                }
                if (state == GameState::MAP_SELECT) 
                {
                    if (kc == Keyboard::Num1) selectedMap = 0;
                    if (kc == Keyboard::Num2) selectedMap = 1;
                    if (kc == Keyboard::Num3) selectedMap = 2;
                    if (kc == Keyboard::Enter || kc == Keyboard::Return) 
                    {
                        currentMapDef = getMap(selectedMap);
                        buildPath();
                        state = GameState::PREP;
                    }
                    continue;
                }

               
                if (kc == Keyboard::C) selectedTower = (selectedTower == 'C') ? 0 : 'C';
                if (kc == Keyboard::S) selectedTower = (selectedTower == 'S') ? 0 : 'S';
                if (kc == Keyboard::M) selectedTower = (selectedTower == 'M') ? 0 : 'M';
                if (kc == Keyboard::W) selectedTower = (selectedTower == 'W') ? 0 : 'W';
                if (kc == Keyboard::B) selectedTower = (selectedTower == 'B') ? 0 : 'B';

               
                if (kc == Keyboard::Space) 
                {
                    if (state == GameState::PREP || state == GameState::WAVE_CLEAR) 
                    {
                        ++currentWave;
                        if (currentWave > totalWaves) 
                        {
                            state = GameState::WIN;
                            recordScore();
                        }
                        else 
                        {
                            enemiesToSpawn = 5 + (currentWave - 1) * 3;
                            spawnTimer = 0.f;
                            state = GameState::PLAYING;
                        }
                    }
                }
            }

            if (event.type == Event::MouseButtonPressed &&
                event.mouseButton.button == Mouse::Left)
            {
                if (state == GameState::MAP_SELECT)
                    handleMapSelectClick(event.mouseButton.x, event.mouseButton.y);
                else
                    handleClick(event.mouseButton.x, event.mouseButton.y);
            }

            if (event.type == Event::MouseMoved) 
            {
                hoveredCol = event.mouseMove.x / CELL;
                hoveredRow = event.mouseMove.y / CELL;
                if (hoveredCol >= COLS || hoveredRow >= ROWS ||
                    event.mouseMove.x >= MAP_W)
                    hoveredCol = hoveredRow = -1;
            }
        }

        if (state == GameState::PLAYING)
        {
          //spawn enemies 
            if (enemiesToSpawn > 0) 
            {
                spawnTimer -= dt;
                if (spawnTimer <= 0.f) 
                {
                    spawnEnemy();
                    --enemiesToSpawn;
                    float interval = 1.2f - (currentWave - 1) * 0.08f;
                    if (interval < 0.35f) interval = 0.35f;
                    spawnTimer = interval;
                }
            }

            updateEnemies(dt);
            enemiesAttackTowers(dt);
            updateTowers(dt);
            cleanupDead();

            if (state == GameState::PLAYING && allEnemiesDone()) {
                if (currentWave >= totalWaves) {
                    state = GameState::WIN;
                    recordScore();
                }
                else {
                    state = GameState::WAVE_CLEAR;
                }
            }
        }

        window.clear(Color(10, 16, 22));

        if (state == GameState::MAP_SELECT) {
            drawMapSelect();
        }
        else {
            drawBackground();
            drawPath();
            drawGrid();
            drawTowers();
            drawEnemies();
            drawHUD();
            drawOverlay();
        }

        window.display();
    }
}