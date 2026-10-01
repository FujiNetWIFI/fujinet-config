/* Handoff between the apps; sits just above their --limit in the Makefile. */
struct entry_data
{
  unsigned char magic[2];
  unsigned char entry;
  unsigned char host_slot;
  unsigned char device_slot;
};

#define ENTRY_DATA ((struct entry_data *)0x7BF8)

#define ENTRY_NONE  0
#define ENTRY_INFO  1
#define ENTRY_FILES 2
#define ENTRY_HOSTS 3

void entry_set(unsigned char entry, unsigned char host_slot, unsigned char device_slot);

/* Returns the pending entry code (ENTRY_NONE if none) and clears it. */
unsigned char entry_take(void);
