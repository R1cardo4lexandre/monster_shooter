#include "opencv2/opencv.hpp"
#include <iostream>

using namespace cv;
using namespace std;

// Função para desenhar uma imagem transparente sobre um quadro
void drawImage(Mat frame, Mat img, int xPos, int yPos) {
    // Calcular as bordas da área onde a imagem será desenhada
    int startX = max(0, xPos);
    int startY = max(0, yPos);
    int endX = min(frame.cols, xPos + img.cols);
    int endY = min(frame.rows, yPos + img.rows);

    // Verificar se a área de desenho está completamente fora da tela
    if (startX >= endX || startY >= endY)
        return;

    // Ajustar a região da imagem interativa para caber na tela
    Rect roiDest(startX, startY, endX - startX, endY - startY);
    Rect roiSrc(max(0, -xPos), max(0, -yPos), roiDest.width, roiDest.height);

    Mat imgCropped = img(roiSrc);

    if (imgCropped.channels() == 4) { // Imagem com transparência
        Mat mask;
        vector<Mat> layers;
        split(imgCropped, layers); // Separa os canais
        Mat rgb[3] = { layers[0], layers[1], layers[2] }; // Canais RGB
        mask = layers[3]; // Canal alfa é a máscara
        merge(rgb, 3, imgCropped); // Junta os canais RGB
        imgCropped.copyTo(frame(roiDest), mask); // Copia com a máscara
    } else { // Imagem sem transparência
        imgCropped.copyTo(frame(roiDest));
    }
}

int main() {
    // Abrir a webcam
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cout << "Erro ao acessar a câmera!" << endl;
        return -1;
    }

    // Carregar a imagem que será usada para interação
    Mat interactiveImg = imread("mob.png", IMREAD_UNCHANGED);
    if (interactiveImg.empty()) {
        cout << "Erro ao carregar a imagem interativa!" << endl;
        return -1;
    }

    // Redimensionar a imagem interativa (se necessário)
    resize(interactiveImg, interactiveImg, Size(100, 100)); // Ajuste de tamanho da imagem

    // Posição fixa da imagem interativa
    int x = 50, y = 50; // Coordenadas do canto superior esquerdo

    // Faixa de cor para detecção (exemplo: vermelho)
    Scalar lowerColor(0, 150, 50);
    Scalar upperColor(10, 255, 255);

    // Loop principal
    while (true) {
        Mat frame, hsvFrame, mask, result;

        // Capturar o quadro da webcam
        cap >> frame;
        if (frame.empty()) break;

        // Converter o quadro para o espaço de cor HSV
        cvtColor(frame, hsvFrame, COLOR_BGR2HSV);

        // Criar uma máscara para os pixels dentro da faixa de cor
        inRange(hsvFrame, lowerColor, upperColor, mask);

        // Encontrar contornos na máscara
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        // Calcular o ponto médio entre as áreas detectadas
        Point2f center(0, 0);
        int totalPoints = 0;
        if (!contours.empty()) {
            for (const auto& contour : contours) {
                for (const auto& point : contour) {
                    center += Point2f(point);
                    totalPoints++;
                }
            }

            center.x /= totalPoints;
            center.y /= totalPoints;

            // Desenhar o círculo no ponto médio
            circle(frame, center, 20, Scalar(0, 255, 0), 2); // Desenha um círculo verde
        }

        // Desenhar a imagem interativa
        drawImage(frame, interactiveImg, x, y);

        // Verificar colisão entre o círculo de detecção de cor e a imagem interativa
        Rect interactiveRect(x, y, interactiveImg.cols, interactiveImg.rows);
        bool collisionDetected = false;

        if (totalPoints > 0) { // Verificar se há algo detectado
            Rect detectionRect(center.x - 20, center.y - 20, 40, 40); // Área do círculo detectado
            if ((detectionRect & interactiveRect).area() > 0) {
                collisionDetected = true;
                rectangle(frame, interactiveRect, Scalar(0, 0, 255), 3); // Destacar a imagem interativa em vermelho
            } else {
                rectangle(frame, interactiveRect, Scalar(255, 0, 0), 3); // Destacar a imagem interativa em azul
            }
        }

        // Verificar a tecla pressionada
        int key = waitKey(30);
        if (key == 27) break; // Sair se 'ESC' for pressionado
        if (collisionDetected && key == 32) { // 32 é o código ASCII para 'Spacebar'
            putText(frame, "tiro", Point(50, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
            system("mplayer tiro.wav &");
        }

        // Exibir o quadro processado
        imshow("Deteccao de Cor com Interacao", frame);
    }

    // Liberar recursos
    cap.release();
    destroyAllWindows();

    return 0;
}