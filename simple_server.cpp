/* run using ./server <port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <threads.h>
#include <vector>
#include <signal.h>

#include <sys/stat.h>
#include <netinet/in.h>
#include <fstream>
#include <sstream>
#include "http_server.hh"

#include <vector>

#include <sys/stat.h>

#include <fstream>
#include <sstream>
#include <pthread.h>
using namespace std;
#include <queue>

#define NUM_THREADS 100
#define THREAD_ARRAY_SIZE NUM_THREADS

pthread_t arr[THREAD_ARRAY_SIZE];

int flag = 0;

int QUEUE_SIZE = 10000;
int count = 0;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty_q = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill_q = PTHREAD_COND_INITIALIZER;
int ptr_fill = 0;
int ptr_use = 0;
std::queue<int> q;

void interrupt_handler(int);

void error(char *msg)
{
  perror(msg);
  exit(1);
}

void *process_packets(void *i)
{

  pthread_t ptid = pthread_self();

  int newsockfd = -1;

  while (1)
  {
    pthread_mutex_lock(&mut);
    while (q.empty() && flag == 0)
    {
      pthread_cond_wait(&fill_q, &mut);
    }
    if (flag)
    {
      pthread_mutex_unlock(&mut);
      pthread_exit(NULL);
      break;
    }
    newsockfd = q.front();
    q.pop();
    pthread_cond_signal(&empty_q);

    pthread_mutex_unlock(&mut);

    char buffer[1024];

    int n;

    bzero(buffer, 1024);
    n = read(newsockfd, buffer, 1024);
    if (n < 0)
    {
      close(newsockfd);
      perror("ERROR reading from socket");
      continue;
    }
    if (n == 0)
    {
      close(newsockfd);

      continue;
    }

    /* send reply to client */
    HTTP_Response *res = handle_request(buffer);

    n = write(newsockfd, (res->get_string()).c_str(), res->get_string().length());
    if (n < 0)
      perror("ERROR writing to socket");

    delete res;

    close(newsockfd);
  }

  pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  signal(SIGINT, interrupt_handler);
  //  cout<<"main nnnectionnssss....\n";
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  // char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n;

  if (argc < 2)
  {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  /* create socket */

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  /* fill in port number to listen on. IP address can be anything (INADDR_ANY)
   */
  // error("Accepting connnesdsd3-4343ctionnssss....");

  bzero((char *)&serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  /* bind socket to this port number on this machine */

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");
  /* listen for incoming connection requests */
  listen(sockfd, 10000);
  clilen = sizeof(cli_addr);

  /* accept a new request, create a newsockfd */

  for (int i = 0; i < NUM_THREADS; i++)
  {
    pthread_t thread_id;

    pthread_create(&arr[i], NULL, process_packets, NULL);

  }

  while (1)
  {
    /* code */

    int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      error("ERROR on accept");

    pthread_mutex_lock(&mut);
    while (q.size() == QUEUE_SIZE)
    {
      pthread_cond_wait(&empty_q, &mut);
    }
    q.push(newsockfd);
    pthread_cond_signal(&fill_q);
    pthread_mutex_unlock(&mut);
  }
  return 0;
}

void interrupt_handler(int sig)
{
  flag = 1;

  pthread_cond_broadcast(&fill_q);

  for (int i = 0; i < THREAD_ARRAY_SIZE; i++)
  {
      pthread_cancel(arr[i]);
      pthread_join(arr[i], NULL);
  }

  exit(0);
}
