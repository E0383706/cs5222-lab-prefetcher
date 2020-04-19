//
// Data Prefetching Championship Simulator 2
// Seth Pugsley, seth.h.pugsley@intel.com
//

/*

  This file does NOT implement any prefetcher, and is just an outline

 */

#include <stdio.h>
#include "../inc/prefetcher.h"

#define IP_COUNTER 256
#define GHB_COUNTER 256
#define PREFETCH_DEGREE 4

typedef struct ip_tracker {
  unsigned long long int ip;
  int ghb_pt;
  unsigned long long int lru_cycle;
} ip_tracker_t;
ip_tracker_t trackers[IP_COUNTER];

typedef struct ghb_tracker {
  unsigned long long int addr;
  int ip_pt;
  int next;
  int previous;
} ghb_tracker_t;
ghb_tracker_t ghb[GHB_COUNTER];

int ghb_header;

void l2_prefetcher_initialize(int cpu_num)
{
  printf("No Prefetching\n");
  // you can inspect these knob values from your code to see which configuration you're runnig in
  printf("Knobs visible from prefetcher: %d %d %d\n", knob_scramble_loads, knob_small_llc, knob_low_bandwidth);

  for (int i=0; i<IP_COUNTER; i++) {
    trackers[i].ip = 0;
    trackers[i].ghb_pt = -1;
    trackers[i].lru_cycle = 0;
  }

  for (int i=0; i<GHB_COUNTER; i++) {
    ghb[i].addr = 0;
    ghb[i].ip_pt = -1;
    ghb[i].next = -1;
    ghb[i].previous = -1;
  }

  ghb_header = 0;
}

void l2_prefetcher_operate(int cpu_num, unsigned long long int addr, unsigned long long int ip, int cache_hit)
{
  // uncomment this line to see all the information available to make prefetch decisions
  //printf("(0x%llx 0x%llx %d %d %d) ", addr, ip, cache_hit, get_l2_read_queue_occupancy(0), get_l2_mshr_occupancy(0));

  if (cache_hit == 1) {
    return;
  }

  // get tracker index to fill or replace
  int tracker_index = -1;
  for (int i=0; i<IP_COUNTER; i++) {
    if (trackers[i].ip = ip) {
      tracker_index = i;
      break;
    }
  }

  if (tracker_index == -1) {
    int lru_index = 0;
    unsigned long long int lru_cycle = trackers[0].lru_cycle;
    for (int i=1; i<IP_COUNTER; i++) {
      if (trackers[i].lru_cycle < lru_cycle) {
        lru_index = i;
        lru_cycle = trackers[lru_index].lru_cycle;
      }
    }
    tracker_index = lru_index;
    trackers[tracker_index].ip = ip
  }
  // end of get tracker index to fill or replace

  // update ip table to ghb, update linked list
  trackers[tracker_index].lru_cycle = get_current_cycle(0);
  ghb[ghb_header].addr = addr;
  ghb[ghb_header].next = -1;
  if (ghb[ghb_header].previous != -1) {
    ghb[ghb[ghb_header].previous].next = -1;
  }
  ghb[ghb_header].previous = -1;
  if (ghb[ghb_header].ip_pt != -1) {
    trackers[ghb[ghb_header].ip_pt].ghb_pt = -1;
  }
  ghb[ghb_header].ip_pt = tracker_index;

  if (trackers[tracker_index].ghb_pt != -1) {
    ghb[trackers[tracker_index].ghb_pt].ip_pt = -1;
    ghb[trackers[tracker_index].ghb_pt].previous = ghb_header;
    ghb[ghb_header].next = trackers[tracker_index].ghb_pt;
  }
  ghb[ghb_header].ip_pt = tracker_index;
  trackers[tracker_index].ghb_pt = ghb_header;
  // end of update ip table to ghb, update linked list

  ghb_header = (ghb_header + 1) % GHB_COUNTER;

  ghb_tracker_t start = ghb[trackers[tracker_index].ghb_pt];
  if (start.next != -1 && ghb[start.next].next != -1) {
    long long int stride1 = start.addr - ghb[start.next].addr;
    ghb_tracker_t current = ghb[ghb[start.next].next];
    long long int stride2 = ghb[start.next].addr - current.addr;

    long long int running_stride1 = stride1;
    long long int running_stride2 = stride2;
    while (current.next != -1) {
      running_stride1 = running_stride2;
      running_stride2 = current.addr - ghb[current.next].addr;
      if (running_stride1 == stride1 && running_stride2 == stride2) {
        current = ghb[current.previous];
        int prefetched = 0;
        while (current.previous != -1 && prefetched < PREFETCH_DEGREE) {
          l2_prefetch_line();
          current = ghb[current.previous];
          prefetched++;
        }
        break;
      }
      current = ghb[current.next];
    }
  }

}

void l2_cache_fill(int cpu_num, unsigned long long int addr, int set, int way, int prefetch, unsigned long long int evicted_addr)
{
  // uncomment this line to see the information available to you when there is a cache fill event
  //printf("0x%llx %d %d %d 0x%llx\n", addr, set, way, prefetch, evicted_addr);
}

void l2_prefetcher_heartbeat_stats(int cpu_num)
{
  printf("Prefetcher heartbeat stats\n");
}

void l2_prefetcher_warmup_stats(int cpu_num)
{
  printf("Prefetcher warmup complete stats\n\n");
}

void l2_prefetcher_final_stats(int cpu_num)
{
  printf("Prefetcher final stats\n");
}
