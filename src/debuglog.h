#ifndef DEBUGLOG_H
#define DEBUGLOG_H

#include <stdint.h>
#include <stddef.h>
#include <psp2kern/kernel/iofilemgr.h>

// Simple diagnostic logger: appends lines to ur0:data/vitacontrol_log.txt
// Limited to a fixed number of lines so it can't fill storage or slow input forever.

#define DEBUGLOG_PATH      "ur0:data/vitacontrol_log.txt"
#define DEBUGLOG_MAX_LINES 400

namespace DebugLog
{
    static int lineCount = 0;

    static inline void appendStr(char *dst, size_t &pos, size_t cap, const char *s)
    {
        while (*s && pos + 1 < cap) dst[pos++] = *s++;
        dst[pos] = 0;
    }

    static inline void appendHex(char *dst, size_t &pos, size_t cap, uint32_t val, int digits)
    {
        static const char hex[] = "0123456789ABCDEF";
        for (int i = digits - 1; i >= 0 && pos + 1 < cap; i--)
            dst[pos++] = hex[(val >> (i * 4)) & 0xF];
        dst[pos] = 0;
    }

    static inline void writeLine(const char *line, size_t len)
    {
        if (lineCount >= DEBUGLOG_MAX_LINES) return;
        lineCount++;

        SceUID fd = ksceIoOpen(DEBUGLOG_PATH, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0666);
        if (fd < 0) return;
        ksceIoWrite(fd, line, len);
        ksceIoWrite(fd, "\n", 1);
        ksceIoClose(fd);
    }

    // Log a connection with its VID/PID and chosen driver
    static inline void logConnect(uint16_t vid, uint16_t pid, const char *driver)
    {
        char line[96];
        size_t pos = 0;
        appendStr(line, pos, sizeof(line), "CONNECT vid=");
        appendHex(line, pos, sizeof(line), vid, 4);
        appendStr(line, pos, sizeof(line), " pid=");
        appendHex(line, pos, sizeof(line), pid, 4);
        appendStr(line, pos, sizeof(line), " driver=");
        appendStr(line, pos, sizeof(line), driver);
        writeLine(line, pos);
    }

    // Log raw report bytes
    static inline void logReport(const uint8_t *buf, size_t count)
    {
        char line[200];
        size_t pos = 0;
        appendStr(line, pos, sizeof(line), "RPT");
        for (size_t i = 0; i < count; i++)
        {
            appendStr(line, pos, sizeof(line), " ");
            appendHex(line, pos, sizeof(line), buf[i], 2);
        }
        writeLine(line, pos);
    }
}

#endif // DEBUGLOG_H
