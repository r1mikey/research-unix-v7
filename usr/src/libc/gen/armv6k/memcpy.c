

/*
 * Not efficient, not caring...
 */
void *memcpy(void *dest, const void *src, int n)
{
  unsigned char *d = dest;
  const unsigned char *s = src;

  for (; n; n--) *d++ = *s++;
  return dest;
}
