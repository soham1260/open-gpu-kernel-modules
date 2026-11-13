#ifndef _UVM_NETLINK_H
#define _UVM_NETLINK_H

// choosed number 31 (last) is to avoid known protocols conflicts
#define NETLINK_UVM_FAULTS 31

struct uvm_fault_stat_msg 
{
    unsigned int service_id;
    unsigned int batch_id;
    unsigned long long batch_processing_time_us;
    int pid;
    unsigned int raw_faults;
    unsigned int coalesced_faults;
    unsigned int unique_faults;
    unsigned long long time_us;
    char comm[16];
};

int  uvm_netlink_init(void);
void uvm_netlink_exit(void);
void send_uvm_fault_stat(struct uvm_fault_stat_msg *stat);

#endif // _UVM_NETLINK_H
