#include "game_manager.h"
#include <iostream>

int main() {
    while (true) {
        GameManager game;

        if (!game.initialize()) {
            std::cerr << "Falha na inicializacao!" << std::endl;
            return -1;
        }

        if (!game.showMenu()) {
            std::cout << "Saindo do jogo..." << std::endl;
            break;
        }

        // Limpa completamente o buffer de eventos do OpenCV
        int key;
        do {
            key = cv::waitKey(10);
        } while (key != -1);

        std::cout << "=== MONSTER SHOOTER ===" << std::endl;
        std::cout << "Objetivo: Vencer as " << game.getMaxHordes() << " hordas" << std::endl;
        std::cout << "Voce pode errar no maximo " << game.getMaxMissedShots() << " tiros" << std::endl;
        std::cout << "Controles:" << std::endl;
        std::cout << " - Mire com o objeto verde" << std::endl;
        std::cout << " - Pressione ESPACO para atirar" << std::endl;
        std::cout << " - Pressione ESC para sair" << std::endl;
        std::cout << "=======================" << std::endl;

        game.run();

        if (game.isGameWon()) {
            std::cout << "\nPARABENS! Voce derrotou o chefe final!" << std::endl;
        } else {
            std::cout << "\nFim de jogo! Voce chegou na horda "
                      << game.getHordeNumber() << std::endl;
        }

        std::cout << "Estatisticas finais:" << std::endl;
        std::cout << " - Monstros eliminados: " << game.getTotalMonstersDefeated() << std::endl;
        std::cout << " - Tiros errados: " << game.getMissedShots()
                  << "/" << game.getMaxMissedShots() << std::endl;

        // Se o jogador não quiser voltar ao menu, saia do loop
        if (!game.isGameRunning() && !game.isGameWon()) {
            // O loop só continua se o jogador pressionou 'M' para voltar ao menu
            continue;
        }
        break;
    }
    return 0;
}