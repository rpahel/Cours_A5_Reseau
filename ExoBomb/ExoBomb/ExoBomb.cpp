#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#define CODE_TO_SEND_1 "TKarmli3t>Lffe"
#define CODE_TO_SEND_2 "TtvSLVF1805"
#define CODE_TO_SEND_3 "LPA38-50AJC"

#include <cassert> //< assert
#include <iostream> //< std::cout/std::cerr
#include <stdexcept> //< std::string / std::string_view
#include <string> //< std::string / std::string_view
#include <vector> //< std::vector
#include <winsock2.h> //< Header principal de Winsock
#include <ws2tcpip.h> //< Header pour le modèle TCP/IP, permettant notamment la gestion d'adresses IP

// Sous Windows il faut linker ws2_32.lib (Propriétés du projet => Éditeur de lien => Entrée => Dépendances supplémentaires)
// Ce projet est également configuré en C++17 (ce n'est pas nécessaire à winsock)

std::vector<std::uint8_t> ReadData(SOCKET sock);
void SendData(SOCKET sock, const void* data, std::size_t dataLength);

int main()
{
	// Comme vu précédemment on initialise le réseau et créons une socket TCP
	// Ces opérations sont décrites plus en détail dans les corrigés précédents
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		std::cerr << "failed to open socket (" << WSAGetLastError() << ")\n";
		return EXIT_FAILURE;
	}

	// Construction de l'adresse IP / port pour pouvoir ensuite s'y connecter
	// Ces opérations sont décrites plus en détail dans le corrigé sur le serveur de chat
	sockaddr_in clientIp;
	clientIp.sin_family = AF_INET;
	clientIp.sin_port = htons(42666);

	inet_pton(AF_INET, "127.0.0.1", &clientIp.sin_addr);

	if (connect(sock, reinterpret_cast<sockaddr*>(&clientIp), sizeof(clientIp)) == SOCKET_ERROR)
	{
		std::cout << "failed to connect to server (" << WSAGetLastError() << ")";
		return EXIT_FAILURE;
	}

	// La connexion est établie, commençons à envoyer nos messages
	std::cout << "connected to server" << std::endl;

	std::string message = CODE_TO_SEND_1;
	SendData(sock, message.data(), message.size());

	SOCKET bomb_socket;
	std::string key;
	uint16_t firstint16 = 0;
	uint16_t secondint16 = 0;
	uint32_t codeToBomb = 0;
	uint32_t codeFromBomb = 0;
	while (true)
	{
		WSAPOLLFD pollFd;
		pollFd.fd = sock;
		pollFd.events = POLLIN;
		pollFd.revents = 0;

		if (WSAPoll(&pollFd, 1, 100) > 0)
		{
			std::vector<uint8_t> r = ReadData(sock);

			if (key.empty())
			{
				// Receive
				key.resize(r.size());
				std::memcpy(&key[0], &r[0], r.size());
				std::cout << "Received key from bomb : " << key << std::endl;

				// Reply
				message = CODE_TO_SEND_2;
				std::vector<uint8_t> byteArr;
				byteArr.resize(1 + message.size());
				byteArr[0] = message.size();
				std::memcpy(&byteArr[1], &message[0], message.size());

				SendData(sock, byteArr.data(), byteArr.size());
			}
			else if(firstint16 == 0)
			{
				// Receive
				std::memcpy(&firstint16, &r[0], r.size());
				firstint16 = ntohs(firstint16);
				std::cout << "Received 16bit number from bomb : " << firstint16 << std::endl;

				// Reply
				message = CODE_TO_SEND_3;
				std::string messageToSend = message;

				for (size_t i = 0; i < messageToSend.size(); i++)
					messageToSend[i] ^= key[i % key.size()];

				std::vector<uint8_t> byteArr;
				uint16_t size = htons(messageToSend.size());
				byteArr.resize(2 + messageToSend.size());
				std::memcpy(&byteArr[0], &size, 2);
				std::memcpy(&byteArr[2], &messageToSend[0], messageToSend.size());

				SendData(sock, byteArr.data(), byteArr.size());
			}
			else if (secondint16 == 0)
			{
				// Receive
				std::memcpy(&secondint16, &r[0], r.size());
				secondint16 = ntohs(secondint16);
				std::cout << "Received 16bit number from bomb : " << secondint16 << std::endl;

				codeToBomb = firstint16;
				codeToBomb <<= 16;
				codeToBomb |= secondint16;

				std::cout << "codeToBomb : " << codeToBomb << std::endl;

				// Reply
				uint32_t send32 = htonl(codeToBomb);
				SendData(sock, &send32, sizeof(uint32_t));
			}
			else if(codeFromBomb == 0)
			{
				std::memcpy(&codeFromBomb, &r[0], r.size());
				codeFromBomb = ntohl(codeFromBomb);
				std::cout << "Received 32bit number from bomb : " << codeFromBomb << std::endl;

				uint32_t decalage = codeFromBomb & 7;
				std::cout << "Decalage : " << decalage << std::endl;

				codeFromBomb >>= (decalage + 3);
				uint8_t value = codeFromBomb & 255;

				std::cout << "value : " << +value << std::endl;

				SendData(sock, &value, sizeof(uint8_t));
			}
		}
	}

	closesocket(bomb_socket);
	closesocket(sock);
	WSACleanup();

	return EXIT_SUCCESS;
}

// Les fonctons ci-dessous sont des helpers pour ne pas avoir à se soucier de la gestion d'erreur / des appels aux fonctions réseau,
// ceci n'étant pas le but de cet exo. De plus nous gérons les erreurs avec une exceptions, interrompant immédiatement le programme en cas d'erreur.

std::vector<std::uint8_t> ReadData(SOCKET sock)
{
	std::vector<std::uint8_t> output(1024);
	int byteRead = recv(sock, reinterpret_cast<char*>(output.data()), static_cast<int>(output.size()), 0);
	if (byteRead == 0 || byteRead == SOCKET_ERROR)
	{
		if (byteRead == 0)
			std::cout << "server closed connection" << std::endl;
		else
			std::cerr << "failed to read data from server (" << WSAGetLastError() << ")" << std::endl;

		throw std::runtime_error("failed to read data");
	}

	output.resize(byteRead);

	return output;
}

void SendData(SOCKET sock, const void* data, std::size_t dataLength)
{
	if (send(sock, static_cast<const char*>(data), static_cast<int>(dataLength), 0) == SOCKET_ERROR)
	{
		std::cerr << "failed to send data to server (" << WSAGetLastError() << ")" << std::endl;
		throw std::runtime_error("failed to send data");
	}
}

