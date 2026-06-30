make modules -j$(nproc)

sudo rmmod nvidia_uvm

sudo insmod kernel-open/nvidia-uvm.ko uvm_perf_prefetch_enable=0

sudo dmesg -w
