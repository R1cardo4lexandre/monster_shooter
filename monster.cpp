#include "monster.h"
#include "gameUtils.h"

Monster::Monster(const cv::Mat& img, const cv::Point& pos, const cv::Point& vel) 
    : position(pos), velocity(vel), alive(true), markedForRemoval(false), image(img.clone()) {}

void Monster::update(const cv::Size& frameSize) {
    if (!alive) return;
    
    position += velocity;
    
    if (position.x <= 0 || position.x + image.cols >= frameSize.width) {
        velocity.x *= -1;
    }
    if (position.y <= 0 || position.y + image.rows >= frameSize.height) {
        velocity.y *= -1;
    }
}

void Monster::draw(cv::Mat& frame) const {
    if (alive) {
        drawTransparentImage(frame, image, position.x, position.y);
    }
}

bool Monster::checkCollision(const cv::Rect& area) const {
    return alive && (area & getBoundingBox()).area() > 0;
}

bool Monster::isAlive() const {
    return alive;
}

void Monster::kill() {
    alive = false;
}

void Monster::markForRemoval() {
    markedForRemoval = true;
}

bool Monster::shouldRemove() const {
    return markedForRemoval;
}

cv::Rect Monster::getBoundingBox() const {
    return cv::Rect(position.x, position.y, image.cols, image.rows);
}