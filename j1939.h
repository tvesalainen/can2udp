
#ifndef _J1939_H
#define _J1939_H

#define SA(id) ((id) & 0xff)
#define PS(id) (((id)>>8) & 0xff)
#define PF(id) (((id)>>16) & 0xff)
#define DP(id) (((id)>>24) & 0x1)
#define EDP(id) (((id)>>25) & 0x1)
#define PRIORITY(id) (((id)>>26) & 0x3)
#define PGN(id) (PF(id)<0xf0?(DP(id)<<16)<<16|(PF(id)<<8):(DP(id)<<16)|PF(id)<<8)|PS(id)

#endif /* _J1939_H */
