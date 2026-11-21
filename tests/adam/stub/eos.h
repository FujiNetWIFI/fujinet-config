#ifndef STUB_EOS_H
#define STUB_EOS_H

#define EOS_ERR_NONE 0

void eos_init(void);
void eos_scan_for_devices(void);
unsigned char eos_write_block(unsigned char dev, unsigned long blockno, void *buf);

#endif
