#include <iostream> //< std::cout/std::cerr
#include <string> //< std::string / std::string_view
#include <winsock2.h> //< Header principal de Winsock
#include <ws2tcpip.h> //< Header pour le mod�le TCP/IP, permettant notamment la gestion d'adresses IP
#include <vector>
#include <conio.h>

// Sous Windows il faut linker ws2_32.lib (Prori�t�s du projpet => �diteur de lien => Entr�e => D�pendances suppl�mentaires)
// Ce projet est �galement configur� en C++17 (ce n'est pas n�cessaire � winsock)

struct ClientData
{
	SOCKET socket;
	std::string nickName;
};

int server();
int client();

int main()
{
	// Initialisation de Winsock en version 2.2
	// Cette op�ration est obligatoire sous Windows avant d'utiliser les sockets
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data); //< MAKEWORD compose un entier 16bits � partir de deux entiers 8bits utilis�s par WSAStartup pour conna�tre la version � initialiser

	std::cout << "server/client? " << std::flush;
	std::string choice;
	std::getline(std::cin, choice);

	int r;
	if (choice == "s")
		r = server();
	else
		r = client();

	// Fermeture de Winsock
	WSACleanup();

	return r;
}

int server()
{
	SOCKET serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSock == INVALID_SOCKET)
	{
		std::cerr << "failed to open socket (" << WSAGetLastError() << ")\n";
		return EXIT_FAILURE;
	}

	sockaddr_in bindAddr;
	bindAddr.sin_addr.s_addr = INADDR_ANY;
	bindAddr.sin_port = htons(1337); //< Conversion du nombre en big endian (endianness r�seau)
	bindAddr.sin_family = AF_INET;

	// On associe notre socket � une adresse / port d'�coute
	if (bind(serverSock, reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr)) == SOCKET_ERROR)
	{
		std::cerr << "failed to bind socket (" << WSAGetLastError() << ")\n";
		return EXIT_FAILURE;
	}

	if (listen(serverSock, 10) == SOCKET_ERROR)
	{
		std::cerr << "failed to put socket into listen mode (" << WSAGetLastError() << ")\n";
		return EXIT_FAILURE;
	}

	std::vector<ClientData> clients;
	// Boucle infinie pour que le serveur tourne ind�finiment
	while (true)
	{
		// On construit une liste de descripteurs pour la fonctions WSAPoll, qui nous permet de surveiller plusieurs sockets simultan�ment
		// Ces descripteurs r�f�rencent les sockets � surveiller ainsi que les �v�nements � �couter (le plus souvent on surveillera l'entr�e,
		// � l'aide de POLLIN). Ceci va d�tecter les donn�es re�ues en entr�e par nos sockets, mais aussi les �v�nements de d�connexion.
		// Dans le cas de la socket serveur, cela permet aussi de savoir lorsqu'un client est en attente d'acceptation (et donc que l'appel � accept ne va pas bloquer).

		// Note: on pourrait ne pas reconstruire le tableau � chaque fois, si vous voulez le faire en exercice ;o
		std::vector<WSAPOLLFD> pollFds;
		// La m�thode emplace_back construit un objet � l'int�rieur du vector et nous renvoie une r�f�rence dessus
		// alternativement nous pourrions �galement construire une variable de type WSAPOLLFD et l'ajouter au vector avec push_back 
		WSAPOLLFD& serverFd = pollFds.emplace_back();
		serverFd.fd = serverSock;
		serverFd.events = POLLIN;
		serverFd.revents = 0;

		// On rajoute un descripteur pour chacun de nos clients actuels
		for (ClientData client : clients)
		{
			WSAPOLLFD& clientFd = pollFds.emplace_back();
			clientFd.fd = client.socket;
			clientFd.events = POLLIN;
			clientFd.revents = 0;
		}

		// On appelle la fonction WSAPoll (�quivalent poll sous Linux) pour bloquer jusqu'� ce qu'un �v�nement se produise
		// au niveau d'une de nos sockets. Cette fonction attend un nombre d�fini de millisecondes (-1 pour une attente infinie) avant
		// de retourner le nombre de sockets actives.

		int r = WSAPoll(pollFds.data(), pollFds.size(), 100);
		if (r > 0)
		{
			// WSAPoll modifie le champ revents des descripteurs pass�s en param�tre pour indiquer les �v�nements d�clench�s
			for (const WSAPOLLFD& pollFd : pollFds)
			{
				// Ce descripteur n'a pas �t� d�clench�, on passe au suivant
				if (pollFd.revents == 0)
					continue;

				// Ce descripteur a �t� d�clench�, et deux cas de figures sont possibles.
				// Soit il s'agit du descripteur de la socket serveur (celle permettant la connexion de clients), signifiant qu'un nouveau client est en attente
				// Soit une socket client est active, signifiant que nous avons re�u des donn�es (ou potentiellement que le client s'est d�connect�)
				if (pollFd.fd == serverSock)
				{
					// La socket
					sockaddr_in clientAddr;
					int clientAddrSize = sizeof(clientAddr);

					SOCKET c = accept(serverSock, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);
					if (c == INVALID_SOCKET)
					{
						std::cerr << "failed to put accept new client (" << WSAGetLastError() << ")\n";
						return EXIT_FAILURE;
					}

					ClientData client;
					client.socket = c;
					client.nickName = "";

					// Repr�sente une adresse IP (celle du client venant de se connecter) sous forme textuelle
					char strAddr[INET_ADDRSTRLEN];
					inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, strAddr, INET_ADDRSTRLEN);

					std::cout << "new client connected from " << strAddr << ":" << ntohs(clientAddr.sin_port) << std::endl;

					std::string message = "please input an username : \n";
					if (send(client.socket, message.data(), message.size(), 0) == SOCKET_ERROR)
						std::cerr << "failed to send message to client (" << WSAGetLastError() << ")\n";

					clients.push_back(client);
				}
				else
				{
					// Client
					char buffer[1024];
					int len = recv(pollFd.fd, buffer, sizeof(buffer), 0);
					if (len == 0 || len == SOCKET_ERROR)
					{
						// Disconnect
						closesocket(pollFd.fd);

						auto it = std::find_if(clients.begin(), clients.end(), [&pollFd](ClientData data){return data.socket == pollFd.fd;});

						std::string username = it->nickName;

						if (len == SOCKET_ERROR)
							std::cerr << "recv failed (" << WSAGetLastError() << ")\n";
						else
							std::cout << username << " client disconnected" << std::endl;

						clients.erase(it);

						std::string messageDisconnect = username + " disconnected.";
						for (ClientData client : clients)
						{
							if (send(client.socket, messageDisconnect.data(), messageDisconnect.size(), 0) == SOCKET_ERROR)
							{
								std::cerr << "failed to send message to client (" << WSAGetLastError() << ")\n";
								// Pas de return ici pour �viter de casser le serveur sur l'envoi � un seul client,
								// contentons-nous pour l'instant de logger l'erreur
							}
						}

						continue;
					}

					std::cout << "received " << std::string_view(buffer, len) << std::endl;

					// On renvoie le message � tous les autres clients connect�s
					for (ClientData client : clients)
					{
						bool newClient = false;
						// On �vite d'envoyer le message � la personne qui vient de l'envoyer
						if (client.socket == pollFd.fd)
						{
							if (client.nickName == "")
							{
								client.nickName == std::string(buffer, len);
								newClient = true;
							}

							continue;
						}

						if (newClient)
						{
							std::string newClientMessage = client.nickName + " just joined the chat.\n";
							if (send(client.socket, newClientMessage.data(), newClientMessage.size(), 0) == SOCKET_ERROR)
								std::cerr << "failed to send message to client (" << WSAGetLastError() << ")\n";
						}
						else
						{
							if (send(client.socket, buffer, len, 0) == SOCKET_ERROR)
							{
								std::cerr << "failed to send message to client (" << WSAGetLastError() << ")\n";
								// Pas de return ici pour �viter de casser le serveur sur l'envoi � un seul client,
								// contentons-nous pour l'instant de logger l'erreur
							}
						}
					}
				}
			}
		}
	}

	closesocket(serverSock);

	return EXIT_SUCCESS;
}

