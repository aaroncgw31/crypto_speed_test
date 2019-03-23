#include "fix_common.h"

#include "compat.h"
#include "array.h"
#include "time.h"
#include "die.h"

#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <inttypes.h>
#include <libgen.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <float.h>
#include <netdb.h>
#include <stdio.h>
#include <math.h>

#include "fix_client.h"
#include "test.h"

struct fix_session *session = NULL;
struct fix_field *fields = NULL;
unsigned long nr;

static sig_atomic_t stop;

static int socket_setopt(int sockfd, int level, int optname, int optval)
{
	return setsockopt(sockfd, level, optname, (void *) &optval, sizeof(optval));
}

static void signal_handler(int signum)
{
	if (signum == SIGINT)
		stop = 1;
}

static int fix_logout_send(struct fix_session *session, const char *text)
{
	struct fix_field fields[] = {
		FIX_STRING_FIELD(Text, text),
	};
	long nr_fields = ARRAY_SIZE(fields);
	struct fix_message logout_msg;

	if (!text)
		nr_fields--;

	logout_msg	= (struct fix_message) {
		.type		= FIX_MSG_TYPE_LOGOUT,
		.nr_fields	= nr_fields,
		.fields		= fields,
	};

	return fix_session_send(session, &logout_msg, 0);
}

static int fix_client_logout(struct fix_session *session, const char *text, bool grace)
{
	int ret;

	if (grace)
		ret = fix_session_logout(session, text);
	else
		ret = fix_logout_send(session, text);

	if (ret)
		fprintf(stderr, "FIX client Logout FAILED\n");
	else
		fprintf(stdout, "FIX client Logout OK\n");

	return ret;
}

static unsigned long fix_new_order_single_fields(struct fix_session *session, struct fix_field *fields)
{
	unsigned long nr = 0;

	fields[nr++] = FIX_STRING_FIELD(TransactTime, session->str_now);
	fields[nr++] = FIX_STRING_FIELD(ClOrdID, "22ee5f42-287c-11e8-b467-0ed5f89f718b");
	fields[nr++] = FIX_STRING_FIELD(Symbol, "BTC-USD");
	fields[nr++] = FIX_FLOAT_FIELD(OrderQty, 1);
	fields[nr++] = FIX_STRING_FIELD(OrdType, "1");
	fields[nr++] = FIX_STRING_FIELD(Side, "1");
	//fields[nr++] = FIX_FLOAT_FIELD(Price, 100);
	//fields[nr++] = FIX_STRING_FIELD(TimeInForce, "1");

	return nr;
	
}

void fix_new_order_single(void){
	fields = calloc(FIX_MAX_FIELD_NUMBER, sizeof(struct fix_field));
	nr = fix_new_order_single_fields(session, fields);
	fix_session_new_order_single(session, fields, nr);
}


void fix_client_run(void)
{
	struct fix_session_cfg cfg;	
	struct sockaddr_in sa;
	int saved_errno = 0;
	struct hostent *he;
	char **ap;

	const char *host = "127.0.0.1";
	int port = 4197;
	enum fix_version version = FIX_4_2;
	fix_session_cfg_init(&cfg);
	cfg.dialect	= &fix_dialects[version];	
	cfg.target_comp_id = "Coinbase";
	cfg.sender_comp_id = "6777f39458390bbf84ed25060bddc72d";
	cfg.api_secret = "VKSPYLpnEiuRRi4pOzrpgfozzfQxaZh+wGpZlEqbSA9Uyys/KWY9BHwfb7vIltHiPklju0hgCUkv9jD7/4gGHQ==";
	cfg.password = "ximndivkdra";
	cfg.heartbtint = 30;

	struct timespec cur, prev;
	struct fix_message *msg;
	
	
	int ret = -1;
	int diff;
	//int heartbeat_count = 0;
	
	he = gethostbyname(host);
	if (!he)
		error("Unable to look up %s (%s)", host, hstrerror(h_errno));

	for (ap = he->h_addr_list; *ap; ap++) {
		cfg.sockfd = socket(he->h_addrtype, SOCK_STREAM, IPPROTO_TCP);
		if (cfg.sockfd < 0) {
			saved_errno = errno;
			continue;
		}

		sa = (struct sockaddr_in) {
			.sin_family		= he->h_addrtype,
			.sin_port		= htons(port),
		};
		memcpy(&sa.sin_addr, *ap, he->h_length);

		if (connect(cfg.sockfd, (const struct sockaddr *)&sa, sizeof(struct sockaddr_in)) < 0) {
			saved_errno = errno;
			close(cfg.sockfd);
			cfg.sockfd = -1;
			continue;
		}
		break;
	}

	if (cfg.sockfd < 0)
		error("Unable to connect to a socket (%s)", strerror(saved_errno));

	if (socket_setopt(cfg.sockfd, IPPROTO_TCP, TCP_NODELAY, 1) < 0)
		die("cannot set socket option TCP_NODELAY");




	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		fprintf(stderr, "Unable to register a signal handler\n");
		goto exit;
	}
	
	session	= fix_session_new(&cfg);
	if (!session) {
		fprintf(stderr, "FIX session cannot be created\n");
		goto exit;
	}

	ret = fix_session_logon(session);
	if (ret) {
		fprintf(stderr, "FIX client Logon FAILED\n");
		goto exit;
	}
	
	
	fprintf(stdout, "FIX client Logon OK\n");
	
	
	
	clock_gettime(CLOCK_MONOTONIC, &prev);
	
	while (!stop && session->active) {
		
		/*
		if (sendOrder){		
			fix_session_new_order_single(session, fields, nr);
			sendOrder = false;		
		}*/		

		clock_gettime(CLOCK_MONOTONIC, &cur);
		diff = cur.tv_sec - prev.tv_sec;
		
		if (diff > 0.1 * session->heartbtint) {
			prev = cur;

			if (!fix_session_keepalive(session, &cur)) {
				stop = 1;
				break;
			}
		}
		

		if (fix_session_time_update(session)) {
			stop = 1;
			break;
		}
		
		if (fix_session_recv(session, &msg, FIX_RECV_FLAG_MSG_DONTWAIT) <= 0) {
					
			fprintmsg(stdout, msg);
			
			if (fix_session_admin(session, msg))
				continue;
			
			switch (msg->type) {
			case FIX_MSG_TYPE_LOGOUT:
				stop = 1;
				break;
			default:
				stop = 1;
				break;
			}
		}
		
		fprintmsg(stdout, msg);
		//heartbeat_count = heartbeat_count + 1;		
	}

	if (session->active) {
		ret = fix_session_logout(session, NULL);
		if (ret) {
			fprintf(stderr, "FIX client Logout FAILED\n");
			goto exit;
		}
	}

	fprintf(stdout, "FIX client Logout OK\n");

exit:
	fix_session_free(session);

	shutdown(cfg.sockfd, SHUT_RDWR);

	if (close(cfg.sockfd) < 0)
		die("close");

	//return ret;
}






/*
int main(int argc, char *argv[])
{	
	printf("Hello World!\n");
	int ret = fix_client_session();
	return ret;
}
*/
