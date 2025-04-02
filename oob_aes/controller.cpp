#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

int set_pkg_power_limit(uint32_t power_limit1, uint32_t power_limit2, int enable1, int enable2, 
                        int clamp1, int clamp2, uint8_t time_window1, uint8_t time_window2) {
    int fd;
    uint32_t msr = 0x610;  // MSR_PKG_POWER_LIMIT
    uint64_t val = 0;
    uint64_t new_val = 0;
    
    // Open MSR device file
    fd = open("/dev/cpu/1/msr", O_RDWR);
    if (fd == -1) {
        printf("Could not open MSR file, are you running as root?\n");
        return -1;
    }
    
    // Read current value
    if (pread(fd, &val, sizeof(val), msr) == -1) {
        printf("pread failed\n");
        close(fd);
        return -1;
    }
    
    printf("[!DEBUG] Original value read: 0x%lx\n", val);
    
    // Construct new value
    // Power Limit #1 (bits 14:0)
    new_val |= power_limit1 & 0x7FFF;
    
    // Enable Power Limit #1 (bit 15)
    if (enable1)
        new_val |= (1ULL << 15);
    
    // Package Clamping Limitation #1 (bit 16): Allow going below OS-requested P/T state setting during time window specified by bits 23:17
    if (clamp1)
        new_val |= (1ULL << 16);
    
    // Time Window for Power Limit #1 (bits 23:17)
    new_val |= ((uint64_t)(time_window1 & 0x7F) << 17);
    
    // Power Limit #2 (bits 46:32)
    new_val |= ((uint64_t)(power_limit2 & 0x7FFF) << 32);
    
    // Enable Power Limit #2 (bit 47)
    if (enable2)
        new_val |= (1ULL << 47);
    
    // Clamping Limitation #2 (bit 48)
    if (clamp2)
        new_val |= (1ULL << 48);
    
    // Time Window for Power Limit #2 (bits 55:49)
    new_val |= ((uint64_t)(time_window2 & 0x7F) << 49);
    
    // Do not set lock bit (bit 63)
    
    // Write new value
    if (pwrite(fd, &new_val, sizeof(new_val), msr) == -1) {
        printf("pwrite failed\n");
        close(fd);
        return -1;
    }
    
    // Verify the value was written
    uint64_t verify_val = 0;
    if (pread(fd, &verify_val, sizeof(verify_val), msr) == -1) {
        printf("verification pread failed\n");
        close(fd);
        return -1;
    }
    
    printf("[!DEBUG] New value written: 0x%lx\n", verify_val);
    
    if (verify_val != new_val) {
        printf("Warning: Written value differs from requested value\n");
    }
    
    close(fd);
    return 0;
}

int main(int argc, char *argv[]) {
    uint32_t power_limit1 = 1300;  // Default value
    uint32_t power_limit2 = 1300;  // Default value
    int enable1 = 1;
    int enable2 = 1;
    int clamp1 = 0;
    int clamp2 = 0;
    uint8_t time_window1 = 0xF;
    uint8_t time_window2 = 0xF;
    
    // Parse command line arguments if provided
    if (argc >= 3) {
        power_limit1 = atoi(argv[1]);
        power_limit2 = atoi(argv[2]);
    }
    
    if (argc >= 5) {
        enable1 = atoi(argv[3]);
        enable2 = atoi(argv[4]);
    }
    
    if (argc >= 7) {
        clamp1 = atoi(argv[5]);
        clamp2 = atoi(argv[6]);
    }
    
    if (argc >= 9) {
        time_window1 = atoi(argv[7]);
        time_window2 = atoi(argv[8]);
    }
    
    printf("Setting MSR_PKG_POWER_LIMIT (0x610):\n");
    printf("- Power Limit #1: %u, Enabled: %d, Clamping: %d, Time Window: 0x%x\n", 
           power_limit1, enable1, clamp1, time_window1);
    printf("- Power Limit #2: %u, Enabled: %d, Clamping: %d, Time Window: 0x%x\n", 
           power_limit2, enable2, clamp2, time_window2);
    
    return set_pkg_power_limit(power_limit1, power_limit2, enable1, enable2, 
                               clamp1, clamp2, time_window1, time_window2);
}