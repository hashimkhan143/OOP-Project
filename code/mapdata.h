#pragma once
#include <SFML/Graphics.hpp>

struct MapTheme {
    sf::Color bgMap;       
    sf::Color bgPanel;     
    sf::Color pathTile;     
    sf::Color gridLine;   
    sf::Color borderLine;   
    const char* name;
    const char* description;
};
static const int MAP_COLS = 16;
static const int MAP_ROWS = 15;
static const int MAP_CELL = 52;
static const int MAX_PATH_WP = 20;
struct MapDef {
    MapTheme   theme;
    int        pathLen;
    int        wpCol[MAX_PATH_WP];
    int        wpRow[MAX_PATH_WP];
};

inline MapDef getMap(int mapIndex) {
    MapDef m{};
    switch (mapIndex) {

    default:
    case 0:
        m.theme = {
            sf::Color(18, 32, 28),      
            sf::Color(15, 20, 30),     
            sf::Color(45, 75, 55),      
            sf::Color(255,255,255,12),  
            sf::Color(60, 80,100),     
            "Forest Path",
            "Simple path through the forest."
        };
        m.pathLen = 4;
        {
            int c[] = { 0, 7, 7, 15 };
            int r[] = { 2, 2, 12, 12 };
            for (int i = 0; i < m.pathLen; ++i) {
                m.wpCol[i] = c[i];
                m.wpRow[i] = r[i];
            }
        }
        break;
    case 1:
        m.theme = {
            sf::Color(35, 25, 10),   
            sf::Color(25, 15, 8),      
            sf::Color(160, 120, 50),  
            sf::Color(255,220,100,10), 
            sf::Color(160, 100, 40),  
            "Desert Road",
            "S-shaped road through the desert."
        };
        m.pathLen = 6;
        {
            int c[] = { 0, 14, 14,  1,  1, 15 };
            int r[] = { 1,  1,  7,  7, 13, 13 };
            for (int i = 0; i < m.pathLen; ++i) {
                m.wpCol[i] = c[i];
                m.wpRow[i] = r[i];
            }
        }
        break;

    case 2:
        m.theme = {
            sf::Color(20, 22, 35),    
            sf::Color(12, 12, 25),    
            sf::Color(70, 80, 100),    
            sf::Color(200,200,255,10),
            sf::Color(80, 80, 140),    
            "Mountain Trail",
            "Zigzag path down the mountain."
        };
       
        m.pathLen = 8;
        {
            int c[] = { 0, 13, 13,  2,  2, 13, 13, 15 };
            int r[] = { 1,  1,  5,  5,  9,  9, 13, 13 };
            for (int i = 0; i < m.pathLen; ++i) {
                m.wpCol[i] = c[i];
                m.wpRow[i] = r[i];
            }
        }
        break;
    }
    return m;
}