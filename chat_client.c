#include <stdio.h>  
#include <pthread.h>
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
  
#define SERVER_IP "127.0.0.1"  
#define SERVER_PORT 8888  
#define MAX_BUFFER 1024  
int client_fd;  
struct sockaddr_in server_addr;  
  
struct User{
  char username[MAX_BUFFER];
  char password[MAX_BUFFER];
}User;


/*
 * 发送信息到服务端
 * */
void send_msg(const char *msg){
  ssize_t n;  
  n = sendto(client_fd, msg, strlen(msg), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));  
  if (n == -1) {  
      perror("sendto");  
      exit(EXIT_FAILURE);  
  }  
}
/*
 * 信息输入并发送
 * */
void say(struct User user){
  static char buffer[MAX_BUFFER];
  printf("say: ");  
  fgets(buffer, MAX_BUFFER, stdin);  
  buffer[strcspn(buffer, "\n")] = 0;
  static char msg[MAX_BUFFER];
  strcpy(msg, user.username);
  strcat(msg, ": ");
  strcat(msg, buffer);
  send_msg(msg);

}
/*
 * 登录认证
 * */
struct User login(){
  struct User user;
  printf("username: ");
  fgets(user.username, MAX_BUFFER, stdin);  
  user.username[strcspn(user.username, "\n")] = 0;
  
  printf("password: ");
  fgets(user.password, MAX_BUFFER, stdin);  
  
  static char msg[MAX_BUFFER] = "New client: ";
  strcat(msg, user.username);
  send_msg(msg);

  return user;
}

/*
 * 接收服务端发送的信息
 * */
void connect_server_msg(){
  static char buffer[MAX_BUFFER];
  ssize_t n;  
  socklen_t server_len = sizeof(server_addr);  
  n =  read(client_fd, buffer, MAX_BUFFER); 
  if(n == 0) return;
  buffer[n] = '\0';  
  printf("\n%s\nsay: ", buffer);  
}

/*
 * 信息接收线程
 * */
void *receive(){
  //printf("[Debug] receive ready...\n");
  while (1) {
    connect_server_msg();
  }
}
/*
 * 信息发送线程
 * */
void *say_thread(struct User user){
    while (1) {  
      say(user);
    }  
}


int main() {  
    char buffer[MAX_BUFFER];  
  
    pthread_t receive_thread, send_thread;
    // 创建UDP套接字  
    if ((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {  
        perror("socket");  
        exit(EXIT_FAILURE);  
    }  
  
    // 设置服务器地址结构  
    memset(&server_addr, 0, sizeof(server_addr));  
    server_addr.sin_family = AF_INET;  
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);  
    server_addr.sin_port = htons(SERVER_PORT);  
  
    struct User user = login();

    pthread_create(&receive_thread, NULL, receive, NULL);
    pthread_create(&send_thread, NULL, say_thread(user), NULL);

    pthread_join(send_thread, NULL);
    pthread_join(receive_thread, NULL);


    close(client_fd);  
    return 0;  
}
