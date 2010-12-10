#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

typedef struct {
    time_t when;
    unsigned int count;
    void* next;
} history_s;

int save_to_history(unsigned int, time_t);
int output_time_with_text(char*);
int output_last_waffel();
int output_uptime();
int waffelloop(FILE*);
void alert(unsigned int);
void mocp_start();
void mocp_stop();

void* udplisten(void*);
int qWaffelCount();
struct tm* qLastWaffel();
int blubber;
int print_greetings();
int print_datetime();
void print_help();
int start_alsamixer();
int show_plot();

history_s* history = NULL;

#define BUFFSIZE 255
#define PORT 1042

int main(void)
{
    FILE* infile = NULL;
    int err = 0;
    pthread_t udplistener;
    int udplistener_rv;

    history = (history_s*) malloc(sizeof(history_s));
    if (history == NULL)
	return printf("malloc failed! %d", __LINE__), -1;
    if (save_to_history(0, 0) == -1)
	return printf("save_to_history failed! %d", __LINE__), -1;

    mocp_start();

    output_time_with_text("Begin: ");

    print_greetings();

    infile = fopen("waffel.input", "r");
    if (infile != NULL)
    {
	printf("reading history...\n");
	waffelloop(infile);
	fclose(infile);
    }
    udplistener_rv = pthread_create(&udplistener, NULL, udplisten, NULL);
    print_help();
    err = waffelloop(stdin);
    if (err != 0)
	printf("error in line %d\n", err);

    free(history);
    mocp_stop();

    return 0;
}

int save_to_history(unsigned int count, time_t when)
{
    history_s* new_history = NULL;

    history->when = when;
    history->count = count;

    if (count > 0)
    {
	struct tm* thetime;
	time_t now = when;

	thetime = localtime(&now);

	printf("%3d Waffeln verkauft: %s", count, asctime(thetime));
    }

    new_history = (history_s*) malloc(sizeof(history_s));
    if (new_history == NULL)
	return -1;
    new_history->next = history;
    history = new_history;
    history->when = 0;
    history->count = 0;

    return ((history_s*) history->next)->count;
}

int output_time_with_text(char* text)
{
    struct tm* thetime;
    time_t now;
    now = time(NULL);

    thetime = localtime(&now);
    printf("%s%s\n", text, asctime(thetime));

    return 0;
}

int output_last_waffel()
{
    struct tm* thetime;
    time_t now;
    now = ((history_s*)history->next)->when;

    thetime = localtime(&now);
    printf("Letzte Waffel: %s\n", asctime(thetime));

    return 0;
}

int waffelloop(FILE* infile)
{
    char* buffer;
    const int buffer_size = 32;
    FILE* outfile = NULL;

    buffer = (char*) malloc(buffer_size * sizeof(char));
    if (buffer == NULL)
	return __LINE__;

    if (infile == stdin)
    {
	outfile = fopen("waffel.input", "a");
	if (outfile == NULL)
	    return __LINE__;
    }

    while (1)
    {
	int waffel_count = 0;
	time_t when=0;

	if (infile == stdin)
	    printf("Waffel $ ");
	fflush(stdout);
	if (fgets(buffer, buffer_size, infile) == NULL)
	    break;

	if (buffer[0] == '\n')
	{
	    buffer[0] = '1';
	    buffer[1] = '\n';
	}
	if (infile != stdin)
	{
	    int when_int;
	    sscanf(buffer, "%d %d", &waffel_count, &when_int);
	    when = when_int;
	}
	else
	{
	    sscanf(buffer, "%d", &waffel_count);
	    when = time(0);
	    fprintf(outfile, "%3d %d\n", waffel_count, (int)when);
	    fflush(outfile);
	}

	if (waffel_count == 0)
	{
	    if (buffer[0] == 'x' || buffer[0] == 'X')
		break;
	    if (buffer[0] == 'a' || buffer[0] == 'A')
		start_alsamixer();
	    if (buffer[0] == 'h' || buffer[0] == 'H')
		print_help();
	    if (buffer[0] == '?')
		print_help();
	    if (buffer[0] == 's' || buffer[0] == 'S')
		output_uptime();
	    if (buffer[0] == 'l' || buffer[0] == 'L')
		output_last_waffel();
	    if (buffer[0] == 'n' || buffer[0] == 'N')
		blubber = system("mocp --next");
	    if (buffer[0] == 'p' || buffer[0] == 'P')
		show_plot();
	    if (buffer[0] == 't' || buffer[0] == 'T')
		print_datetime();
	}
	else
	{
	    save_to_history(waffel_count, when);
	    if (infile == stdin)
	    {
		if (waffel_count > 0)
		    alert(waffel_count);
		printf("\n");
	    }
	}
    }

    if (infile == stdin)
	fclose(outfile);
    
    return 0;
}

int output_uptime()
{
    int load_1  = 0;
    int load_5  = 0;
    int load_15 = 0;
    history_s* hist;
    time_t now = time(0);
    const unsigned int secperwaffel = 30;

    hist = history;
    while (hist != NULL)
    {
	if (now - hist->when < 1*60)
	    load_1+=hist->count;
	if (now - hist->when < 5*60)
	    load_5+=hist->count;
	if (now - hist->when < 15*60)
	    load_15+=hist->count;
	hist = hist->next;
    }
    printf("Waffel Load (1,5,15 min): %4.2f %4.2f %4.2f\n", load_1/(1.0*60/secperwaffel), load_5/(5.0*60/secperwaffel), load_15/(15.0*60/secperwaffel));

    return 0;
}

