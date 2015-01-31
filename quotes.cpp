#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <ctime>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <vector>
#include <string>
#include <iostream>
#include <sstream>

std::vector<std::string> g_quotes = {
	"To infinity and beyond!",
	"Space, the final frontier.",
	"Live long and prosper.",
	"What's your favourite thing about space? Mine is space!",
	"The Earth is the cradle of humanity, but mankind cannot stay in the cradle "
		"forever.",
	"Spaaaaaaaaace!",
	"Space Space wanna go to Space!",
	"Space is big. Really big. You just won't believe how vastly, hugely, "
		"mind-bogglingly big it is. I mean, you may think it's a long way down the "
		"road to the chemist's, but that's just peanuts to space.",
	"Space. It seems to go on and on forever. Then you get to the end, and a "
		"monkey starts throwing barrels at you.",
	"No creature loves an empty space // Their bodies measure out their place.",
	"You'd better be prepared for the jump into hyper-space. It's unpleasantly "
		"like being drunk.",
	"Matter tells Space how to curve, and Space tells Matter how to move.",
	"We aim to misbehave.",
	"We're not just doing it for money... We're doing it for a shitload of "
		"money!",
	"May the Schwartz be with you!",
	"Space is hard - but worth it. We will persevere and move forward together.",
	"That's one small step for man; one giant leap for mankind.",
	"Failure is not an option.",
	"»Once the rockets are up, who cares where they come down? That's not my "
		"department«, says Wernher von Braun.",
	"Make it so.",
	"Engage.",
	"Things are only impossible until they're not!",
	"The sky's the limit.",
	"There is a great disturbance in the Force.",
	"Try not. Do... or do not. There is no try.",
	"No different! Only different in your mind. You must unlearn what you have "
		"learned.",
	"You must complete the training!",
	"Any sufficiently advanced technology is indistinguishable from magic.",
	"The Earth is just too small and fragile a basket for the human race to keep "
		"all its eggs in.",
	"Space is to place as eternity is to time.",
	"Every cubic inch of space is a miracle.",
	"To go places and do things that have never been done before—that's what "
		"living is all about.",
	"To confine our attention to terrestrial matters would be to limit the human "
		"spirit.",
	"Human interest in exploring the heavens goes back centuries. This is what "
		"human nature is all about.",
	"I didn't care if I was first, 50th, or 500th in space. I just wanted to go.",
	"The next time I go into space, I'll be able to take my family with me.",
	"You're in charge but don't touch the controls.",
	"It's just a bunch of junk up there.",
	"One test result is worth one thousand expert opinions.",
	"Space isn't remote at all. It's only an hour's drive away, if your car could"
		" go straight upwards.",
	"Going to space is going to be outstanding. I can't believe that's actually "
		"happening.",
	"In space, race doesn't matter, nationality doesn't matter. In space, you see"
		" the world as a globe and you don't see the boundaries.",
	"Space is for everybody. It's not just for a few people in science or math, "
		"or for a select group of astronauts. That's our new frontier out there, "
		"and it's everybody's business to know about space.",
	"You are star-stuff.",
};

int main(int argc, char ** argv) {
	if(argc < 2 || argc >= 2 &&
			(strncmp(argv[1], "--help", 6) == 0 || strncmp(argv[1], "-h", 2) == 0)) {
		std::cout << "Usage: quotes <port> | --help | -h" << std::endl;
		return 0;
	}

  addrinfo hints, * ai_res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
  int fd = getaddrinfo(NULL, argv[1], &hints, &ai_res);
	if(fd < 0) {
		std::cerr << gai_strerror(fd) << std::endl;
		return -1;
	}

	std::string errfn;

	for(addrinfo * ai = ai_res; ai != NULL; ai = ai->ai_next) {
		if(fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol) < 0) {
			errfn = "socket";
			continue;
		}

		int yes = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

		if(bind(fd, ai->ai_addr, ai->ai_addrlen) != 0) {
			errfn = "bind";
			close(fd);
			continue;
		} else {
			if(listen(fd, 10) < 0) {
				errfn = "listen";
				close(fd);
				continue;
			}

			std::ostringstream oss;
			char addr[INET6_ADDRSTRLEN];
			memset(addr, 0, sizeof(addr));

			switch(ai->ai_addr->sa_family) {
				case AF_INET6: {
					sockaddr_in6 * sin6 = ((sockaddr_in6 *) ai->ai_addr);
					inet_ntop(sin6->sin6_family, &sin6->sin6_addr, addr, sizeof(addr));
					oss << "[" << addr << "]:" << ntohs(sin6->sin6_port);
					break;
				}
				case AF_INET: {
					sockaddr_in * sin = ((sockaddr_in *) ai->ai_addr);
					inet_ntop(sin->sin_family, &sin->sin_addr, addr, sizeof(addr));
					oss << addr << ":" << ntohs(sin->sin_port);
					break;
				}
			}

			std::cout << "Listening on " << oss.str() << std::endl;
			break;
		}
	}

	freeaddrinfo(ai_res);

	if(fd < 0) {
		std::cerr << "Could not bind to port: " << errfn << strerror(errno)
			<< std::endl;
		return -1;
	}

	srand(time(NULL));
	while(true) {
		int ret;
		int newfd = accept(fd, NULL, NULL);
		char buf[1024];

		while(recv(newfd, &buf, sizeof(buf), MSG_DONTWAIT) == -1 &&
				(errno == EAGAIN || errno == EWOULDBLOCK)) {}

		std::string quote = g_quotes[rand() % g_quotes.size()];
		snprintf(buf, sizeof(buf),
				"HTTP/1.1 200 OK\r\n"
				"Server: space-quotes/0.0.1\r\n"
				"Content-Type: text/plain; charset=utf-8\r\n"
				"Content-Length: %d\r\n"
				"Connection: close\r\n\r\n%s",
				quote.size(),
				quote.c_str()
			);
		send(newfd, buf, sizeof(buf), 0);
		close(newfd);
	}

	return 0;
}
