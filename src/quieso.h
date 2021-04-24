/* This file carries all functions declaration and structure definations */
struct thread_opts {
	char host[INET_ADDRSTRLEN];
	unsigned int port, timeout, thread_id, start, end;
};

int quieso_error(const char *s, int sock);

void *worker(void *thread_args);

int scanner(const char * host, unsigned int *port, unsigned int timeout, unsigned int *start, unsigned int *end);