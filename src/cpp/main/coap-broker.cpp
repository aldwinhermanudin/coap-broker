#include <iostream>
#include <vector>
#include <signal.h>
#include <cstring>
#include <memory>
#include "coap.h" 
#include <LinkedListDB.hpp>
#include "UString.hpp"
#include "Resource.hpp"
#include "Server.hpp"
#include "BrokerResource.hpp"

using namespace std; 

int quit = 0;
static void handle_sigint(int signum) {
  quit = 1;
}

static void usage( const char *program, const char *version);

int main(){
 
    coap::Server coap_server; 

    coap_server.set_log_level(LOG_DEBUG);
    coap::BrokerResource coap_broker(coap::UString("ps"));
    coap_server.add_resource(coap_broker); 

    signal(SIGINT, handle_sigint);
    while (!quit) {
        coap_server.run();
		coap_broker.topicDataMAMonitor();
		coap_broker.topicMAMonitor(coap_server.get_context());
    }   

	coap_server.end_server();
    return 0;
}


static void usage( const char *program, const char *version) {
	const char *p;

	p = strrchr( program, '/' );
	if ( p ) program = ++p;

	fprintf( stderr, "%s v%s -- Final Project, a CoAP Broker implementation\n"
		"(c) 2017 Aldwin Hermanudin<aldwin@hermanudin.com>\n\n"
		"usage: %s [-e addtional-server] [-v verbose]\n\n"
		"\t-e server\tenable additional server. mqtt for mqtt bridge. rd for resource directory\n"
		"\t-v num\t\tverbosity level (default: 3)\n"
		"\n"
		"examples:\n"
		"\t%s -e mqtt -e rd -v 9\n"
		,program, version, program,program );
}
