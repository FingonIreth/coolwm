#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG
#ifdef DEBUG
    #define DLOG(fmt, args...) fprintf(stderr, "%s %d: " fmt "\n", __FILE__, __LINE__, ##args)
#endif

#endif /* DEBUG_H */
