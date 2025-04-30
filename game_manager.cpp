#include "game_manager.h"
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <ctime>
#include <fstream> 



GameManager::GameManager() : 
    hordeNumber(1), monstersKilled(0), totalMonstersDefeated(0),
    shotsFired(0), missedShots(0),
    hordeSize(3), baseSpeed(3), finalBossSpawned(false),
    lowerGreen(35, 50, 50), upperGreen(85, 255, 255),
    showCrosshair(false), gameRunning(true), gameWon(false) {
    std::srand(std::time(0));
}

GameManager::~GameManager() {
    cap.release();
    cv::destroyAllWindows();
}

bool GameManager::initialize() {
    background = cv::imread("space.jpg");
    if (background.empty()) {
        std::cerr << "Erro ao carregar o background!" << std::endl;
        return false;
    }

    monsterImg = cv::imread("meramon.png", cv::IMREAD_UNCHANGED);
    if (monsterImg.empty()) {
        std::cerr << "Erro ao carregar a imagem do monstro!" << std::endl;
        return false;
    }
    cv::resize(monsterImg, monsterImg, cv::Size(100, 100));

    cap.open(0);
    if (!cap.isOpened()) {
        std::cerr << "Erro ao acessar a câmera!" << std::endl;
        return false;
    }

    spawnHorde();
    cv::namedWindow("Monster Shooter");
    return true;
}

void GameManager::run() {
    cv::Mat frame;
    while (gameRunning && cap.read(frame)) {
        cv::flip(frame, frame, 1);
        processFrame(frame);
        
        cv::Mat output = background.clone();
        render(output);
        
        if (showCrosshair) {
            drawCrosshair(output);
        }
        
        int key = cv::waitKey(30);
        handleInput(key);
        
        cv::imshow("Monster Shooter", output);
    }
}

void GameManager::processFrame(cv::Mat& frame) {
    int totalPoints;
    if (detectGreenObject(frame, currentAim, totalPoints)) {
        showCrosshair = true;
    } else {
        showCrosshair = false;
    }
    updateGameState();
}

bool GameManager::detectGreenObject(const cv::Mat& frame, cv::Point2f& center, int& totalPoints) const {
    cv::Mat hsvFrame, mask;
    cv::cvtColor(frame, hsvFrame, cv::COLOR_BGR2HSV);
    cv::inRange(hsvFrame, lowerGreen, upperGreen, mask);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    center = cv::Point2f(0, 0);
    totalPoints = 0;
    
    if (!contours.empty()) {
        for (const auto& contour : contours) {
            for (const auto& point : contour) {
                center += cv::Point2f(point);
                totalPoints++;
            }
        }
        center.x /= totalPoints;
        center.y /= totalPoints;
        return true;
    }
    return false;
}

void GameManager::spawnHorde() {
    if (hordeNumber == FINAL_BOSS_HORDE) {
        spawnFinalBoss();
        return;
    }

    hordeSize = 3 + (hordeNumber * 2);
    baseSpeed = 3 + (hordeNumber / 2);
    
    for (int i = 0; i < hordeSize; i++) {
        int x = 50 + rand() % (background.cols - 100);
        int y = 50 + rand() % (background.rows - 100);
        int dx = (rand() % (baseSpeed*2 + 1)) - baseSpeed;
        int dy = (rand() % (baseSpeed*2 + 1)) - baseSpeed;
        
        if (dx == 0) dx = 1;
        if (dy == 0) dy = 1;
        
        monsters.emplace_back(monsterImg, cv::Point(x, y), cv::Point(dx, dy));
    }
    showHordaInfo();
}

void GameManager::spawnFinalBoss() {
    finalBossSpawned = true;
    monsters.clear();
    
    cv::Mat bossImg = monsterImg.clone();
    cv::resize(bossImg, bossImg, cv::Size(150, 150));
    
    int x = background.cols / 2 - 75;
    int y = background.rows / 2 - 75;
    monsters.emplace_back(bossImg, cv::Point(x, y), cv::Point(baseSpeed*2, baseSpeed*2));
    
    std::cout << "\n=== HORDA FINAL ===" << std::endl;
    std::cout << "CHEFE FINAL APARECEU!" << std::endl;
}

void GameManager::fireShot() {
    shotsFired++;
    system("mplayer tiro.wav &");
    
    bool hit = false;
    cv::Rect shotArea(currentAim.x - 15, currentAim.y - 15, 30, 30);
    
    for (auto& monster : monsters) {
        if (monster.checkCollision(shotArea)) {
            monster.markForRemoval();
            hit = true;
        }
    }
    
    if (!hit) {
        missedShots++;
        std::cout << "Tiro errado! (" << missedShots << "/" << MAX_MISSED_SHOTS << ")" << std::endl;
    }
}

void GameManager::updateGameState() {
    if (missedShots >= MAX_MISSED_SHOTS) {
        gameRunning = false;
        showDefeatScreen("Muitos tiros errados!");
        return;
    }

    int previousMonsterCount = monsters.size();

    monsters.erase(
        std::remove_if(monsters.begin(), monsters.end(),
            [](const Monster& m) { return m.shouldRemove(); }),
        monsters.end());

    int monstersEliminatedThisFrame = previousMonsterCount - monsters.size();
    monstersKilled += monstersEliminatedThisFrame;  // Incrementa a contagem correta de monstros eliminados
    totalMonstersDefeated += monstersEliminatedThisFrame;

    if (finalBossSpawned && monsters.empty()) {
        gameWon = true;
        gameRunning = false;
        showVictoryScreen();
        return;
    }

    if (monsters.size() >= MAX_MONSTERS) {
        gameRunning = false;
        showDefeatScreen("Muitos monstros vivos!");
        return;
    }

    if (monsters.empty() && !finalBossSpawned) {
        hordeNumber++;
        if (hordeNumber <= MAX_HORDES) {
            spawnHorde();
        } else {
            spawnFinalBoss();
        }
    }
}


