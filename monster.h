#ifndef MONSTER_H
#define MONSTER_H

#include <opencv2/opencv.hpp>

class Monster {
private:
    cv::Point position;
    cv::Point velocity;
    bool alive;
    bool markedForRemoval;
    cv::Mat image;

public:
    Monster(const cv::Mat& img, const cv::Point& pos, const cv::Point& vel);
    
    void update(const cv::Size& frameSize);
    void draw(cv::Mat& frame) const;
    bool checkCollision(const cv::Rect& area) const;
    bool isAlive() const;
    void kill();
    void markForRemoval();
    bool shouldRemove() const;
    cv::Rect getBoundingBox() const;
};

#endif

























class GameManager {
    private:
        - cv::Mat background;
        - cv::Mat monsterImg;
        - cv::VideoCapture cap;
    
        - unsigned int hordeNumber;
        - unsigned int monstersKilled;
        - unsigned int totalMonstersDefeated;
        - const unsigned int MAX_HORDES;
        - const unsigned int MAX_MONSTERS;
        - const unsigned int FINAL_BOSS_HORDE;
        - std::chrono::steady_clock::time_point roundStartTime;
        - const int ROUND_DURATION_SECONDS;
    
        - unsigned int shotsFired;
        - unsigned int missedShots;
        - const unsigned int MAX_MISSED_SHOTS;
    
        - int hordeSize;
        - int baseSpeed;
        - bool finalBossSpawned;
    
        - cv::Scalar lowerGreen;
        - cv::Scalar upperGreen;
        - cv::Point2f currentAim;
        - bool showCrosshair;
    
        - std::vector<Monster> monsters;
        - bool gameRunning;
        - bool gameWon;
    
    public:
        + GameManager();
        + ~GameManager();
    
        + bool initialize
        + void run
        + bool isGameRunning
        + bool isGameWon
        + unsigned int getHordeNumber
        + unsigned int getMonstersKilled
        + unsigned int getTotalMonstersDefeated
        + unsigned int getMissedShots
        + unsigned int getMaxMissedShots
        + unsigned int getMaxHordes
        + void saveHighScore
        + int loadHighScore
        + void showTransitionScreen
        + bool showMenu
        - void spawnHorde
        - void spawnFinalBoss
        - void processFrame
        - void updateGameState
        - void render
        - void handleInput
        - void drawCrosshair
        - void fireShot
        - void showVictoryScreen;
        - void showDefeatScreen
        - void showHordaInfo
        - bool detectGreenObject
    };
    