void mocp_start()
{
    FILE* test;

    blubber = system("mocp --server 2>/dev/null");
    blubber = system("mocp --clear");
    blubber = system("mocp --on shuffle,r,n");

    test = fopen("liste1.m3u", "r");
    if (test == NULL)
	return;
    else
	fclose(test);

    blubber = system("mocp --play --append liste1.m3u");
}

void mocp_stop()
{
    blubber = system("mocp --stop");
}

void alert(unsigned int count)
{
    FILE* test;
    unsigned int i;

    test = fopen("danke.wav", "r");
    if (test == NULL)
	return;
    else
	fclose(test);

    blubber = system("mocp --pause");
    usleep(450000);

    if (count > 3)
	count--;
    if (count > 4)
	count = 4;

    for (i=0; i<count; i++)
	blubber = system("aplay danke.wav 2>/dev/null");

    usleep(500000);
    blubber = system("mocp --unpause");
}

void* udplisten(void* nueksch)
{
    int sock;
    struct sockaddr_in echoserver;
    struct sockaddr_in echoclient;
    char buffer[BUFFSIZE];
    unsigned int clientlen, serverlen;
    int received = 0;
    char* answer;

    answer = (char*) malloc(sizeof(char) * BUFFSIZE);
    if (answer == NULL)
    {
	fprintf(stderr, "malloc failed @ line %d\n", __LINE__);
	exit(-1);
    }

    /* Create the UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
	fprintf(stderr, "Failed to create socket\n");
    }
    /* Construct the server sockaddr_in structure */
    memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
    echoserver.sin_family = AF_INET;                  /* Internet/IP */
    echoserver.sin_addr.s_addr = htonl(INADDR_ANY);   /* Any IP address */
    echoserver.sin_port = htons(PORT);                /* server port */

    /* Bind the socket */
    serverlen = sizeof(echoserver);
    if (bind(sock, (struct sockaddr *) &echoserver, serverlen) < 0)
    {
	fprintf(stderr, "Failed to bind server socket\n");
    }

    /* Run until cancelled */
    while (1)
    {
	/* Receive a message from the client */
	clientlen = sizeof(echoclient);
	if ((received = recvfrom(sock, buffer, BUFFSIZE, 0,
			(struct sockaddr *) &echoclient,
			&clientlen)) < 0)
	{
	    fprintf(stderr, "Failed to receive message\n");
	}
	buffer[received] = '\0';

	sprintf(answer, "---");
	if (strcmp(buffer, "qWaffelCount") == 0)
	    sprintf(answer, "Bisher verkaufte Waffeln: %d\n", qWaffelCount());
	if (strcmp(buffer, "qSpendenSumme") == 0)
	    sprintf(answer, "Aktuelle Spendensumme: %.2f Euro\n", 0.42*qWaffelCount());
	if (strcmp(buffer, "qLastWaffel") == 0)
	    sprintf(answer, "Letzte Waffel: %s", asctime(qLastWaffel()));
	if (strcmp(buffer, "qSong") == 0)
	{
	    FILE* fpipe;
	    char* command="mocp -i | grep ^Title";
	    char line[256];

	    if ( !(fpipe = (FILE*)popen(command, "r")) )
	    {
		perror("Problems with pipe");
		exit(1);
	    }

	    while ( fgets( line, sizeof line, fpipe))
	    {
		sprintf(answer, "Aktuelles Lied: %s", line);
	    }
	    pclose(fpipe);
	}

	/*
	fprintf(stderr, "Client connected: %s\n", inet_ntoa(echoclient.sin_addr));
	fprintf(stderr, "Q: %s\n", buffer);
	fprintf(stderr, "A: %s\n", answer);
	*/

	/* Send the message back to client */
	sendto(sock, answer, strlen(answer), 0, (struct sockaddr *) &echoclient, sizeof(echoclient));
    }

    return NULL;
}

int qWaffelCount()
{
    history_s* hist;
    int count = 0;

    hist = history;
    while (hist != NULL)
    {
	count+=hist->count;
	hist = hist->next;
    }
    return count;
}

struct tm* qLastWaffel()
{
    struct tm* thetime;
    time_t now;
    now = ((history_s*)history->next)->when;
    thetime = localtime(&now);

    return thetime;
}

int print_greetings()
{
    return system("cat logo");
}

void print_help()
{
    printf("enter NUMBER and press enter to sell NUMBER many Waffeln\n");
    printf("\n");
    printf("enter a or A for [A]lsamixer\n");
    printf("enter h, H or ? for this [H]elp\n");
    printf("enter l or L to print [L]ast Waffel\n");
    printf("enter n or N to play [N]ext song\n");
    printf("enter p or P to show [P]lot\n");
    printf("enter s or S to print [S]tatistics\n");
    printf("enter t or T to show date[T]ime\n");
    printf("enter x or X to e[X]it\n");
    printf("press ctrl + alt + del (strg + alt + entf) to shutdown\n");
    printf("\n");
}

int start_alsamixer()
{
    return system("alsamixer");
}

int show_plot()
{
    return system("gnuplot < gnu.plot");
}

int print_datetime()
{
    return system("date");
}

