#define _GNU_SOURCE
#include <link.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define TARGET_LIB    "/linux64/steamclient.so"
#define LOG_FILE      "audit_patch.log"

static const uint8_t PATTERN[] = {
    0x41, 0x54, 0x41, 0x89, 0xD4, 0x55, 0x48, 0x89, 0xCD, 0x53, 0x44, 0x8B, 0x47, 0x28
};
#define PATTERN_LEN (sizeof(PATTERN))

// --- Helpers ---

static FILE *logf = NULL;

static void log_open(void) {
    if (!logf)
        logf = fopen(LOG_FILE, "w");
}

static void log_msg(const char *fmt, ...) {
    log_open();
    if (!logf) return;

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", tm);
    fprintf(logf, "[%s] ", ts);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(logf, fmt, ap);
    va_end(ap);

    fflush(logf);
}

int strstrend(const char *str, const char *substr) {
    if (str && substr) {
        const char *start = strstr(str, substr);
        if (start) {
            return strlen(start) == strlen(substr);
        }
    }

    return 0;
}

static uint8_t *find_pattern(uint8_t *start, size_t size) {
    if (size < PATTERN_LEN) return NULL;
    size_t limit = size - PATTERN_LEN;
    for (size_t i = 0; i <= limit; i++) {
        if (memcmp(start + i, PATTERN, PATTERN_LEN) == 0)
            return start + i;
    }
    return NULL;
}

static int get_text_region(uintptr_t base, uint8_t **out_start, size_t *out_size) {
    FILE *maps = fopen("/proc/self/maps", "r");
    if (!maps) return 0;

    char line[256];
    while (fgets(line, sizeof(line), maps)) {
        uintptr_t lo, hi;
        char perms[8];
        if (sscanf(line, "%lx-%lx %7s", &lo, &hi, perms) != 3)
            continue;

        // Must be the right library's region, executable, and start at/after base
        if (lo < base || !(perms[2] == 'x'))
            continue;

        // Confirm this mapping belongs to our target lib
        if (!strstr(line, TARGET_LIB))
            continue;

        *out_start = (uint8_t *)lo;
        *out_size  = hi - lo;
        fclose(maps);
        return 1;
    }

    fclose(maps);
    return 0;
}

// --- Audit entrypoints ---

unsigned int la_version(unsigned int version) {
    log_msg("LD_AUDIT loaded, version %u\n", version);
    return LAV_CURRENT;
}

unsigned int la_objopen(struct link_map *map, Lmid_t lmid, uintptr_t *cookie) {
    if (!strstrend(map->l_name, TARGET_LIB)) {
        return 0;
    }

    uintptr_t base = map->l_addr;
    log_msg("Found %s at base %#lx\n", TARGET_LIB, base);

    // Locate executable region
    uint8_t *region_start = NULL;
    size_t   region_size  = 0;
    if (!get_text_region(base, &region_start, &region_size)) {
        log_msg("ERROR: could not locate executable region in /proc/self/maps\n");
        return 0;
    }
    log_msg("Scanning executable region %p - %p (%zu bytes)\n", (void *)region_start, (void *)(region_start + region_size), region_size);

    // Scan for pattern
    uint8_t *target = find_pattern(region_start, region_size);
    if (!target) {
        log_msg("ERROR: pattern not found\n");
        return 0;
    }
    log_msg("Pattern found at %p (offset %#lx)\n", (void *)target, (uintptr_t)target - base);

    // Patch at target
    uintptr_t page_size = sysconf(_SC_PAGESIZE);
    uintptr_t page_start = (uintptr_t)target & ~(page_size - 1);

    if (mprotect((void *)page_start, page_size * 2, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        log_msg("ERROR: mprotect failed: %m\n");
        return 0;
    }

    uint8_t patch[] = { 0xB8, 0x01, 0x00, 0x00, 0x00, 0xC3 };
    memcpy(target, patch, sizeof(patch));

    mprotect((void *)page_start, page_size * 2, PROT_READ | PROT_EXEC);

    log_msg("Bytes after patch:  %02x %02x %02x %02x %02x %02x\n",
            target[0], target[1], target[2],
            target[3], target[4], target[5]);
    log_msg("Patch applied successfully\n");

    return 0;
}
