import socket
import threading

class Client:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connected = False
        self.ready_event = threading.Event()

    def connect(self):
        try:
            self.client_socket.connect((self.host, self.port))
            self.connected = True
            print("Você está conectado ao servidor.")
        except socket.error as e:
            print(f"[!] Erro ao conectar ao servidor: {e}")

    def receive_message(self):              #Funcao responsavel por receber mensagens do servidor
        while self.connected:
            try:
                message = self.client_socket.recv(1024).decode()
                print(message)
            except socket.error as e:
                print(f"[!] Erro ao receber mensagem: {e}")
                self.connected = False
                break

    def send_message(self, message):        #Funcao responsavel por enviar mensagens no servidor
        try:
            self.client_socket.sendall(message.encode())
        except socket.error as e:
            print(f"[!] Erro ao enviar mensagem: {e}")
            self.connected = False

    def play_game(self):
        print("Esperando pelo segundo jogador...")

        # Inicia o thread para receber mensagens
        threading.Thread(target=self.receive_message).start()

        # Aguarda até que o evento seja definido
        self.ready_event.wait()
        print("[*] O jogo está prestes a começar!")
        
        while self.connected:
            bet = input("Apostar ou não apostar? ").lower()
            self.send_message(bet)

            response = self.client_socket.recv(1024).decode()
            print(response)

            if response.lower() == "nao":
                self.connected = False

    def close_connection(self):
        try:
            self.client_socket.close()
        except socket.error as e:
            print(f"[!] Erro ao fechar conexão: {e}")

if __name__ == "__main__":
    client = Client("127.0.0.1", 5054)
    client.connect()

    # Define o evento indicando que o jogador está pronto
    client.ready_event.set()

    client.play_game()
    client.close_connection()
