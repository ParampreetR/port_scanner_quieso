/* This file carries all functions and other stuff responsible for parsing aruments */

#include <argp.h>
#include <stdlib.h>
#include <error.h>

struct arguments
{
    char host[INET_ADDRSTRLEN];           /* hostname or IP to scan */
    int timeout;          /* timeout for each port */
    int version;          /* a flag for '-v' to check version */
    char file_to_output[30]; /* output to file */
};

struct argp_option options[] = {
    {"host", 'h', "HOST", 0, "Target host to scan" },
    {"timeout", 't', "SECONDS", 0, "Speed of scanning aka seconds of timeout." },
    {"output",   'o', "FILE",  0, "Output to FILE instead of standard output" },
    {"version",    'v',  0, 0, "Print version and exit"},
    { 0 }
};

char doc[] =
    "quieso is a simple port scanner that is intended to show some logic behind port scanning.\
\vThis is still in devlopment, more things will be added soon";

/* A description of the arguments we accept. */
char args_doc[] = "";

/* Keys for options without short-options. */
#define OPT_ABORT  1            /* –abort */


error_t parse_opt (int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;

    switch (key)
    {
        case 'h':
            strncpy(arguments->host, arg, (size_t) INET_ADDRSTRLEN);
            break;
        case 't':
            //printf("%s", arg);
            arguments->timeout = atoi(arg);
            break;
        case 'o':
            strncpy(arguments->file_to_output, arg, 30);
            break;
        case 'v':
            arguments->version = 1;
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

struct argp argp = { options, parse_opt, args_doc, doc };


struct arguments *parse_args(int argc, char *argv[]) {
    static struct arguments args;
    argp_parse (&argp, argc, argv, 0, 0, &args);
    return &args;
}