

/**
 * helper macro
 */
#define ZBB_MIRROR_HANDLE(handle) \
    struct handle##__ { \
        int unused; \
    }; \
    typedef struct handle##__* handle
