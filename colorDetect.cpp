#include "opencv2/opencv.hpp"
#include <iostream>

using namespace cv;
using namespace std;

int main() {
    // Abrir a webcam
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cout << "Erro ao acessar a câmera!" << endl;
        return -1;
    }

    // faixa de cor pra detecção (vermelho)
    Scalar lowerColor(0, 150, 50);
    Scalar upperColor(10, 255, 255);

    // Criar uma janela para exibir apenas o resultado da detecção de cor
    namedWindow("Cor Detectada", WINDOW_AUTOSIZE);

    while (true) {
        Mat frame, hsvFrame, mask, result;

        // Capturar o quadro da webcam
        cap >> frame;
        if (frame.empty()) break; // Verificar se o quadro está vazio (fim do vídeo ou erro)

        // Converter o quadro para o espaço de cor HSV
        cvtColor(frame, hsvFrame, COLOR_BGR2HSV);

        // Criar uma máscara para os pixels dentro da faixa de cor
        inRange(hsvFrame, lowerColor, upperColor, mask);

        // Encontrar contornos na máscara
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        // Calcular o ponto médio entre as áreas detectadas
        if (!contours.empty()) {
            Point2f center(0, 0);
            int totalPoints = 0;

            for (const auto& contour : contours) {
                for (const auto& point : contour) {
                    center += Point2f(point);
                    totalPoints++;
                }
            }

            center.x /= totalPoints;
            center.y /= totalPoints;

            // Desenhar o círculo no ponto médio
            circle(frame, center, 20, Scalar(0, 255, 0), 2);
        }

        // Aplicar a máscara ao quadro original para mostrar apenas as cores detectadas
        bitwise_and(frame, frame, result, mask);

        // Exibir o resultado
        imshow("Cor Detectada", frame);

        // Sair do loop ao pressionar a tecla 'ESC'
        char key = (char)waitKey(30);
        if (key == 27) break; // Código ASCII para 'ESC'
    }

    // Liberar os recursos
    cap.release();
    destroyAllWindows(); // Fechar todas as janelas

    return 0;
}