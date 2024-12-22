#include "fnet.h"

namespace Ferrum {

FNet::FNet() = default;
FNet::~FNet() = default;

/* Compute checksum for count bytes starting at addr, using one's complement of
 * one's complement sum*/
uint16_t FNet::checksum(uint16_t *addr, uint32_t count) {
  uint32_t sum = 0;
  while (count > 1) {
    sum += *addr++;
    count -= 2;
  }
  // if any bytes left, pad the bytes and add
  if (count > 0) {
    sum += ((*addr) & htons(0xFF00));
  }
  // Fold sum to 16 bits: add carrier to result
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }
  // one's complement
  sum = ~sum;
  return static_cast<uint16_t>(sum);
}

uint16_t FNet::ipChecksum(struct iphdr *iphdrp) {
  iphdrp->check = 0;
  return checksum(reinterpret_cast<uint16_t *>(iphdrp), iphdrp->ihl << 2);
}

/* set tcp checksum: given IP header and tcp segment */
uint16_t FNet::tcpChecksum(struct iphdr *pIph, struct tcphdr *tcphdrp) {
  uint32_t sum = 0;
  uint16_t tcpLen = ntohs(pIph->tot_len) - (pIph->ihl << 2);
  uint16_t *payload = reinterpret_cast<uint16_t *>(tcphdrp);
  tcphdrp->check = 0;
  // add the pseudo header
  // the source ip
  sum += (pIph->saddr >> 16) & 0xFFFF;
  sum += (pIph->saddr) & 0xFFFF;
  // the dest ip
  sum += (pIph->daddr >> 16) & 0xFFFF;
  sum += (pIph->daddr) & 0xFFFF;
  // protocol and reserved: 6
  sum += htons(IPPROTO_TCP);
  // the length
  sum += htons(tcpLen);

  // add the IP payload
  while (tcpLen > 1) {
    sum += *payload++;
    tcpLen -= 2;
  }
  // if any bytes left, pad the bytes and add
  if (tcpLen > 0) {
    // printf("+++++++++++padding, %dn", tcpLen);
    sum += ((*payload) & htons(0xFF00));
  }
  // Fold 32-bit sum to 16 bits: add carrier to result
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }
  // set computation result
  return static_cast<uint16_t>(~sum);
}

/* set udp checksum: given IP header and UDP datagram */
uint16_t FNet::udpChecksum(struct iphdr *pIph, struct udphdr *udphdrp) {
  uint32_t sum = 0;
  uint16_t *payload = reinterpret_cast<uint16_t *>(udphdrp);
  uint16_t udpLen = htons(udphdrp->len);
  udphdrp->check = 0;
  // add the pseudo header
  // the source ip
  sum += (pIph->saddr >> 16) & 0xFFFF;
  sum += (pIph->saddr) & 0xFFFF;
  // the dest ip
  sum += (pIph->daddr >> 16) & 0xFFFF;
  sum += (pIph->daddr) & 0xFFFF;
  // protocol and reserved: 17
  sum += htons(IPPROTO_UDP);
  // the length
  sum += udphdrp->len;

  // add the IP payload
  // initialize checksum to 0
  while (udpLen > 1) {
    sum += *payload++;
    udpLen -= 2;
  }
  // if any bytes left, pad the bytes and add
  if (udpLen > 0) {
    // printf("+++++++++++++++padding: %dn", udpLen);
    sum += ((*payload) & htons(0xFF00));
  }
  // Fold sum to 16 bits: add carrier to result
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }
  // printf("one's complementn");
  return static_cast<uint16_t>(~sum);
}

}  // namespace Ferrum