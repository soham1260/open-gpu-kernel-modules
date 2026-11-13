#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>

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

int main(void) 
{
    int sock_fd;
    struct sockaddr_nl src_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    struct msghdr msg;

    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_UVM_FAULTS);
    if (sock_fd < 0) 
    {
        perror("Socket creation failed");
        return -1;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();
    src_addr.nl_groups = 1; // join group 1 of NETLINK_UVM_FAULTS, each channel has 32 multicast groups

    if (bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0) 
    {
        perror("Bind failed");
        close(sock_fd);
        return -1;
    }

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(struct uvm_fault_stat_msg)));
    if (!nlh) 
    {
        perror("Malloc failed");
        close(sock_fd);
        return -1;
    }

    memset(nlh, 0, NLMSG_SPACE(sizeof(struct uvm_fault_stat_msg)));
    
    iov.iov_base = (void *)nlh; // our data comes here (same data what was in skb_out(nsl header + payload))
    iov.iov_len = NLMSG_SPACE(sizeof(struct uvm_fault_stat_msg));
    
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    printf("\033[2J\033[H"); // Clear screen
    printf("UVM-TOP - Live GPU Page Fault Metrics\n");
    printf("--------------------------------------------------------------------------------------------------------------------\n");
    printf("%-8s %-16s %-10s %-10s %-12s %-12s %-12s %-12s %-14s\n", "PID", "COMM", "SVC_ID", "BATCH_ID", "RAW", "COALESCED", "UNIQUE", "TIME(us)", "PROC_TIME(us)");
    printf("--------------------------------------------------------------------------------------------------------------------\n");

    while (1) 
    {
        ssize_t ret = recvmsg(sock_fd, &msg, 0);
        if (ret < 0) 
        {
            printf("recvmsg failed");
            continue;
        }

        struct uvm_fault_stat_msg *stat = (struct uvm_fault_stat_msg *)NLMSG_DATA(nlh);

        printf("%-8d %-16s %-10u %-10u %-12u %-12u %-12u %-12llu %-14llu\n", stat->pid, stat->comm, stat->service_id, stat->batch_id, stat->raw_faults, stat->coalesced_faults, stat->unique_faults, stat->time_us, stat->batch_processing_time_us);
    }

    close(sock_fd);
    free(nlh);
    return 0;
}