int client()
{
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		std::cerr << "failed to open socket (" << WSAGetLastError() << ")\n";
		return EXIT_FAILURE;
	}

	sockaddr_in bindAddr;
	bindAddr.sin_port = htons(1337); //< Conversion du nombre en big endian (endianness r�seau)
	bindAddr.sin_family = AF_INET;

	// Boucle demandant l'IP de connexion et se connectant jusqu'� y arriver
	while (true)
	{
		std::cout << "IP du serveur: " << std::flush;
		std::string ip;
		std::getline(std::cin, ip);
		if (ip.empty())
			ip = "127.0.0.1";

		// Conversion d'une adresse IP textuelle vers une adresse IP binaire
		if (inet_pton(AF_INET, ip.data(), &bindAddr.sin_addr.s_addr) != 1)
		{
			std::cerr << "invalid ip" << std::endl;
			continue;
		}

		// La fonction connect va chercher � �tablir une connexion vers une application serveur (disposant d'une socket en mode �coute sur ce port).
		// en mode bloquant, cette fonction ne retourne qu'une fois la connexion �tablie ou �chou�e.
		if (connect(sock, reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr)) == SOCKET_ERROR)
		{
			std::cerr << "failed to connect" << std::endl;
			continue;
		}

		std::cout << "Connected" << std::endl;
		break;
	}

	std::string message;
	std::cout << "> " << std::flush;

	while (true)
	{
		// Saisie de texte
		// Pour que notre application console nous permette de saisir du texte sans bloquer
		// il faut passer par des API syst�mes comme conio, qui nous permet de r�cup�rer l'�tat du clavier
		// sans faire de la saisie classique.
		if (_kbhit()) //< Est-ce qu'une touche est enfonc�e ?
		{
			char c = _getch(); //< On r�cup�re la touche enfonc�e (sans l'afficher)
			std::cout << c; // mais on l'affiche quand m�me (note: on aurait aussi pu utiliser _getche permet de faire les deux)
			if (c == '\b') //< gestion du retour arri�re
			{
				// Afficher un retour arri�re d�place juste le curseur vers le caract�re pr�c�dent
				// pour l'effacer nous pouvons mettre un espace et red�placer le curseur � nouveau vers l'arri�re
				std::cout << ' ' << '\b';

				// On enl�ve le caract�re de notre message en cours
				if (!message.empty())
					message.pop_back();
			}
			else if (c == '\r')
			{
				// \r correspond � l'appui sur la touche entr�e, on envoie le message si celui-ci n'est pas vide
				if (!message.empty())
				{
					if (send(sock, message.data(), message.size(), 0) == SOCKET_ERROR)
					{
						std::cout << "failed to send message to server: " << WSAGetLastError() << std::endl;
						return EXIT_FAILURE;
					}

					// On affiche un retour � la ligne et le marqueur de saisie
					std::cout << "\n> " << std::flush;

					// On vide le message pour revenir � la suite
					message.clear();
				}
			}
			else
				message.push_back(c); // on rajoute tout autre caract�re dans notre saisie
		}

		// V�rifier si le serveur nous a envoy� un message, et l'afficher
		// On peut utiliser la fonction WSAPoll pour v�rifier si notre socket est pr�te � lire
		WSAPOLLFD pollFd;
		pollFd.fd = sock;
		pollFd.events = POLLIN;
		pollFd.revents = 0;

		// On v�rifie si notre socket est active
		// Comme notre socket est la seule � �tre test�e, on peut simplement v�rifier le retour
		// On met un timeout de 10ms pour �viter de pomper inutilement 100% du CPU ;o
		if (WSAPoll(&pollFd, 1, 10) > 0)
		{
			char buffer[1024];
			int len = recv(sock, buffer, sizeof(buffer), 0);
			if (len == 0 || len == SOCKET_ERROR)
			{
				// recv renvoie 0 en cas de d�connexion propre, ou SOCKET_ERROR en cas d'erreur (typiquement time out)
				if (len == 0)
					std::cout << "disconnected from server" << std::endl;
				else
					std::cout << "disconnected with error: " << WSAGetLastError() << std::endl;

				return EXIT_FAILURE;
			}

			std::string_view receivedMessage(buffer, len);

			// Pour garder la saisie continue, on fait un retour � la ligne avant d'afficher le message
			// puis � nouveau le marqueur de saisie et le message en cours
			// ce n'est pas id�al mais c'est plus simple qu'effacer la ligne
			std::cout << '\n' << receivedMessage << "\n> " << message << std::flush;
		}
	}

	// Fermeture de la socket client
	closesocket(sock);

	return EXIT_SUCCESS;
}