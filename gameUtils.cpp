#include "gameUtils.h"

void drawTransparentImage(cv::Mat& frame, const cv::Mat& img, int xPos, int yPos) {
    int startX = std::max(0, xPos);
    int startY = std::max(0, yPos);
    int endX = std::min(frame.cols, xPos + img.cols);
    int endY = std::min(frame.rows, yPos + img.rows);

    if (startX >= endX || startY >= endY) return;

    cv::Rect roiDest(startX, startY, endX - startX, endY - startY);
    cv::Rect roiSrc(std::max(0, -xPos), std::max(0, -yPos), roiDest.width, roiDest.height);

    cv::Mat imgCropped = img(roiSrc);

    if (imgCropped.channels() == 4) {
        cv::Mat mask;
        std::vector<cv::Mat> layers;
        cv::split(imgCropped, layers);
        cv::Mat rgb[3] = { layers[0], layers[1], layers[2] };
        mask = layers[3];
        cv::merge(rgb, 3, imgCropped);
        imgCropped.copyTo(frame(roiDest), mask);
    } else {
        imgCropped.copyTo(frame(roiDest));
    }
}

bool detectGreenObject(const cv::Mat& frame, cv::Point2f& center, int& totalPoints,
                      const cv::Scalar& lowerGreen, const cv::Scalar& upperGreen) {
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