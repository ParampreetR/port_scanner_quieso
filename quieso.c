#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "src/quieso.h"
#include "src/arg_parse.h"

#define MAX_THREADS 500 /* Total Threads */


/*
 * This function is a core of this port scanner. It scans a port specified in port parameter.
 * That parameter can be changed after passing to process second port to scan. Most important
 * part of this function is we are setting socket on non blocking and waiting if we got a 
 * permission to write on socket.
 */
int scanner(const char * host, unsigned int *port, unsigned int timeout, unsigned int *start, unsigned int *end)
{
	// This struct has all information which is required to connect to target
	struct sockaddr_in address, bind_addr;
	// This struct is used in select(). It contains timeout information.
	struct timeval tv;
	fd_set write_fds;
	socklen_t so_error_len;
	// The socket descriptor, error status and yes.
	int sd, so_error = 1, yes = 1;
	
	int write_permission;

	// Wait until start flag is not enabled by main process
	while(!*start) {
		sleep(2);	/* Wait for 2 seconds */
	}

	// Process until end flag is not set by main process
	while(!*end) {
		// Wait for 2 seconds till port is 0
		while(*port == 0) {
			sleep(2);
		}

		// Fill sockaddr_in struct. Refer to online c documentation for details.
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = inet_addr(host);	/* inet_addr() converts string of host IP to int */
		address.sin_port = htons(*port);	/* htons() returns int with data set as big endian. Most computers follow little endian and network devices only know big endian. */
		
		// Seconds to timeout
		tv.tv_sec = timeout;
		// Microseconds to timeout
		tv.tv_usec = 0;

		FD_ZERO(&write_fds);

		so_error_len = sizeof(so_error);

		// Create a socket
		if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			return quieso_error("socket() An error has occurred", 0);
		

		// Set port as reuseable. So we may not use up all avilable ports.
		if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
			return quieso_error("setsockopt() An error has occured", 0);

		// Make our socket non-blocking. Program will not stop until connection is made.
		if(fcntl(sd, F_SETFL, O_NONBLOCK) == -1)
			return quieso_error("fcntl() caused error", 1);;

		// Now connect() function will always returns -1 as we are in non-blocking flag.
		if (connect(sd, (struct sockaddr *) &address, sizeof(address)) == -1) {
		
			switch (errno) {
				case EWOULDBLOCK:	/* Processing going on */
				case EINPROGRESS:	/* Connection in progress */
					break;

				default:			/* We want to give error on every other case */
					return quieso_error("connect() An error has occurred", sd);
			}
		}

		FD_SET(sd, &write_fds);
		
		// Waiting for time when we can write on socket or timeout occurs
		if((write_permission = select(sd + 1, NULL, &write_fds, NULL, &tv)) == -1)
			return quieso_error("select() An error has occurred", sd);

		// If we got write permission
		if(write_permission)
			if(getsockopt(sd, SOL_SOCKET, SO_ERROR, &so_error, &so_error_len) != -1) {
				if(so_error == 0)
					printf("%d OPEN\n", *port);
			}

		// Set port to 0. So we do not process one port again and again
		*port = 0;
	}
}

/*
 * A worket function or thread function that will run beside main. This will be responsible
 * for scanning ports passed by main function to a thread
 */
void *worker(void *thread_opts)	// Be careful, it is void *.
{
	// Create pointer to struct which carries all options passed by main
	struct thread_opts *opts;

	// Now opt will point to thread_opt passed by main
	opts = thread_opts;

	// Call a core function will do entire work of scanning
	scanner(opts->host, &opts->port, opts->timeout, &opts->start, &opts->end);

	// Exit current thread
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	// Declare everything we'll need
	int thread_id;
	pthread_t threads[MAX_THREADS];
	struct thread_opts opts[MAX_THREADS];
	struct arguments *user_args;
	int unsigned port_scan = 1;
	struct hostent *target;

	user_args = parse_args(argc, argv);

	// If user specified -v then print version and exit
	if(user_args->version) {
		printf("scanner v0.1");
		exit(0);
	}

	// If user do not specified host then print error and exit
	if(strlen(user_args->host) == 0) {
		quieso_error("[-] Please specify host\n", 1);
	}

	// Resolve hostname
	target = gethostbyname(user_args->host);

	// Clear out space
	bzero(user_args->host, sizeof(user_args->host));
	// Copy to struct with typecasting
	strcpy(user_args->host , inet_ntoa(*( (struct in_addr *)target->h_addr_list[0] )));
	printf("Scanning %s\n", user_args->host);



	// Create threads that will not do anything until we set opts[thread_id].start = 1
	for(thread_id = 0; thread_id < MAX_THREADS; thread_id++) {
		opts[thread_id].start = 0;	/* Placeholder, we are only creating threads here */
		opts[thread_id].end = 0;	/* threads will check this variable if they should exit or not */
		opts[thread_id].port = 0;	/* Placeholder, we are only creating threads here */
		opts[thread_id].timeout = user_args->timeout;	/* No placeholder, we will not be passing pointer of this timeout variable */
		opts[thread_id].thread_id = thread_id;			/* Assign each thread a ID */
		strncpy(opts[thread_id].host, user_args->host, (size_t) INET_ADDRSTRLEN);	/* Set target host */

		/* Create threads */
		if (pthread_create(&threads[thread_id], NULL, worker, (void *) &opts[thread_id])) {
			#ifdef DEBUGING
				perror("pthread_create() error");	/* Print error in thread creation */
			#endif
			return EXIT_FAILURE;
		}
	}

	thread_id = 0;	
	printf("--> Created %d threads.\n", MAX_THREADS);

	// Loop till over all ports are scanned
	while(port_scan < 65535) {
		/* Iterate through all threads */
		for(int i = 0; i < MAX_THREADS; i++) {
			if(opts[i].port == 0) {
				opts[i].port = port_scan;		/* giving port to each thread to scan */
				port_scan++;					/* Increment by one */
				opts[i].start = 1;				/* Switch red light to green so threads can run */
			}
		}
	}
	/* 
	 * We can use any other approach to ensure all threads are exited but
	 * in our case we are sure that no thread can run more than user_args->timeout + 1.
	 * Still we are doubling time.
	 */
	sleep(user_args->timeout + user_args->timeout); /* ensure all threads had done their work */
}

/*
 * Close socket and print some information if debugging.
 */
int quieso_error(const char *s, int sock)
{
	#ifdef DEBUGING
	perror(s);
	#endif
	if (sock)
		close(sock);
	return 0;
}