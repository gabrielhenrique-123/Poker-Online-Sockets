#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>
#include <random>


// Função para embaralhar as cartas
void shuffleDeck(std::vector<int> &deck, std::vector<int> &naipe) {
    std::srand(std::time(0));
    std::shuffle(deck.begin(), deck.end(), std::default_random_engine(std::rand()));
    std::shuffle(naipe.begin(), naipe.end(), std::default_random_engine(std::rand()));
}

// Função para lidar com a lógica do jogo
int playGame(int player1Choice, int player2Choice, const std::vector<int> &player1Cards, const std::vector<int> &player2Cards) {
    // Implementação da lógica do jogo
    // Retorna 1 se o jogador 1 vencer, 2 se o jogador 2 vencer, 0 se for empate

    if (player1Choice == 0 && player2Choice == 0) {
        // Ambos não apostam
        return 0;  // Empate
    } else if (player1Choice == 1 && player2Choice == 1) {
        // Ambos apostam
        int sumPlayer1 = std::accumulate(player1Cards.begin(), player1Cards.end(), 0);
        int sumPlayer2 = std::accumulate(player2Cards.begin(), player2Cards.end(), 0);
        if (sumPlayer1 > sumPlayer2) {
            std::cout << "Jogador 1 venceu" << std::endl;
            return 1;  // Jogador 1 vence
        } else if (sumPlayer1 < sumPlayer2) {
            std::cout << "Jogador 2 venceu" << std::endl;
            return 2;  // Jogador 2 vence
        } else {
            std::cout << "Empatou" << std::endl;
            return 0;  // Empate
        }
    } else {
        // Um apostou e o outro não
        if (player1Choice == 1) {
            return 1;  // Jogador 1 vence
        } else {
            return 2;  // Jogador 2 vence
        }
    }
}


int main() {
    // Configuração do servidor
    int serverSocket, clientSocket1, clientSocket2;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrSize = sizeof(struct sockaddr_in);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating server socket\n";
        return -1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(12345);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding server socket\n";
        close(serverSocket);
        return -1;
    }

    if (listen(serverSocket, 2) == -1) {
        std::cerr << "Error listening for connections\n";
        close(serverSocket);
        return -1;
    }

    // Aceitar conexão do primeiro cliente
    clientSocket1 = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrSize);
    std::cout << "Player 1 connected\n";

    // Limpar a estrutura clientAddr antes de usá-la novamente
    memset(&clientAddr, 0, sizeof(clientAddr));

    // Aceitar conexão do segundo cliente
    clientSocket2 = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrSize);
    std::cout << "Player 2 connected\n";
   
    bool continuePlaying = true;

    while (continuePlaying) {

        std::vector<int> deck{2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};  // Simplificando para cartas numeradas
        std::vector<int> naipe{1, 2, 3, 4};
        shuffleDeck(deck, naipe);

        // Envie as cartas para os jogadores
        send(clientSocket1, deck.data(), 3 * sizeof(int), 0);
        send(clientSocket1, naipe.data() + 3, 3 * sizeof(int), 0);
        send(clientSocket2, deck.data() + 6, 3 * sizeof(int), 0);
        send(clientSocket2, naipe.data() + 9, 3 * sizeof(int), 0);

        // Aguarde as escolhas dos jogadores e determine o vencedor
        int player1Choice, player2Choice;
        std::vector<int> player1Cards(3);
        std::vector<int> player2Cards(3);

        recv(clientSocket1, &player1Choice, sizeof(int), 0);
        recv(clientSocket2, &player2Choice, sizeof(int), 0);
        // Receber as cartas dos jogadores
        std::cout << "Aguardando cartas do jogador 1...\n";
        recv(clientSocket1, player1Cards.data(), 3 * sizeof(int), 0);
        std::cout << "Cartas do jogador 1 recebidas.\n";

        std::cout << "Aguardando cartas do jogador 2...\n";
        recv(clientSocket2, player2Cards.data(), 3 * sizeof(int), 0);
        std::cout << "Cartas do jogador 2 recebidas.\n";
        int winner = playGame(player1Choice, player2Choice, player1Cards, player2Cards);
        std::cout << winner << std::endl;
        // Envie o resultado para os jogadores
        send(clientSocket1, &winner, sizeof(int), 0);
        send(clientSocket2, &winner, sizeof(int), 0);
        recv(clientSocket1, &continuePlaying, sizeof(bool), 0);
        recv(clientSocket2, &continuePlaying, sizeof(bool), 0);

        // Se os jogadores optarem por continuar, envie a escolha para os clientes
        // if (continuePlaying) {
        //     send(clientSocket1, &continuePlaying, sizeof(bool), 0);
        //     send(clientSocket2, &continuePlaying, sizeof(bool), 0);
        // }
    }

    // Fechar os sockets
    close(clientSocket1);
    close(clientSocket2);
    close(serverSocket);

    return 0;
}