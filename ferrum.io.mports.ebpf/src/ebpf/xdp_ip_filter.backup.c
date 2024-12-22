// disable vscode format-on-save for this file
// vscode settings: {
//   "editor.formatOnSave": false
// }

#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <bpf/bpf_helpers.h>

/* struct bpf_map_def SEC("maps") drop_map = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(__u32),
    .value_size = sizeof(__u32),
    .max_entries = 256,
}; */

SEC("xdp")
int xdp_ip_filter(struct xdp_md *ctx) {
  // Define the IP address to filter
  __u32 target_ip = __constant_htonl(0xC0A86969);  // Example: 192.168.105.105
  void *data = (void *)(long)ctx->data;
  void *data_end = (void *)(long)ctx->data_end;

  // Read the Ethernet header
  struct ethhdr *eth = (struct ethhdr *)data;

  // Check if the packet is large enough for an Ethernet header
  if ((void *)(eth + 1) > data_end) {
    return XDP_ABORTED;
  }

  // Check for IPv4
  /* if (eth->h_proto == __constant_htons(ETH_P_IP)) {
    // Move pointer past Ethernet header to IP header
    struct iphdr *ip = (struct iphdr *)(eth + 1);

    // Ensure we have a valid IP header
    if ((void *)(ip + 1) > data_end) {
      return XDP_ABORTED;
    }

    // Check if the source or destination IP matches the target IP
    if (ip->saddr != target_ip && ip->daddr != target_ip) {
      return XDP_DROP;  // Drop packet if it doesn't match
    }
  } */

  return XDP_PASS;  // Pass the packet otherwise
}

char _license[] SEC("license") = "GPL";
