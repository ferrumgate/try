#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/netfilter_ipv4.h>

#define DEFAULT_PORT 9000
#define BUFFER_SIZE 1024

void crash(const char *msg) {
  perror(msg);
  exit(1);
}

void handle_message(int sockfd) {
  struct sockaddr_in client_addr;
  struct msghdr msg;
  struct iovec iov;
  char buffer[BUFFER_SIZE];
  char control[CMSG_SPACE(sizeof(struct in_pktinfo))];
  struct cmsghdr *cmsg;
  struct in_pktinfo *pktinfo;

  iov.iov_base = buffer;
  iov.iov_len = sizeof(buffer);

  msg.msg_name = &client_addr;
  msg.msg_namelen = sizeof(client_addr);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = control;
  msg.msg_controllen = sizeof(control);
  msg.msg_flags = 0;

  ssize_t nread = recvmsg(sockfd, &msg, 0);
  if (nread == -1) {
    perror("recvmsg");
    return;
  }

  for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
    if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_PKTINFO) {
      pktinfo = (struct in_pktinfo *)CMSG_DATA(cmsg);
      char dest_ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &pktinfo->ipi_addr, dest_ip, INET_ADDRSTRLEN);
      std::cout << "Destination IP: " << dest_ip << std::endl;
      break;
    }
  }

  char client_ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
  std::cout << "Client IP: " << client_ip << std::endl;
  std::cout << "Client Port: " << ntohs(client_addr.sin_port) << std::endl;

  // Extract the destination port from the sockaddr_in structure
  struct sockaddr_in *dest_addr = (struct sockaddr_in *)msg.msg_name;
  int dest_port = ntohs(dest_addr->sin_port);
  std::cout << "Destination Port: " << dest_port << std::endl;

  // Process the received data
  std::cout << "Received data: " << std::string(buffer, nread) << std::endl;

  // Send a response
  char response[BUFFER_SIZE];

  sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
}

int main() {
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) crash("couldn't allocate socket");

  const int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) crash("SO_REUSEADDR failed");
  if (setsockopt(sockfd, IPPROTO_IP, IP_PKTINFO, &yes, sizeof(yes)) < 0) crash("IP_PKTINFO failed");

  struct sockaddr_in bind_addr;
  bind_addr.sin_family = AF_INET;
  bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind_addr.sin_port = htons(DEFAULT_PORT);
  if (bind(sockfd, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) crash("bind failed");

  std::cout << "Listening on port " << DEFAULT_PORT << " for redirected UDP traffic" << std::endl;

  while (true) {
    handle_message(sockfd);
  }

  close(sockfd);
  return 0;
}