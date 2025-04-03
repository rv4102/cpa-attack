#ifndef MSR_HANDLER_H
#define MSR_HANDLER_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define IA32_PERFEVTSEL0 0x186
#define IA32_PMC0 0xC1

class MSRHandler {
private:
    int fd;

public:
    MSRHandler(int cpu = 0) {
        char msr_path[64];
        snprintf(msr_path, sizeof(msr_path), "/dev/cpu/%d/msr", cpu);
        
        fd = open(msr_path, O_RDWR);
        if (fd < 0) {
            perror("Error opening MSR device");
            exit(1);
        }
    }

    ~MSRHandler() {
        if (fd >= 0) close(fd);
    }

    void disable_PMC0() {
        // Disable PMC0 by writing 0 to IA32_PERFEVTSEL0
        uint64_t value = 0;
        if (pwrite(fd, &value, sizeof(value), IA32_PERFEVTSEL0) != sizeof(value)) {
            perror("Error disabling PMC0");
            exit(1);
        }
    }

    void set_PMC0_lsb() {
        // Read current PMC0 value
        uint64_t value = 0;
        if (pread(fd, &value, sizeof(value), IA32_PMC0) != sizeof(value)) {
            perror("Error reading PMC0");
            exit(1);
        }
        
        // Set LSB
        value |= 1;
        
        // Write back
        if (pwrite(fd, &value, sizeof(value), IA32_PMC0) != sizeof(value)) {
            perror("Error setting PMC0 LSB");
            exit(1);
        }
    }

    void clear_PMC0_lsb() {
        // Read current PMC0 value
        uint64_t value = 0;
        if (pread(fd, &value, sizeof(value), IA32_PMC0) != sizeof(value)) {
            perror("Error reading PMC0");
            exit(1);
        }
        
        // Clear LSB
        value &= ~1ULL;
        
        // Write back
        if (pwrite(fd, &value, sizeof(value), IA32_PMC0) != sizeof(value)) {
            perror("Error clearing PMC0 LSB");
            exit(1);
        }
    }
};

#endif