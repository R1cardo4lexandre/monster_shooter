#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdlib> // Para o comando system()
#include <random>  // Para gerar movimento aleatório

using namespace cv;
using namespace std;

// Função para desenhar uma imagem transparente sobre um quadro
void drawImage(Mat frame, Mat img, int xPos, int yPos) {
    int startX = max(0, xPos);
    int startY = max(0, yPos);
    int endX = min(frame.cols, xPos + img.cols);
    int endY = min(frame.rows, yPos + img.rows);

    if (startX >= endX || startY >= endY)
        return;

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
    // Carregar o background
    Mat background = imread("space.jpg"); // Substitua pelo caminho para sua imagem de fundo
    if (background.empty()) {
        cout << "Erro ao carregar o background!" << endl;
        return -1;
    }

    // Carregar a imagem interativa (PNG com transparência)
    Mat interactiveImg = imread("meramon.png", IMREAD_UNCHANGED);
    if (interactiveImg.empty()) {
        cout << "Erro ao carregar a imagem interativa!" << endl;
        return -1;
    }

    resize(interactiveImg, interactiveImg, Size(100, 100));

    // Inicializar a captura de vídeo
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cout << "Erro ao acessar a câmera!" << endl;
        return -1;
    }

    // Faixa de cor para detecção (verde)
    Scalar lowerGreen(35, 50, 50);   // Limite inferior (Hue, Saturation, Value)
    Scalar upperGreen(85, 255, 255); // Limite superior (Hue, Saturation, Value)

    Point2f smoothedCenter(0, 0);
    Point2f lastCenter(0, 0);
    float smoothingFactor = 0.2;

    // Variáveis para o movimento do monstrinho
    int x = 50, y = 50; // Posição inicial
    int dx = 5, dy = 5; // Direção e velocidade do movimento
    bool monsterAlive = true; // O monstrinho está vivo

    // Criar a janela para exibição
    namedWindow("Deteccao de Cor com Interacao");

    while (true) {
        Mat frame, hsvFrame, mask;

        cap >> frame;
        if (frame.empty()) break;

        // Espelhar a imagem horizontalmente
        flip(frame, frame, 1); // 1 significa espelhamento horizontal

        cvtColor(frame, hsvFrame, COLOR_BGR2HSV);

        // Criar máscara para os pixels dentro da faixa de verde
        inRange(hsvFrame, lowerGreen, upperGreen, mask);

        // Melhorar a máscara com operações morfológicas
        Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
        morphologyEx(mask, mask, MORPH_CLOSE, kernel);
        morphologyEx(mask, mask, MORPH_OPEN, kernel);

        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

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

            smoothedCenter = smoothedCenter * (1 - smoothingFactor) + center * smoothingFactor;
            lastCenter = smoothedCenter;
        } else {
            smoothedCenter = lastCenter;
        }

        // Criar uma cópia do background para desenhar os elementos
        Mat output = background.clone();

        // Desenhar o círculo no local detectado
        if (totalPoints > 0) {
            circle(output, smoothedCenter, 20, Scalar(0, 255, 0), 2);
        }

        // Atualizar a posição do monstrinho (se ele estiver vivo)
        bool collisionDetected = false;
        if (monsterAlive) {
            x += dx;
            y += dy;

            // Verificar colisão com as bordas da janela
            if (x <= 0 || x + interactiveImg.cols >= output.cols) dx = -dx;
            if (y <= 0 || y + interactiveImg.rows >= output.rows) dy = -dy;

            // Desenhar o monstrinho
            drawImage(output, interactiveImg, x, y);

            // Verificar interseção entre o círculo e o monstrinho
            Rect interactiveRect(x, y, interactiveImg.cols, interactiveImg.rows);
            if (totalPoints > 0) {
                Rect detectionRect(smoothedCenter.x - 20, smoothedCenter.y - 20, 40, 40);
                if ((detectionRect & interactiveRect).area() > 0) {
                    collisionDetected = true;
                    // Retângulo REMOVIDO para evitar exibição
                }
            }
        }

        // Verificar tecla SPACE para reproduzir som e eliminar o monstrinho
        int key = waitKey(30);
        if (key == 27) break; // Tecla ESC para sair

        if (key == 32) { // Tecla SPACE para executar som
            cout << "Tecla SPACE pressionada. Executando som..." << endl;
            system("mplayer tiro.wav &"); // Executa o som (certifique-se de que mplayer está instalado e 'tiro.wav' existe)

            if (collisionDetected && monsterAlive) {
                cout << "Interseção detectada! Monstrinho eliminado!" << endl;
                monsterAlive = false; // Eliminar o monstrinho
            }
        }

        // Exibir o resultado
        imshow("Deteccao de Cor com Interacao", output);
    }

    cap.release();
    destroyAllWindows();

    return 0;
}