// note: is not supposed to not match binary layout!
typedef struct {
  uint64_t parm_addr;
  uint64_t initrd_addr;
  uint64_t initrd_len;
  uint64_t psw;
  uint64_t extra;	// 1 = scsi, 0 = !scsi
  unsigned flags;	// bit 0: scsi, bit 1: kdump
  unsigned ok:1;	// 1 = zipl stage 3 header read
} zipl_stage3_head_t;


void dump_zipl_components(disk_t *disk, uint64_t sec);
void dump_zipl(disk_t *disk);