void GameManager::render(cv::Mat& output) {
    int highScore = loadHighScore();

    std::string status = "Horda: " + std::to_string(hordeNumber) + 
                       "/" + std::to_string(MAX_HORDES) +
                       " | Monstros: " + std::to_string(monsters.size()) +
                       " | Erros: " + std::to_string(missedShots) +
                       "/" + std::to_string(MAX_MISSED_SHOTS);
                       
                       std::string highScoreText = "Recorde: " + std::to_string(highScore);

    cv::putText(output, status, cv::Point(10, 30), 
               cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);

    cv::putText(output, highScoreText, cv::Point(10, 60), 
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);

    for (auto& monster : monsters) {
        monster.update(output.size());
        monster.draw(output);
    }
}

void GameManager::handleInput(int key) {
    if (key == 27) { // ESC
        gameRunning = false;
    } else if (key == 32) { // SPACE
        if (showCrosshair) {
            fireShot();
        }
    }
}

void GameManager::drawCrosshair(cv::Mat& frame) {
    int centerX = currentAim.x;
    int centerY = currentAim.y;
    int size = 20;
    
    cv::line(frame, 
             cv::Point(centerX - size, centerY),
             cv::Point(centerX + size, centerY),
             cv::Scalar(0, 255, 0), 2);
    
    cv::line(frame,
             cv::Point(centerX, centerY - size),
             cv::Point(centerX, centerY + size),
             cv::Scalar(0, 255, 0), 2);
    
    cv::circle(frame, currentAim, size/2, cv::Scalar(0, 255, 0), 2);
}

void GameManager::showHordaInfo() {
    std::cout << "\n=== HORDA " << hordeNumber << " ===" << std::endl;
    std::cout << "Monstros: " << hordeSize << std::endl;
    std::cout << "Velocidade: " << baseSpeed << std::endl;
    
    if (hordeNumber == MAX_HORDES - 1) {
        std::cout << "ALERTA: Horda final chegando!" << std::endl;
    }
}

void GameManager::showVictoryScreen() {
    cv::Mat screen = background.clone();
    
    cv::putText(screen, "VITORIA TOTAL!", cv::Point(150, 200), 
               cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 255, 255), 4);
    
    std::string stats = "Hordas completadas: " + std::to_string(MAX_HORDES);
    cv::putText(screen, stats, cv::Point(150, 300), 
               cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);
    
    stats = "Monstros eliminados: " + std::to_string(totalMonstersDefeated);
    cv::putText(screen, stats, cv::Point(150, 350), 
               cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);
    
    cv::imshow("Monster Shooter", screen);
    cv::waitKey(5000);

    saveHighScore(); // Salva a pontuação mais alta
}

void GameManager::showDefeatScreen(const std::string& reason) {
    cv::Mat screen = background.clone();
    
    cv::putText(screen, "GAME OVER", cv::Point(150, 200), 
               cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 0, 255), 4);
    
    cv::putText(screen, reason, cv::Point(100, 250), 
               cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 255), 2);
    
    std::string stats = "Hordas completadas: " + std::to_string(hordeNumber - 1);
    cv::putText(screen, stats, cv::Point(150, 300), 
               cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 255), 2);
    
    stats = "Monstros eliminados: " + std::to_string(totalMonstersDefeated);
    cv::putText(screen, stats, cv::Point(150, 350), 
               cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 255), 2);
    
    cv::imshow("Monster Shooter", screen);
    cv::waitKey(5000);

    saveHighScore(); // Salva a pontuação mais alta
}

unsigned int GameManager::getMaxHordes() const { return MAX_HORDES; }
unsigned int GameManager::getMissedShots() const { return missedShots; }
unsigned int GameManager::getMaxMissedShots() const { return MAX_MISSED_SHOTS; }
bool GameManager::isGameRunning() const { return gameRunning; }
bool GameManager::isGameWon() const { return gameWon; }
unsigned int GameManager::getHordeNumber() const { return hordeNumber; }
unsigned int GameManager::getMonstersKilled() const { return monstersKilled; }
unsigned int GameManager::getTotalMonstersDefeated() const {return totalMonstersDefeated; }

void GameManager::saveHighScore() {
    int currentScore = totalMonstersDefeated; // ou qualquer métrica de pontuação que você queira usar
    int highScore = loadHighScore();

    // Verifique se a pontuação atual é maior que o recorde
    if (currentScore > highScore) {
        std::ofstream outFile("recorde.txt");
        if (outFile.is_open()) {
            outFile << currentScore;
            outFile.close();
            std::cout << "Novo recorde! Pontuação salva: " << currentScore << std::endl;
        } else {
            std::cerr << "Erro ao salvar o recorde!" << std::endl;
        }
    }
}

int GameManager::loadHighScore() {
    int highScore = 0;
    std::ifstream inFile("recorde.txt");
    if (inFile.is_open()) {
        inFile >> highScore;
        inFile.close();
    } else {
        std::cout << "Nenhum recorde encontrado, criando um novo arquivo." << std::endl;
    }
    return highScore;
}
