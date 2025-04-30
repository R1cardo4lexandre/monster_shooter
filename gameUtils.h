#ifndef GAMEUTILS_H
#define GAMEUTILS_H

#include <opencv2/opencv.hpp>

void drawTransparentImage(cv::Mat& frame, const cv::Mat& img, int xPos, int yPos);
bool detectGreenObject(const cv::Mat& frame, cv::Point2f& center, int& totalPoints, 
                      const cv::Scalar& lowerGreen, const cv::Scalar& upperGreen);

#endif