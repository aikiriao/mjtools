#ifndef MJUTILITY_H_INCLUDED
#define MJUTILITY_H_INCLUDED

/* 静的アサート */
#define MJUTILITY_STATIC_ASSERT(cond) void static_assetion_failed(char static_assetion_failed[(cond) ? 1 : -1])

#endif /* MJUTILITY_H_INCLUDED */
