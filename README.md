# c_TCP_demo

## run
```
gcc chat_client.c -o client
gcc chat_server.c -o server
```
### 服务端
```
./server
```
### 客户端
```
./client
```

## 基本功能
1、用户登录其他在线用户可以收到该用户的登录信息
2、用户发消息，其他在线用户可以收到
3、用户退出，其他用户可以收到消息
4、服务器可以发送系统消息



### 简单说明
由于accept监控bug懒得调了，直接在recvfrom后进行客户端保存(笑
这毋庸置疑会影响性能，但是基本功能做出来了


