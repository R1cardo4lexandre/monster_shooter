#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include "monster.h"

class GameManager {
private:
    cv::Mat background;
    cv::Mat monsterImg;
    cv::VideoCapture cap;
    
    unsigned int hordeNumber;
    unsigned int monstersKilled;
    unsigned int totalMonstersDefeated;
    const unsigned int MAX_HORDES = 10;
    const unsigned int MAX_MONSTERS = 30;
    const unsigned int FINAL_BOSS_HORDE = 10;
    
    unsigned int shotsFired;
    unsigned int missedShots;
    const unsigned int MAX_MISSED_SHOTS = 15;
    
    int hordeSize;
    int baseSpeed;
    bool finalBossSpawned;
    
    cv::Scalar lowerGreen;
    cv::Scalar upperGreen;
    cv::Point2f currentAim;
    bool showCrosshair;
    
    std::vector<Monster> monsters;
    bool gameRunning;
    bool gameWon;

public:
    GameManager();
    ~GameManager();
    
    bool initialize();
    void run();
    bool isGameRunning() const;
    bool isGameWon() const;
    unsigned int getHordeNumber() const;
    unsigned int getMonstersKilled() const;
    unsigned int getTotalMonstersDefeated() const;
    unsigned int getMissedShots() const;
    unsigned int getMaxMissedShots() const;
    unsigned int getMaxHordes() const;
    void saveHighScore();   // Função para salvar o recorde
    int loadHighScore();    // Função para carregar o recorde
    

private:
    void spawnHorde();
    void spawnFinalBoss();
    void processFrame(cv::Mat& frame);
    void updateGameState();
    void render(cv::Mat& output);
    void handleInput(int key);
    void drawCrosshair(cv::Mat& frame);
    void fireShot();
    void showVictoryScreen();
    void showDefeatScreen(const std::string& reason);
    void showHordaInfo();
    bool detectGreenObject(const cv::Mat& frame, cv::Point2f& center, int& totalPoints) const;
};

#endif