#ifndef PDC_HASH_UTIL_H
#define PDC_HASH_UTIL_H

unsigned long djb2_hash_ul(unsigned char *str);

unsigned long sdbm_hash_ul(unsigned char *str);

unsigned long lose_lose_hash_ul(unsigned char *str);

#endif /* !PDC_HASH_UTIL_H */