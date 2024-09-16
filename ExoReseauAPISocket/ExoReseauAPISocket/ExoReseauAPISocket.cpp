#include <iostream> //< std::cout/std::cerr
#include <string> //< std::string / std::string_view
#include <winsock2.h> //< Header principal de Winsock
#include <ws2tcpip.h> //< Header pour le modèle TCP/IP, permettant notamment la gestion d'adresses IP

// Sous Windows il faut linker ws2_32.lib (Proriétés du projpet => Éditeur de lien => Entrée => Dépendances supplémentaires)
// Ce projet est également configuré en C++17 (ce n'est pas nécessaire à winsock)

/*
//////
Petit projet d'exemple de mini serveur HTTP (capable d'accepter la connexion d'un navigateur via http://localhost:1337) et de renvoyer une page
Ce code est loin d'être parfait (ne gère qu'un seul client avant de se fermer, pas de libération de ressource en cas d'erreur,
ne fonctionne qu'en local à cause d'un seul appel à read, ne décode pas les données envoyées par le navigateur, etc.)
//////
*/

int main()
{
	// Initialisation de Winsock en version 2.2
	// Cette opération est obligatoire sous Windows avant d'utiliser les sockets
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data); //< MAKEWORD compose un entier 16bits à partir de deux entiers 8bits utilisés par WSAStartup pour connaître la version à initialiser

	// La création d'une socket se fait à l'aide de la fonction `socket`, celle-ci prend la famille de sockets, le type de socket,
	// ainsi que le protocole désiré (0 est possible en troisième paramètre pour laisser le choix du protocole à la fonction).
	// Pour IPv4, on utilisera AF_INET et pour IPv6 AF_INET6
	// Ici on initialise donc une socket TCP
	// Sous POSIX la fonction renvoie un entier valant -1 en cas d'erreur
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		// En cas d'erreur avec Winsock, la fonction WSAGetLastError() permet de récupérer le dernier code d'erreur
		// Sous POSIX l'équivalent est errno
		std::cerr << "failed to open socket (" << WSAGetLastError() << ")\n";
		return EXIT_FAILURE;
	}

	// On compose une adresse IP (celle-ci sert à décrire ce qui est autorisé à se connecter ainsi que le port d'écoute)
	// Cette adresse IP est associée à un port ainsi qu'à une famille (IPv4/IPv6)
	sockaddr_in bindAddr;
	bindAddr.sin_addr.s_addr = INADDR_ANY;
	bindAddr.sin_port = htons(1337); //< Conversion du nombre en big endian (endianness réseau)
	bindAddr.sin_family = AF_INET;

	// On associe notre socket à une adresse / port d'écoute
	if (bind(sock, reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr)) == SOCKET_ERROR)
	{
		std::cerr << "failed to bind socket (" << WSAGetLastError() << ")\n";
		return EXIT_FAILURE;
	}

	// On passe la socket en mode écoute, passant notre socket TCP en mode serveur, capable d'accepter des connexions externes
	// Le second argument de la fonction est le nombre de clients maximum pouvant être en attente
	if (listen(sock, 10) == SOCKET_ERROR)
	{
		std::cerr << "failed to put socket into listen mode (" << WSAGetLastError() << ")\n";
		return EXIT_FAILURE;
	}

	sockaddr_in clientAddr;
	int clientAddrSize = sizeof(clientAddr);

	// accept va sortir un client en attente depuis la file, écrire son adresse (et sa taille) dans les paramètres de sortie
	// et retourner une socket TCP correspondant à ce client
	// Note: par défaut cette fonction va mettre le programme en pause si la file d'attente est vide (on parle de fonction bloquante)
	SOCKET client = accept(sock, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);
	if (client == INVALID_SOCKET)
	{
		std::cerr << "failed to put accept new client (" << WSAGetLastError() << ")\n";
		return EXIT_FAILURE;
	}

	// Représente une adresse IP (celle du client venant de se connecter) sous forme textuelle
	char strAddr[INET_ADDRSTRLEN];
	inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, strAddr, INET_ADDRSTRLEN);

	std::cout << "new client connected from " << strAddr << std::endl;

	// Lit des données depuis la socket vers un buffer mémoire, la fonction attend une capacité maximale à lire
	// et renvoie le nombre d'octets lus
	// Note: par défaut cette fonction bloque également le programme si aucune donnée n'est disponible
	char buffer[1024];
	int byteRead = recv(client, buffer, sizeof(buffer), 0);
	if (byteRead == SOCKET_ERROR)
	{
		std::cerr << "failed to read from client (" << WSAGetLastError() << ")\n";
		return EXIT_FAILURE;
	}

	// Affichage des données reçues depuis le client (le string_view permet de limiter l'écriture du buffer à ce qui a été reçu)
	std::cout << "Received " << byteRead << " from client: " << std::string_view(buffer, byteRead);

	// Contenu de la page HTML que nous affichons en retour
	std::string_view body = "<html><body><center><h1>Bonjour l'IIM !</h1></center></body></html>";

	// Réponse suivant le protocole HTTP afin que le navigateur accepte notre réponse
	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Length: " + std::to_string(body.size()) + "\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Connection: Closed\r\n";
	response += "\r\n";
	response += body;

	// Envoi de la réponse au client
	if (send(client, response.data(), response.size(), 0) == SOCKET_ERROR)
	{
		std::cerr << "failed to send answer to client (" << WSAGetLastError() << ")\n";
		return EXIT_FAILURE;
	}

	// closesocket ferme une socket, la rendant au système, ici nous fermons d'abord la socket client puis ensuite la socket serveur (les deux étant liées)
	// Avec POSIX, l'équivalent est la fonction close
	closesocket(client);
	closesocket(sock);

	// Fermeture de Winsock
	WSACleanup();
}