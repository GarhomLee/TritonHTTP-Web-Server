#include <sysexits.h>
#include <fstream>
// #include <thread>
// #include <unordered_map>
#include "logger.hpp"
#include "HttpdServer.hpp"

HttpdServer::HttpdServer(INIReader &t_config) : config(t_config)
{
	auto log = logger();
	// const char* env_p = ;
	string home(std::getenv("HOME")); // in case to replace "~"
	log->info("home: {}", home);

	/* get the port number */
	string pstr = config.Get("httpd", "port", "");
	if (pstr == "")
	{
		log->error("port was not in the config file");
		exit(EX_CONFIG);
	}
	ci.port = pstr;

	/* get document root */
	string dr = config.Get("httpd", "doc_root", "");
	if (dr == "")
	{
		log->error("doc_root was not in the config file");
		exit(EX_CONFIG);
	}
	ci.doc_root = dr.find("~") == 0 ? home + dr.substr(1) : dr; // replace "~"

	/* get mime mapping */
	string mt = config.Get("httpd", "mime_types", "");
	if (mt == "")
	{
		log->error("mime_types was not in the config file");
		exit(EX_CONFIG);
	}
	mt = mt.find("~") == 0 ? home + mt.substr(1) : mt; // replace "~"
	ifstream mime_file(mt);
	if (!mime_file.is_open())
	{
		log->error("Failed to open mime.types");
		exit(1);
	}
	// log->info("mt: {}", mt);
	// log->info("ci.doc_root: {}", ci.doc_root);
	string line, key, value;
	while (getline(mime_file, line))
	{
		istringstream iss(line);
		iss >> key;
		iss >> value;
		ci.mime_mapping[key] = value;
	}
	mime_file.close();
}

/* launch the server */
void HttpdServer::launch()
{
	auto log = logger();

	log->info("Launching web server");
	log->info("Port: {}", ci.port);
	log->info("doc_root: {}", ci.doc_root);

	// Put code here that actually launches your webserver...
	int servSocket, clntSocket; // socket descriptors for server and client respectively
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_in clntAddr; // client address
	unsigned int clntAddrLen;	// length of client address

	/* initialize hints */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;	 // prefer IPv6, but allow IPv4
	hints.ai_socktype = SOCK_STREAM; // use TCP
	hints.ai_flags = AI_PASSIVE;	 // we're going to be a server

	/* get address information of the server */
	int rv;
	if ((rv = getaddrinfo(NULL, ci.port.c_str(), &hints, &servinfo)) != 0)
	{
		log->error("Failed to get address information of the server.");
		exit(1);
	}
	log->info("Get address information of the server successfully.");

	/* pick one server address */
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		/* start a socket */
		if ((servSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			log->error("Failed to start a socket. Trying next one...");
			continue;
		}
		log->info("Start a socket successfully.");

		/* set address to be used */
		const int reuseFlag = 1;
		if (setsockopt(servSocket, SOL_SOCKET, SO_REUSEADDR, &reuseFlag,
					   sizeof(reuseFlag)) != 0)
		{
			log->error("Failed to set address to be used.");
			exit(1);
		}
		log->info("Set address to be used successfully.");

		/* bind a socket */
		if (::bind(servSocket, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(servSocket);
			log->error("Failed to bind a socket. Trying next one...");
			continue;
		}
		log->info("Bind a socket a socket successfully.");

		break;
	}

	/* no socket is bound */
	if (p == NULL)
	{
		log->error("Failed to bind any sockets.");
		exit(1);
	}

	freeaddrinfo(servinfo);

	/* mark the socket to listen for incoming connections */
	if (listen(servSocket, MAX_PENDING) == -1)
	{
		log->error("Failed to listen.");
		exit(1);
	}
	log->info("Listen to a connection successfully.");

	/* run forever */
	while (true)
	{
		/* accept a connection from the client */
		clntAddrLen = sizeof(clntAddr);
		if ((clntSocket = accept(servSocket, (struct sockaddr *)&clntAddr,
								 &clntAddrLen)) == -1)
		{
			log->error("Failed to accept the connection from the client.");
			exit(1);
		}
		log->info("Accept the connection successfully.");

		/* set time out if not receive request */
		struct timeval timeout;
		timeout.tv_sec = TIME_OUT;
		timeout.tv_usec = 0;
		if (setsockopt(clntSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout,
					   sizeof(timeout)) != 0)
		{
			log->error("Failed to set time out for connections.");
			exit(1);
		}

		/* start a new thread to handle requests of each connection */
		thread([&]() {
			MessageParser mp(clntSocket, ci);
			mp.receive();
		})
			.detach();
		log->info("Start a new thread successfully.");
	}
}
