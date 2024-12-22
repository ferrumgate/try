// Description: This eBPF program filters incoming packets based on the destination IP address and protocol.
#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <bpf/bpf_helpers.h>

struct {
  __uint(type, BPF_MAP_TYPE_HASH);
  __uint(max_entries, 16);
  __type(key, __u32);
  __type(value, __u32);
  __uint(pinning, LIBBPF_PIN_BY_NAME);
} socket_ip_filter_config SEC(".maps");

#define CONFIG_LISTEN_IP 0x00
#define CONFIG_FORWARD_IP 0x01

#define IP_MF 0x2000      // More fragments flag
#define IP_OFFSET 0x1FFF  // Mask for fragmenting bits

static inline int ip_is_fragment(struct __sk_buff *skb, __u32 nhoff) {
  __u16 frag_off;

  bpf_skb_load_bytes(skb, nhoff + offsetof(struct iphdr, frag_off), &frag_off, 2);
  frag_off = __constant_ntohs(frag_off);
  return frag_off & (IP_MF | IP_OFFSET);
}

SEC("socket")
int socket_filter(struct __sk_buff *skb) {
  // Define the IP address to filter
  __u32 listen_ip_key = CONFIG_LISTEN_IP;
  void *listen_ip_value = bpf_map_lookup_elem(
      &socket_ip_filter_config, &listen_ip_key);  // __constant_htonl(0xC0A86969);  // Example: 192.168.105.105
  __u32 listen_ip = listen_ip_value ? (*(__u32 *)listen_ip_value) : 0;

  __u32 forward_ip_key = CONFIG_FORWARD_IP;
  void *forward_ip_value = bpf_map_lookup_elem(&socket_ip_filter_config, &forward_ip_key);
  __u32 forward_ip = forward_ip_value ? (*(__u32 *)forward_ip_value) : 0;

  __u16 proto;
  __u32 nhoff = ETH_HLEN;
  __u32 ip_proto = 0;
  __u32 tcp_hdr_len = 0;
  __u16 tlen;
  __u32 payload_offset = 0;
  __u32 payload_length = 0;
  __u8 hdr_len;
  struct iphdr ip;
  bpf_skb_load_bytes(skb, 12, &proto, 2);
  proto = __constant_htons(proto);
  if (proto != ETH_P_IP) {
    return SK_DROP;  // Pass the packet if it's not an IP packet
  }
  if (ip_is_fragment(skb, nhoff)) {
    return SK_DROP;  // Pass the packet if it's a fragment
  }
  // ip4 header lengths are variable
  // access ihl as a u8 (linux/include/linux/skbuff.h)
  bpf_skb_load_bytes(skb, ETH_HLEN, &hdr_len, sizeof(hdr_len));
  hdr_len &= 0x0f;
  hdr_len *= 4;

  /* verify hlen meets minimum size requirements */
  if (hdr_len < sizeof(struct iphdr)) {
    return SK_DROP;
  }

  bpf_skb_load_bytes(skb, nhoff, &ip, sizeof(struct iphdr));
  if (ip.daddr != listen_ip && ip.daddr != forward_ip) {
    return SK_DROP;  // Pass the packet if the destination IP address is not the target IP address
  }
  if (ip.protocol != IPPROTO_TCP && ip.protocol != IPPROTO_UDP) {
    return SK_DROP;  // Pass the packet if it's not a TCP packet
  }

  return skb->len;  // Pass the packet otherwise
}

char _license[] SEC("license") = "GPL";
