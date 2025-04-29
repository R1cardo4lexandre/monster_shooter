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