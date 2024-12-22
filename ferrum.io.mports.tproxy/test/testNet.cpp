#include <gtest/gtest.h>

#include "fnet.h"

using namespace Ferrum;

TEST(TestNet, ipV4Checksum) {
  uint8_t packet_bytes[] = {0x45, 0x00, 0x00, 0x3c, 0x18, 0x61, 0x40,
                            0x00, 0x40, 0x11, 0xbe, 0x44, 0xc0, 0xa8,
                            0x58, 0xfa, 0x8e, 0xfa, 0xbb, 0x6e};
  iphdr *ip_header = reinterpret_cast<iphdr *>(packet_bytes);
  ip_header->check = 0;
  auto result = FNet::ipChecksum(ip_header);
  EXPECT_EQ(result, ntohs(0xbe44));
}

// uint16_t compute_udp_checksum(struct iphdr *pIph, unsigned short *ipPayload)
// {
//   unsigned long sum = 0;
//   struct udphdr *udphdrp = (struct udphdr *)(ipPayload);
//   unsigned short udpLen = ntohs(udphdrp->len);
//   // printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~udp len=%dn", udpLen);
//   // add the pseudo header
//   // printf("add pseudo headern");
//   // the source ip
//   sum += (pIph->saddr >> 16) & 0xFFFF;
//   sum += (pIph->saddr) & 0xFFFF;
//   // the dest ip
//   sum += (pIph->daddr >> 16) & 0xFFFF;
//   sum += (pIph->daddr) & 0xFFFF;
//   // protocol and reserved: 17
//   sum += htons(IPPROTO_UDP);
//   // the length
//   sum += htons(udpLen);

//   // add the IP payload
//   // printf("add ip payloadn");
//   // initialize checksum to 0
//   udphdrp->check = 0;
//   while (udpLen > 1) {
//     sum += *ipPayload++;
//     udpLen -= 2;
//   }
//   // if any bytes left, pad the bytes and add
//   if (udpLen > 0) {
//     // printf("+++++++++++++++padding: %dn", udpLen);
//     sum += ((*ipPayload) & htons(0xFF00));
//   }
//   // Fold sum to 16 bits: add carrier to result
//   // printf("add carriern");
//   while (sum >> 16) {
//     sum = (sum & 0xffff) + (sum >> 16);
//   }
//   // printf("one's complementn");
//   sum = ~sum;
//   return static_cast<uint16_t>(sum);
// }

// uint16_t udp_checksum(const void *buff, size_t len, in_addr_t src_addr,
//                       in_addr_t dest_addr) {
//   const uint16_t *buf = static_cast<const uint16_t *>(buff);
//   uint32_t sum = 0;
//   uint16_t length = len;

//   // Add the pseudo-header
//   sum += (src_addr >> 16) & 0xFFFF;
//   sum += src_addr & 0xFFFF;
//   sum += (dest_addr >> 16) & 0xFFFF;
//   sum += dest_addr & 0xFFFF;
//   sum += htons(IPPROTO_UDP);
//   sum += htons(length);

//   // Calculate the sum
//   while (len > 1) {
//     sum += *buf++;
//     len -= 2;
//   }

//   // Add the padding if the packet length is odd
//   if (len > 0) {
//     sum += ((*buf) & htons(0xFF00));
//   }

//   // Add the carries
//   while (sum >> 16) {
//     sum = (sum & 0xFFFF) + (sum >> 16);
//   }

//   // Return the one's complement of sum
//   return static_cast<uint16_t>(~sum);
// }

// inline uint32_t checksum(void *buffer, unsigned int count, uint32_t startsum)
// {
//   uint16_t *up = (uint16_t *)buffer;
//   uint32_t sum = startsum;
//   uint32_t upper16;
//   while (count > 1) {
//     sum += *up++;
//     count -= 2;
//   }
//   if (count > 0) {
//     sum += (uint16_t) * (uint8_t *)up;
//   }
//   while ((upper16 = (sum >> 16)) != 0) {
//     sum = (sum & 0xffff) + upper16;
//   }
//   return sum;
// }
// inline uint32_t finish_sum(uint32_t sum) { return ~sum & 0xffff; }

TEST(TestNet, udpChecksum) {
  uint8_t ip_header_bytes[] = {0x45, 0x00, 0x00, 0x35, 0x00, 0x00, 0x40,
                               0x00, 0x39, 0x11, 0x0b, 0xec, 0x8e, 0xfb,
                               0x8d, 0x2e, 0xc0, 0xa8, 0x58, 0xfa};
  uint8_t udp_header_and_data_bytes[] = {
      0x01, 0xbb, 0xa4, 0xe1, 0x00, 0x21, 0x66, 0xb1,  // header
                                                       // data
      0x43, 0xe8, 0xc3, 0x13, 0x2f, 0x97, 0x81, 0x1e, 0xf4, 0x68, 0x3a, 0x71,
      0x50, 0x13, 0x7d, 0x1a, 0x18, 0x2f, 0xa2, 0x1b, 0x0d, 0x29, 0x15, 0x64,
      0x2c};

  iphdr *ip_header = reinterpret_cast<iphdr *>(ip_header_bytes);
  ip_header->check = 0;
  auto ipHeaderChecksum = FNet::ipChecksum(ip_header);
  EXPECT_EQ(ipHeaderChecksum, ntohs(0x0bec));
  udphdr *udp_header =
      reinterpret_cast<struct udphdr *>(udp_header_and_data_bytes);
  auto result = FNet::udpChecksum(ip_header, udp_header);
  EXPECT_EQ(result, ntohs(0x66b1));
}

TEST(TestNet, tcpChecksum) {
  uint8_t ip_header_bytes[] = {0x45, 0x00, 0x00, 0x34, 0x1a, 0x67, 0x40,
                               0x00, 0x40, 0x06, 0x0e, 0xf5, 0xc0, 0xa8,
                               0x58, 0xfa, 0xcc, 0x8d, 0x2b, 0x38};
  uint8_t tcp_header_and_data_bytes[] = {
      0xe0, 0xc0, 0x01, 0xbb, 0xdd, 0xde, 0xbc, 0x90, 0x5b, 0xba, 0x12,
      0xa3, 0x80, 0x10, 0x01, 0xf5, 0x11, 0x8f, 0x00, 0x00, 0x01, 0x01,
      0x08, 0x0a, 0x0a, 0xd7, 0x86, 0x1c, 0xc0, 0x4f, 0x51, 0x01};

  iphdr *ip_header = reinterpret_cast<iphdr *>(ip_header_bytes);
  tcphdr *tcpHeader = reinterpret_cast<tcphdr *>(tcp_header_and_data_bytes);
  auto result = FNet::tcpChecksum(ip_header, tcpHeader);
  EXPECT_EQ(result, ntohs(0xd5d2));
}
