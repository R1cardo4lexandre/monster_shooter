#include "opencv2/opencv.hpp"
#include <iostream>

using namespace cv;
using namespace std;

// desenhar uma imagem transparente sobre um quadro
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
    // Carregar o classificador Haar Cascade para detecção de rostos
    CascadeClassifier faceCascade;
    if (!faceCascade.load("haarcascade_frontalface_default.xml")) {
        cout << "Erro ao carregar o classificador Haar Cascade!" << endl;
        return -1;
    }

    // Abrir a webcam
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cout << "Erro ao acessar a câmera!" << endl;
        return -1;
    }

    // Carregar a imagem que será usada para interagir com o rosto
    Mat interactiveImg = imread("mob.png", IMREAD_UNCHANGED);
    if (interactiveImg.empty()) {
        cout << "Erro ao carregar a imagem interativa!" << endl;
        return -1;
    }

    // Redimensionar a imagem interativa (se necessário)
    resize(interactiveImg, interactiveImg, Size(100, 100)); // Ajuste de tamanho da imagem

    // Variáveis para controlar a posição da imagem
    int x = 10, y = 150;
    int moveStep = 20;

    // Loop principal
    while (true) {
        Mat frame, grayFrame;
        cap >> frame; // Captura um quadro da webcam
        if (frame.empty()) break;

        // Converter para tons de cinza para detecção de rosto
        cvtColor(frame, grayFrame, COLOR_BGR2GRAY);
        equalizeHist(grayFrame, grayFrame);

        // Detectar rostos
        vector<Rect> faces;
        faceCascade.detectMultiScale(grayFrame, faces, 1.1, 3, 0, Size(40, 40));

        // Desenhar a imagem interativa
        drawImage(frame, interactiveImg, x, y);

        // Colisão entre rosto e imagem
        Rect interactiveRect(x, y, interactiveImg.cols, interactiveImg.rows);
        bool collisionDetected = false;

        for (size_t i = 0; i < faces.size(); i++) {
            Rect face = faces[i];

            // Verifica a colisão entre o rosto e a imagem interativa
            if ((face & interactiveRect).area() > 0) {
                collisionDetected = true;
                rectangle(frame, face, Scalar(0, 0, 255), 3); // Destacar o rosto em vermelho
            } else {
                rectangle(frame, face, Scalar(255, 0, 0), 3); // Destacar o rosto em azul
            }
        }

        // Atualizar a posição da imagem interativa
        x += moveStep;
        if (x + interactiveImg.cols > frame.cols || x < 0) moveStep = -moveStep;

        // Verificar a tecla pressionada
        int key = waitKey(30);
        if (key == 27) break; // Sair se 'ESC' for pressionado
        if (collisionDetected && key == 32) { // 32 é o código ASCII para 'Spacebar'
            putText(frame, "tiro", Point(50, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
            system("mplayer tiro.wav &");
        }

        // Exibir o quadro processado
        imshow("Deteccao de Rosto com Interacao", frame);
    }

    // Liberar recursos
    cap.release();
    destroyAllWindows();
    

    return 0;
}