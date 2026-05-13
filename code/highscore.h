#pragma once
#include <fstream>
#include <string>
#include <cstring>

//score = (wavesCompleted * 200) + (goldRemaining) + (lives * 100)

struct ScoreEntry {
    char  name[16]; 
    int   score;
};
static const int MAX_SCORES = 5;

class HighScore {
    ScoreEntry entries[MAX_SCORES];
    int        count;

public:
    HighScore() : count(0) {
        std::memset(entries, 0, sizeof(entries));
        load();
    }
    static int calcScore(int wavesCompleted, int goldLeft, int livesLeft) {
        return (wavesCompleted * 200) + goldLeft + (livesLeft * 100);
    }

    void addScore(const char* name, int score) {
        if (count < MAX_SCORES) {
            strncpy_s(entries[count].name, name, 15);
            entries[count].name[15] = '\0';
            entries[count].score = score;
            ++count;
        }
        else {
            int minIdx = 0;
            for (int i = 1; i < MAX_SCORES; ++i)
                if (entries[i].score < entries[minIdx].score) minIdx = i;
            if (score > entries[minIdx].score) 
            {
                strncpy_s(entries[minIdx].name, name, 15);
                entries[minIdx].name[15] = '\0';
                entries[minIdx].score = score;
            }
        }
        for (int i = 0; i < count - 1; ++i)
            for (int j = i + 1; j < count; ++j)
                if (entries[j].score > entries[i].score) 
                {
                    ScoreEntry tmp = entries[i];
                    entries[i] = entries[j];
                    entries[j] = tmp;
                }
        save();
    }
    bool isHighScore(int score) const {
        if (count < MAX_SCORES) return true;
        for (int i = 0; i < count; ++i)
            if (score > entries[i].score) return true;
        return false;
    }

    int getCount() const
    { return count; }
    const char* getName(int i) const
    { return entries[i].name; }
    int getScore(int i) const 
    { return entries[i].score; }

    void save() const {
        std::ofstream f("highscores.txt");
        if (!f) return;
        f << count << "\n";
        for (int i = 0; i < count; ++i)
            f << entries[i].name << " " << entries[i].score << "\n";
    }

    void load() 
    {
        std::ifstream f("highscores.txt");
        if (!f) return;
        f >> count;
        if (count > MAX_SCORES) count = MAX_SCORES;
        for (int i = 0; i < count; ++i) {
            f >> entries[i].name >> entries[i].score;
        }
    }
};