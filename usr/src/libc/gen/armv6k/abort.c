#include "../../../../include/signal.h"

extern int kill(int pid, int signum);
extern int getpid(void);

void abort(void)
{
  kill(getpid(), SIGIOT);
}
