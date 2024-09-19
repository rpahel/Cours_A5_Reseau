#include <WinSock2.h>

int main()
{
	// En TCP, un send ne correspond pas à un recv lorsqu'on échange des paquets sur internet car
	// le protocole TCP va découper l'information en paquets automatiquement.
	// On peut donc recv plusieurs fois pour un seul send, ou l'inverse en fonction de la taille de l'information.

	// Anatomie d'un paquet de données :
	// Taille (uint16) | OPCode (uint8/16) | Données...

	// Ne pas oublier de convertir vers le big endian quand on serialize des données de
	// plusieurs octets (sauf données de type suite d'octets comme string).
	// uint16_t myData = 10; <- c'est un uint16, attention à le convertir en big endian avant de l'envoyer.

}

//=======================================================================================================
//=======================================================================================================
//=======================================================================================================

//#include <vector>
//#include <iostream>
//#include <WinSock2.h>
//
//enum class Direction
//{
//	Nord		= 0, // 0000
//	NordEst		= 1, // 0001
//	Est			= 2, // 0010
//	SudEst		= 3, // 0011
//	Sud			= 4, // 0100
//	SudOuest	= 5, // 0101
//	Ouest		= 6, // 0110
//	NordOuest	= 7  // 0111
//};
//
//struct PlayerStruct
//{
//	bool movingLeft = 0;
//	bool movingRight = 0;
//	bool jumping = 0;
//	Direction dir = Direction::Nord;
//};
//
//int main()
//{
//	PlayerStruct inputs;
//	inputs.movingLeft = false;
//	inputs.movingRight = true;
//	inputs.jumping = true;
//	inputs.dir = Direction::Ouest;
//
//	uint8_t x = 
//		(inputs.movingLeft		<< 0)
//		| (inputs.movingRight	<< 1)
//		| (inputs.jumping		<< 2)
//		| (uint8_t)inputs.dir	<< 3;
//
//	std::vector<uint8_t> byteArray;
//	byteArray.resize(sizeof(uint8_t));
//
//	std::memcpy(&byteArray[0], &x, sizeof(uint8_t));
//
//	uint8_t retX;
//	std::memcpy(&retX, &byteArray[0], sizeof(uint8_t));
//
//	std::cout << +retX << std::endl;
//
//	PlayerStruct retInputs;
//	retInputs.movingLeft	= retX & (1 << 0);
//	retInputs.movingRight	= retX & (1 << 1);
//	retInputs.jumping		= retX & (1 << 2);
//	retInputs.dir			= (Direction)((retX >> 3) & 7);
//}

//=======================================================================================================
//=======================================================================================================
//=======================================================================================================

