#include "uvm_netlink.h"
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

static unsigned uvm_netlink_enable = 0;
module_param(uvm_netlink_enable, uint, S_IRUGO);
MODULE_PARM_DESC(uvm_netlink_enable, "Enable UVM fault statistics netlink broadcast (default: 0)");

static struct sock *nl_sk = NULL;

int uvm_netlink_init(void)
{
    struct netlink_kernel_cfg cfg = 
    {
        .input = NULL,
    };

    if (!uvm_netlink_enable)
        return 0;

    nl_sk = netlink_kernel_create(&init_net, NETLINK_UVM_FAULTS, &cfg);
    if (!nl_sk) 
    {
        printk(KERN_ALERT "UVM: Error creating Netlink socket.\n");
        return -ENOMEM;
    }
    return 0;
}

void uvm_netlink_exit(void)
{
    if (nl_sk) 
    {
        netlink_kernel_release(nl_sk);
        nl_sk = NULL;
    }
}

void send_uvm_fault_stat(struct uvm_fault_stat_msg *stat)
{
    struct sk_buff *skb_out;
    struct nlmsghdr *nlh; // Netlink Message Header
    int msg_size = sizeof(struct uvm_fault_stat_msg);

    if (!nl_sk)
        return;

    skb_out = nlmsg_new(msg_size, 0); // this contains size of buffer + headers
    if (!skb_out)
        return;

    // nlmsg_put creates the Netlink Header at the very beginning of the buffer
    // It sets the PID to 0 (meaning "from kernel") and type to NLMSG_DONE
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0); // It basically initializes those headers inside buffer
    if (!nlh) 
    {
        kfree_skb(skb_out);
        return;
    }

    memcpy(nlmsg_data(nlh), stat, msg_size); // fill data after the header, nlmsg_data returns pointer to data after header

    // multicast to group 1, ignore error if no listeners
    nlmsg_multicast(nl_sk, skb_out, 0, 1, GFP_ATOMIC);
}
