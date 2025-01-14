//
// CMS Build Fixes
//

#ifndef CREXX_CMS_H
#define CREXX_CMS_H

#undef RX_INLINE
#define RX_INLINE inline

/* VM/370 has a 32 bit (or 24/32) architecture */
#define __32BIT__

/*
 * GCC in VM/370 can't seem to handle all the computed gotos - so use a
 * classic bytecode architecture
 */
#define NTHREADED

/*
 * VM/370 does not support UTF
 */
#define NUTF8

/* Shocking hack ... */
#define snprintf(s,sz,...) sprintf(s,__VA_ARGS__)
#define vsnprintf(s,sz,...) vsprintf(s,__VA_ARGS__)

#endif //CREXX_CMS_H
