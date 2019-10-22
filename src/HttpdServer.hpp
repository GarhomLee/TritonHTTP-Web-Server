#ifndef HTTPDSERVER_HPP
#define HTTPDSERVER_HPP

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sstream>
#include <unordered_map>
#include "inih/INIReader.h"
#include "logger.hpp"
#include "MessageParser.hpp"

#define MAX_PENDING 10
#define TIME_OUT 5

using namespace std;

class HttpdServer {
	public:
		HttpdServer(INIReader& t_config);
		// string response_400(bool isClosed);
		// void handleClientRequests(int clntSocket);
		void launch();

	protected:
        INIReader& config;
		struct config_info ci;
};

#endif // HTTPDSERVER_HPP
