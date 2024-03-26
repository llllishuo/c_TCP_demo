#include <stdio.h>  
#include <pthread.h>
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
  
#define SERVER_PORT 8888  
#define MAX_BUFFER 1024  
int server_fd;  
struct sockaddr_in server_addr;  
struct sockaddr_in client_list[10];
int clients_size = 0;
/*
 * 检查client_list是否含有目标客户端
 * */
int lookup_client(int client_addr){
  for (int i = 0; i < clients_size; i++){
    if(ntohs(client_list[i].sin_port) == client_addr){  
      return 1;
    }
  }
  return 0;
}
/*
 * 发送信息到目标客户端
 * */
void send_msg(const char *msg, struct sockaddr_in client_addr){
  ssize_t n;  
  n = sendto(server_fd, msg, strlen(msg), 0, &client_addr, sizeof(client_addr));
  if (n == -1) {  
      perror("sendto");  
      exit(EXIT_FAILURE);  
  }  
}
/*
 * 向所有接收到的客户端发送数据
 * */
void send_to_all(const char *msg){
  for (int i = 0 ; i < clients_size; i++){
    send_msg(msg, client_list[i]);
  }
}
/*
 * 接收数据并保存客户端地址和端口到client_list
 * */
void receive(){
  struct sockaddr_in client_addr;
  ssize_t n;  
  static char buffer[MAX_BUFFER] = "";
  socklen_t client_len = sizeof(client_addr);  
  n = recvfrom(server_fd, buffer, MAX_BUFFER, 0, (struct sockaddr*)&client_addr, &client_len);  
  if(n <= 0) return;
  // 检索并保存客户端数据
  if(lookup_client(ntohs(client_addr.sin_port)) == 0){
    client_list[clients_size] = client_addr;
    clients_size++;
  }
  buffer[n] = '\0';  
  
  send_to_all(buffer);
  
  printf("\n%s\nsay: \n", buffer);  
  
}
/*
 * 发送公告到所有客户端
 * */
void say(){
  static char buffer[MAX_BUFFER] = "test";
  printf("\nsay: ");  
  fgets(buffer, MAX_BUFFER, stdin);  
  printf("[Debug] msg: %s", buffer);
  buffer[strcspn(buffer, "\n")] = 0;
  static char msg[MAX_BUFFER] = "[Notification]: ";
  strcat(msg, buffer);
  send_to_all(msg);
  printf("\nsay: ");  

  
}
/*
 * 接收线程
 * */
void *receive_from_cliend(){
  printf("[Debug] receive ready...\n");
  while(1){
    receive();
  }
}
/*
 * 公告发送线程
 * */
void *say_thread(){
  printf("[Debug] say ready...\n");
  while(1){
    say();
  }
}
int main() {  
    char buffer[MAX_BUFFER];  
    pthread_t receive_thread, send_thread, accept_thread;
    int opt = 1;
    // 创建UDP套接字  
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {  
        perror("socket");  
        exit(EXIT_FAILURE);  
    }  
    // 设置socket选项  
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {  
        perror("setsockopt");  
        exit(EXIT_FAILURE);  
    }
  
    // 设置服务器地址结构  
    memset(&server_addr, 0, sizeof(server_addr));  
    server_addr.sin_family = AF_INET;  
    server_addr.sin_addr.s_addr = INADDR_ANY;  
    server_addr.sin_port = htons(SERVER_PORT);  
  
    // 绑定套接字到端口  
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {  
        perror("bind");  
        exit(EXIT_FAILURE);  
    }  
    //listen(server_fd, 3);
  
    printf("Server started on port %d\n", SERVER_PORT);  

    pthread_create(&receive_thread, NULL, receive_from_cliend, NULL);
    pthread_create(&send_thread, NULL, say_thread, NULL);
    //say_thread();
    pthread_join(send_thread, NULL);
    pthread_join(receive_thread, NULL);

  
    close(server_fd);  
    return 0;  
}
