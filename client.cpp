#include <iostream>
#include <unistd.h>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>  // Para inet_pton
#include <limits>  // Adicione esta linha para incluir o cabeçalho <limits>


int main() {
    // Configuração do cliente
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    // Conectar ao servidor
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error connecting to server\n";
        close(clientSocket);
        return -1;
    }
    bool continuePlaying = true;

    while (continuePlaying) {
        // Receber as cartas do servidor
        int cards[3], naipes[3];
        if (recv(clientSocket, cards, sizeof(cards), 0) == -1) {
            std::cerr << "Error receiving cards from server\n";
            close(clientSocket);
            return -1;
        }
        if (recv(clientSocket, naipes, sizeof(naipes), 0) == -1) {
            std::cerr << "Error receiving naipes from server\n";
            close(clientSocket);
            return -1;
        }
        // Exibir as cartas sorteadas para o jogador
        std::cout << "Suas cartas: ";
        for (int i = 0; i < 3; ++i) {
            //Valor da carta
            if(cards[i] == 11)
                std::cout << "J ";
                else if(cards[i] == 12)
                    std::cout << "Q ";
                else if(cards[i] == 13)
                    std::cout << "K ";
                else if(cards[i] == 14)
                    std::cout << "A ";
                else
                    std::cout << cards[i] << " ";
            
            //Naipe da carta
            if(naipes[i] == 1)
                std::cout << "Copas";
                else if(naipes[i] == 2)
                    std::cout << "Ouros";
                else if(naipes[i] == 3)
                    std::cout << "Espadas";
                else if(naipes[i] == 4)
                    std::cout << "Paus";
            
            if(i != 2)
                std::cout << ", ";
        }
        std::cout << std::endl;
        // Lógica do jogo - permitir ao usuário escolher apostar ou não
        int choice;
        std::cout << "Escolha (0 para não apostar, 1 para apostar): " << std::flush;
        std::cin >> choice;
        // Receber o resultado do servidor

        if (send(clientSocket, &choice, sizeof(int), 0) == -1) {
            std::cerr << "Error sending choice to server\n";
            close(clientSocket);
            return -1;
        }

        // Enviar as cartas para o servidor
        if (send(clientSocket, cards, sizeof(cards), 0) == -1) {
            std::cerr << "Error sending cards to server\n";
            close(clientSocket);
            return -1;
        }

        int winner;
        if (recv(clientSocket, &winner, sizeof(int), 0) == -1) {
            std::cerr << "Error receiving winner from server\n";
            close(clientSocket);
            return -1;
        }
        // Exibir o resultado para o jogador
        if (winner == 0) {
            std::cout << "Empate!\n";
        } else if (winner == 1) {
            std::cout << "Jogador 1 ganhou!!!\n";
        } else if (winner == 2) {
            std::cout << "Jogador 2 ganhou!!!\n";
        }
        std::cout << "Deseja continuar jogando? (1 para sim, 0 para não): " << std::flush;
        std::cin >> continuePlaying;

        // Limpar o buffer de entrada
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        // Se o jogador optar por continuar, envie a escolha para o servidor
        if (continuePlaying) {
            if (send(clientSocket, &continuePlaying, sizeof(bool), 0) == -1) {
                std::cerr << "Error sending continuePlaying choice to server\n";
                close(clientSocket);
                return -1;
            }
        }
    }
    // Lógica para relatório e continuar a jogar
    std::cout << "Algum dos jogadores encerrou o jogo\n";
    // Fechar o socket
    close(clientSocket);

    return 0;
}