//#include <iostream> //< std::cout/std::cerr
//#include <string> //< std::string / std::string_view
//#include <winsock2.h> //< Header principal de Winsock
//#include <ws2tcpip.h> //< Header pour le modèle TCP/IP, permettant notamment la gestion d'adresses IP
//#include <vector>
//#include <conio.h>
//#include <chrono>
//#include <time.h>
//
//#define CHANGE_NICKNAME_COMMAND "/nick "
//
//// Sous Windows il faut linker ws2_32.lib (Proriétés du projpet => Éditeur de lien => Entrée => Dépendances supplémentaires)
//// Ce projet est également configuré en C++17 (ce n'est pas nécessaire à winsock)
//
//std::string date_to_string(std::time_t time)
//{
//	char buf[100] = { 0 };
//	tm t;
//	localtime_s(&t, &time);
//	std::strftime(buf, sizeof(buf), "%Y-%m-%d - %H:%M:%S", &t);
//	return buf;
//}
//
//struct MessageData
//{
//	std::time_t date = 0;
//	std::string sender = "";
//	std::string message = "";
//};
//
//struct ClientData
//{
//	SOCKET socket = {};
//	std::string nickName = "";
//};
//
//struct ServerData
//{
//	SOCKET socket = {};
//	std::vector<ClientData> clients;
//	std::vector<MessageData> chatHistory;
//};
//
//int server();
//int client();
//
//int main()
//{
//	// Initialisation de Winsock en version 2.2
//	// Cette opération est obligatoire sous Windows avant d'utiliser les sockets
//	WSADATA data;
//	WSAStartup(MAKEWORD(2, 2), &data); //< MAKEWORD compose un entier 16bits à partir de deux entiers 8bits utilisés par WSAStartup pour connaître la version à initialiser
//
//	std::cout << "(S)erver or (C)lient ?" << std::flush;
//	std::string choice;
//	std::getline(std::cin, choice);
//
//	int r;
//	if (choice == "s")
//		r = server();
//	else
//		r = client();
//
//	// Fermeture de Winsock
//	WSACleanup();
//
//	return r;
//}
//
//int server()
//{
//	ServerData serverData;
//
//	serverData.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//	if (serverData.socket == INVALID_SOCKET)
//	{
//		std::cerr << "Failed to open server socket. Error code : " << WSAGetLastError() << std::endl;
//		return EXIT_FAILURE;
//	}
//
//	sockaddr_in bindAddr;
//	bindAddr.sin_addr.s_addr = INADDR_ANY;
//	bindAddr.sin_port = htons(1337); //< Conversion du nombre en big endian (endianness réseau)
//	bindAddr.sin_family = AF_INET;
//
//	// On associe notre socket à une adresse / port d'écoute
//	if (bind(serverData.socket, reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr)) == SOCKET_ERROR)
//	{
//		std::cerr << "Failed to bind server socket. Error code : " << WSAGetLastError() << std::endl;
//		return EXIT_FAILURE;
//	}
//
//	if (listen(serverData.socket, 10) == SOCKET_ERROR)
//	{
//		std::cerr << "Failed to put server socket in listen mode. Error code : " << WSAGetLastError() << std::endl;
//		return EXIT_FAILURE;
//	}
//
//	// Boucle infinie pour que le serveur tourne indéfiniment
//	while (true)
//	{
//		// On construit une liste de descripteurs pour la fonctions WSAPoll, qui nous permet de surveiller plusieurs sockets simultanément
//		// Ces descripteurs référencent les sockets à surveiller ainsi que les événements à écouter (le plus souvent on surveillera l'entrée,
//		// à l'aide de POLLIN). Ceci va détecter les données reçues en entrée par nos sockets, mais aussi les événements de déconnexion.
//		// Dans le cas de la socket serveur, cela permet aussi de savoir lorsqu'un client est en attente d'acceptation (et donc que l'appel à accept ne va pas bloquer).
//
//		// Note: on pourrait ne pas reconstruire le tableau à chaque fois, si vous voulez le faire en exercice ;o
//		std::vector<WSAPOLLFD> pollFds;
//		// La méthode emplace_back construit un objet à l'intérieur du vector et nous renvoie une référence dessus
//		// alternativement nous pourrions également construire une variable de type WSAPOLLFD et l'ajouter au vector avec push_back 
//		WSAPOLLFD& serverFd = pollFds.emplace_back();
//		serverFd.fd = serverData.socket;
//		serverFd.events = POLLIN;
//		serverFd.revents = 0;
//
//		// On rajoute un descripteur pour chacun de nos clients actuels
//		for (ClientData client : serverData.clients)
//		{
//			WSAPOLLFD& clientFd = pollFds.emplace_back();
//			clientFd.fd = client.socket;
//			clientFd.events = POLLIN;
//			clientFd.revents = 0;
//		}
//
//		// On appelle la fonction WSAPoll (équivalent poll sous Linux) pour bloquer jusqu'à ce qu'un événement se produise
//		// au niveau d'une de nos sockets. Cette fonction attend un nombre défini de millisecondes (-1 pour une attente infinie) avant
//		// de retourner le nombre de sockets actives.
//
//		int r = WSAPoll(pollFds.data(), pollFds.size(), 100);
//		if (r > 0)
//		{
//			// WSAPoll modifie le champ revents des descripteurs passés en paramètre pour indiquer les événements déclenchés
//			for (const WSAPOLLFD& pollFd : pollFds)
//			{
//				// Ce descripteur n'a pas été déclenché, on passe au suivant
//				if (pollFd.revents == 0)
//					continue;
//
//				// Ce descripteur a été déclenché, et deux cas de figures sont possibles.
//				// Soit il s'agit du descripteur de la socket serveur (celle permettant la connexion de clients), signifiant qu'un nouveau client est en attente
//				// Soit une socket client est active, signifiant que nous avons reçu des données (ou potentiellement que le client s'est déconnecté)
//				if (pollFd.fd == serverData.socket)
//				{
//					// La socket
//					sockaddr_in clientAddr;
//					int clientAddrSize = sizeof(clientAddr);
//
//					ClientData client;
//
//					client.socket = accept(serverData.socket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);
//					if (client.socket == INVALID_SOCKET)
//					{
//						std::cerr << "Failed to accept new client. Error code : " << WSAGetLastError() << std::endl;
//						return EXIT_FAILURE;
//					}
//
//					// Représente une adresse IP (celle du client venant de se connecter) sous forme textuelle
//					char strAddr[INET_ADDRSTRLEN];
//					inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, strAddr, INET_ADDRSTRLEN);
//
//					std::cout << "New client connected from " << strAddr << " : " << ntohs(clientAddr.sin_port) << std::endl;
//
//					std::string message = "Please input an username :";
//					if (send(client.socket, message.data(), message.size(), 0) == SOCKET_ERROR)
//						std::cerr << "Failed to send message to client. Error code : " << WSAGetLastError() << std::endl;
//
//					serverData.clients.push_back(client);
//				}
//				else
//				{
//					auto it = std::find_if(
//						serverData.clients.begin(),
//						serverData.clients.end(),
//						[&pollFd](const ClientData& data)
//						{
//							return data.socket == pollFd.fd;
//						}
//					);
//
//					// Client
//					char buffer[1024];
//					int len = recv(pollFd.fd, buffer, sizeof(buffer), 0);
//					if (len == 0 || len == SOCKET_ERROR)
//					{
//						// Disconnect
//						closesocket(pollFd.fd);
//
//						std::string username = it->nickName;
//
//						if (len == SOCKET_ERROR)
//							std::cerr << "Failed to receive. Error code : " << WSAGetLastError() << std::endl;
//						else
//							std::cout << "Client " << username << " disconnected." << std::endl;
//
//						serverData.clients.erase(it);
//
//						std::string messageDisconnect = username + " disconnected.";
//						for (const ClientData& client : serverData.clients)
//						{
//							if (send(client.socket, messageDisconnect.data(), messageDisconnect.size(), 0) == SOCKET_ERROR)
//							{
//								std::cerr << "Failed to send message to client. Error code : " << WSAGetLastError() << std::endl;
//								// Pas de return ici pour éviter de casser le serveur sur l'envoi à un seul client,
//								// contentons-nous pour l'instant de logger l'erreur
//							}
//						}
//
//						continue;
//					}
//
//					MessageData messageData;
//					messageData.date = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
//					messageData.sender = it->nickName;
//
//					if (it->nickName == "")
//					{
//						it->nickName = std::string(buffer, len);
//
//						std::string previousMessages;
//
//						for (auto& message : serverData.chatHistory)
//							previousMessages += (date_to_string(message.date) + " - " + (message.sender.empty() ? "" : message.sender + " : ") + message.message + "\n");
//
//						if (send(it->socket, previousMessages.data(), previousMessages.size(), 0) == SOCKET_ERROR)
//							std::cerr << "Failed to send message to client. Error code : " << WSAGetLastError() << std::endl;
//
//						messageData.message = it->nickName + " just joined the chat.";
//					}
//
//					if (messageData.message.empty())
//						messageData.message = std::string(buffer, len);
//
//					if (messageData.message.find(CHANGE_NICKNAME_COMMAND) == 0)
//					{
//						messageData.message.erase(0, std::string(CHANGE_NICKNAME_COMMAND).size());
//						
//						for (MessageData& m : serverData.chatHistory)
//						{
//							if(m.sender == it->nickName)
//								m.sender = messageData.message;
//						}
//
//						std::string deadName = it->nickName;
//						it->nickName = messageData.message;
//						messageData.sender = it->nickName;
//						messageData.message = deadName + " renamed to " + it->nickName + ".";
//					}
//
//					// On renvoie le message à tous les autres clients connectés
//					for (ClientData& client : serverData.clients)
//					{
//						std::string stringToSend;
//
//						stringToSend = messageData.sender.empty() ? messageData.message : messageData.sender + " : " + messageData.message;
//						if (send(client.socket, stringToSend.data(), stringToSend.size(), 0) == SOCKET_ERROR)
//						{
//							std::cerr << "Failed to send message to client. Error code : " << WSAGetLastError() << std::endl;
//							// Pas de return ici pour éviter de casser le serveur sur l'envoi à un seul client,
//							// contentons-nous pour l'instant de logger l'erreur
//						}
//					}
//
//					serverData.chatHistory.push_back(messageData);
//				}
//			}
//		}
//	}
//
//	closesocket(serverData.socket);
//
//	return EXIT_SUCCESS;
//}
//
//int client()
//{
//	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//	if (sock == INVALID_SOCKET)
//	{
//		std::cerr << "Failed to open client socket : " << WSAGetLastError() << std::endl;
//		return EXIT_FAILURE;
//	}
//
//	sockaddr_in bindAddr;
//	bindAddr.sin_port = htons(1337); //< Conversion du nombre en big endian (endianness réseau)
//	bindAddr.sin_family = AF_INET;
//
//	// Boucle demandant l'IP de connexion et se connectant jusqu'à y arriver
//	while (true)
//	{
//		std::cout << "IP du serveur: " << std::flush;
//		std::string ip;
//		std::getline(std::cin, ip);
//		if (ip.empty())
//			ip = "127.0.0.1";
//
//		// Conversion d'une adresse IP textuelle vers une adresse IP binaire
//		if (inet_pton(AF_INET, ip.data(), &bindAddr.sin_addr.s_addr) != 1)
//		{
//			std::cerr << "Invalid ip." << std::endl;
//			continue;
//		}
//
//		// La fonction connect va chercher à établir une connexion vers une application serveur (disposant d'une socket en mode écoute sur ce port).
//		// en mode bloquant, cette fonction ne retourne qu'une fois la connexion établie ou échouée.
//		if (connect(sock, reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr)) == SOCKET_ERROR)
//		{
//			std::cerr << "Failed to connect to ip " << ip << ". Error code : " << WSAGetLastError() << std::endl;
//			continue;
//		}
//
//		std::cout << "Connected" << std::endl;
//		break;
//	}
//
//	std::string message;
//
//	while (true)
//	{
//		// Saisie de texte
//		// Pour que notre application console nous permette de saisir du texte sans bloquer
//		// il faut passer par des API systèmes comme conio, qui nous permet de récupérer l'état du clavier
//		// sans faire de la saisie classique.
//		if (_kbhit()) //< Est-ce qu'une touche est enfoncée ?
//		{
//			char c = _getch(); //< On récupère la touche enfoncée (sans l'afficher)
//			std::cout << c; // mais on l'affiche quand même (note: on aurait aussi pu utiliser _getche permet de faire les deux)
//			if (c == '\b') //< gestion du retour arrière
//			{
//				// Afficher un retour arrière déplace juste le curseur vers le caractère précédent
//				// pour l'effacer nous pouvons mettre un espace et redéplacer le curseur à nouveau vers l'arrière
//				std::cout << ' ' << '\b';
//
//				// On enlève le caractère de notre message en cours
//				if (!message.empty())
//					message.pop_back();
//			}
//			else if (c == '\r')
//			{
//				// \r correspond à l'appui sur la touche entrée, on envoie le message si celui-ci n'est pas vide
//				if (!message.empty())
//				{
//					if (send(sock, message.data(), message.size(), 0) == SOCKET_ERROR)
//					{
//						std::cout << "Failed to send message to server : " << WSAGetLastError() << std::endl;
//						return EXIT_FAILURE;
//					}
//
//					// On affiche un retour à la ligne et le marqueur de saisie
//					for (size_t i = 0; i < message.size(); i++)
//						std::cout << ' ' << '\b';
//
//					// On vide le message pour revenir à la suite
//					message.clear();
//				}
//			}
//			else
//				message.push_back(c); // on rajoute tout autre caractère dans notre saisie
//		}
//
//		// Vérifier si le serveur nous a envoyé un message, et l'afficher
//		// On peut utiliser la fonction WSAPoll pour vérifier si notre socket est prête à lire
//		WSAPOLLFD pollFd;
//		pollFd.fd = sock;
//		pollFd.events = POLLIN;
//		pollFd.revents = 0;
//
//		// On vérifie si notre socket est active
//		// Comme notre socket est la seule à être testée, on peut simplement vérifier le retour
//		// On met un timeout de 10ms pour éviter de pomper inutilement 100% du CPU ;o
//		if (WSAPoll(&pollFd, 1, 10) > 0)
//		{
//			char buffer[2048];
//			int len = recv(sock, buffer, sizeof(buffer), 0);
//			if (len == 0 || len == SOCKET_ERROR)
//			{
//				// recv renvoie 0 en cas de déconnexion propre, ou SOCKET_ERROR en cas d'erreur (typiquement time out)
//				if (len == 0)
//					std::cout << "Disconnected from server." << std::endl;
//				else
//					std::cout << "Disconnected with error: " << WSAGetLastError() << std::endl;
//
//				return EXIT_FAILURE;
//			}
//
//			std::string_view receivedMessage(buffer, len);
//
//			// Pour garder la saisie continue, on fait un retour à la ligne avant d'afficher le message
//			// puis à nouveau le marqueur de saisie et le message en cours
//			// ce n'est pas idéal mais c'est plus simple qu'effacer la ligne
//			std::cout << receivedMessage << "\n" << message << std::flush;
//		}
//	}
//
//	// Fermeture de la socket client
//	closesocket(sock);
//
//	return EXIT_SUCCESS;
//}