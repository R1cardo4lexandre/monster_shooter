#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <random>

using namespace cv;
using namespace std;

void drawImage(Mat frame, Mat img, int xPos, int yPos) {
    int startX = max(0, xPos);
    int startY = max(0, yPos);
    int endX = min(frame.cols, xPos + img.cols);
    int endY = min(frame.rows, yPos + img.rows);
    if (startX >= endX || startY >= endY) return;
    Rect roiDest(startX, startY, endX - startX, endY - startY);
    Rect roiSrc(max(0, -xPos), max(0, -yPos), roiDest.width, roiDest.height);
    Mat imgCropped = img(roiSrc);
    if (imgCropped.channels() == 4) {
        Mat mask;
        vector<Mat> layers;
        split(imgCropped, layers);
        Mat rgb[3] = { layers[0], layers[1], layers[2] };
        mask = layers[3];
        merge(rgb, 3, imgCropped);
        imgCropped.copyTo(frame(roiDest), mask);
    } else {
        imgCropped.copyTo(frame(roiDest));
    }
}

int main() {
    Mat background = imread("space.jpg");
    if (background.empty()) {
        cout << "Erro ao carregar o background!" << endl;
        return -1;
    }

    Mat interactiveImg = imread("meramon.png", IMREAD_UNCHANGED);
    if (interactiveImg.empty()) {
        cout << "Erro ao carregar a imagem interativa!" << endl;
        return -1;
    }

    resize(interactiveImg, interactiveImg, Size(100, 100));
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cout << "Erro ao acessar a câmera!" << endl;
        return -1;
    }

    Scalar lowerGreen(35, 50, 50);
    Scalar upperGreen(85, 255, 255);
    Point2f smoothedCenter(0, 0), lastCenter(0, 0);
    float smoothingFactor = 0.2;
    int x = 50, y = 50, dx = 5, dy = 5;
    bool monsterAlive = true;

    int score = 0;
    int recorde = 0;

    // ✅ Ler recorde do arquivo
    ifstream recFile("recorde.txt");
    if (recFile.is_open()) {
        recFile >> recorde;
        recFile.close();
    }

    namedWindow("Deteccao de Cor com Interacao");

    while (true) {
        Mat frame, hsvFrame, mask;
        cap >> frame;
        if (frame.empty()) break;
        flip(frame, frame, 1);
        cvtColor(frame, hsvFrame, COLOR_BGR2HSV);
        inRange(hsvFrame, lowerGreen, upperGreen, mask);
        Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
        morphologyEx(mask, mask, MORPH_CLOSE, kernel);
        morphologyEx(mask, mask, MORPH_OPEN, kernel);

        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        Point2f center(0, 0);
        int totalPoints = 0;
        if (!contours.empty()) {
            for (const auto& contour : contours)
                for (const auto& point : contour) {
                    center += Point2f(point);
                    totalPoints++;
                }
            center.x /= totalPoints;
            center.y /= totalPoints;
            smoothedCenter = smoothedCenter * (1 - smoothingFactor) + center * smoothingFactor;
            lastCenter = smoothedCenter;
        } else {
            smoothedCenter = lastCenter;
        }

        Mat output = background.clone();
        if (totalPoints > 0) {
            circle(output, smoothedCenter, 20, Scalar(0, 255, 0), 2);
        }

        bool collisionDetected = false;
        if (monsterAlive) {
            x += dx;
            y += dy;
            if (x <= 0 || x + interactiveImg.cols >= output.cols) dx = -dx;
            if (y <= 0 || y + interactiveImg.rows >= output.rows) dy = -dy;
            drawImage(output, interactiveImg, x, y);

            Rect interactiveRect(x, y, interactiveImg.cols, interactiveImg.rows);
            if (totalPoints > 0) {
                Rect detectionRect(smoothedCenter.x - 20, smoothedCenter.y - 20, 40, 40);
                if ((detectionRect & interactiveRect).area() > 0) {
                    collisionDetected = true;
                }
            }
        }

        // Mostrar pontuação e recorde
        putText(output, "Score: " + to_string(score), Point(20, 40),
                FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
        putText(output, "Recorde: " + to_string(recorde), Point(20, 80),
                FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 0), 2);

        int key = waitKey(30);
        if (key == 27) break;

        if (key == 32) {
            cout << "Tecla SPACE pressionada. Executando som..." << endl;
            system("mplayer tiro.wav &");

            if (collisionDetected && monsterAlive) {
                cout << "Interseção detectada! Monstrinho eliminado!" << endl;
                monsterAlive = false;
                score++;  // Aumenta pontuação
            }
        }

        imshow("Deteccao de Cor com Interacao", output);
    }

    cap.release();
    destroyAllWindows();

    ofstream recFileOut("recorde.txt");
    if (recFileOut.is_open()) {
        int maiorPontuacao = max(score, recorde);
        recFileOut << maiorPontuacao;
        recFileOut.close();
        cout << "Recorde salvo em 'recorde.txt': " << maiorPontuacao << endl;
    } else {
        cout << "Erro ao salvar o recorde." << endl;
    }

    return 0;
}
