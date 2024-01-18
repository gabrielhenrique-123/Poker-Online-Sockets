import socket
import threading
import random

class Server:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.clients = []
        self.cards_deck = ['2', '3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K', 'A']
        self.cards_naipe = ['Espada', 'Copas', 'Paus', 'Ouros']
        self.game_running = False
        self.scores = {}
        self.lock = threading.Lock()

    def start(self):
        try:
            self.server_socket.bind((self.host, self.port))
            self.server_socket.listen(2)
            print(f"Servidor esperando por conexões em {self.host}:{self.port}")

            while len(self.clients) < 2:        #Mantém se no loop enquanto nao houver ao menos dois clients
                client_socket, addr = self.server_socket.accept()
                self.clients.append((client_socket, addr))
                print(f"Conexão estabelecida com {addr}")
            
            #Ao sair do loop, a mensagem é exibida
            print("Ambos os jogadores conectados. Iniciando o jogo...")
            self.start_game()
        except Exception as e:
            print(f"[!] Erro ao iniciar o servidor: {e}")
            self.server_socket.close()

    def start_game(self):
        self.game_running = True

        # Inicia threads separadas para cada jogador
        threads = []
        for client, addr in self.clients:
            thread = threading.Thread(target=self.play_turn, args=(client, addr))
            threads.append(thread)
            thread.start()

        # Aguarda todas as threads terminarem antes de prosseguir
        for thread in threads:
            thread.join()

        self.determine_winner()
        self.send_results()

        # Pergunta se os jogadores querem continuar
        if self.ask_to_continue():
            self.send_game_report()
        else:
            self.server_socket.close()

    def play_turn(self, client, addr, ready_event):
        # Aguarda até que o jogador esteja pronto
        ready_event.wait()

        with self.lock:
            self.reset_scores()
            self.deal_cards(client, addr)
            self.place_bets(client, addr)


    def reset_scores(self):
        self.scores = {addr: [] for _, addr in self.clients}

    def deal_cards(self, client, addr):
        contador = 0
        cards = []
        while contador < 3:
            cards += random.sample(self.cards_deck, 1) + random.sample(self.cards_naipe, 1) 
            contador += 1
        self.scores[addr] = cards
        client.sendall(f"Suas cartas: {cards}\n".encode())

    def place_bets(self, client, addr):
        client.sendall("Você deseja apostar? (sim/não): ".encode())
        bet = client.recv(1024).decode().lower()

        if bet == "sim":
            result = random.choice(["ganhar", "perder"])
            if result == "ganhar":
                self.scores[addr].append(3)
                client.sendall("Você ganhou a rodada!\n".encode())
            else:
                self.scores[addr].append(-2)
                client.sendall("Você perdeu a rodada!\n".encode())
        else:
            self.scores[addr].append(-1)
            client.sendall("Você escolheu não apostar.\n".encode())

    def determine_winner(self):
        total_scores = {addr: sum(self.scores[addr]) for _, addr in self.clients}
        winner = max(total_scores, key=total_scores.get)
        print(f"[*] Vencedor: {winner} - Soma das cartas: {total_scores[winner]}")

    def send_results(self):
        total_scores = {addr: sum(self.scores[addr]) for _, addr in self.clients}
        for client, addr in self.clients:
            opponent_addr = [a for _, a in self.clients if a != addr][0]
            opponent_score = total_scores[opponent_addr]

            player_score = total_scores[addr]
            result_message = "Resultado: "

            if player_score > opponent_score:
                result_message += f'Você venceu! ({player_score} pontos contra {opponent_score}).\n'
            elif player_score < opponent_score:
                result_message += f'Você perdeu! ({player_score} pontos contra {opponent_score}).\n'
            else:
                result_message += f'Empate com o oponente! ({player_score} pontos).\n'

            client.sendall(result_message.encode())

    def ask_to_continue(self):
        responses = set()
        for client, addr in self.clients:
            client.sendall("Deseja continuar jogando? (sim/não): ".encode())
            response = client.recv(1024).decode().lower()
            responses.add(response)

        return all(resp == "sim" for resp in responses)

if __name__ == "__main__":
    server = Server("127.0.0.1", 5054)
    server.start()
