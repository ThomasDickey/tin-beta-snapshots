char *sha_key(const unsigned char *secret, size_t seclen,
              const unsigned char *message, size_t msglen);
char *sha_lock(const unsigned char *secret, size_t seclen,
               const unsigned char *message, size_t msglen);
int sha_verify(const char *key, const char *lock);

char *lock_strip_alpha(const char *key, char *type);
char *lock_strip(const char *key, char *type);
