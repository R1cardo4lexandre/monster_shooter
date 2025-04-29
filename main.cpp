#include "game_manager.h"
#include <iostream>

int main() {
    GameManager game;
    
    if (!game.initialize()) {
        std::cerr << "Falha na inicializacao!" << std::endl;
        return -1;
    }

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

    return 0;
}