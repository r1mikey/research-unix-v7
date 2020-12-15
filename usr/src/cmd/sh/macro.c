#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include "defs.h"
#include "sym.h"

static char quote;  /* used locally */
static char quoted; /* used locally */

static getch();
static comsubst();
static flush();

static char *copyto(endch) char endch;
{
  char c;

  while ((c = getch(endch)) != endch && c) {
    pushstak(c | quote);
  }
  zerostak();
  if (c != endch) {
    error(badsub);
  }
}

static skipto(endch) char endch;
{
  /* skip chars up to } */
  char c;
  while ((c = readc()) && c != endch) {
    switch (c) {

    case SQUOTE:
      skipto(SQUOTE);
      break;

    case DQUOTE:
      skipto(DQUOTE);
      break;

    case DOLLAR:
      if (readc() == BRACE) {
        skipto('}');
      }
    };
  }
  if (c != endch) {
    error(badsub);
  }
}

static getch(endch) char endch;
{
  char d;

retry:
  d = readc();
  if (!subchar(d)) {
    return (d);
  }
  if (d == DOLLAR) {
    int c;
    if ((c = readc(), dolchar(c))) {
      NAMPTR n = NIL;
      int dolg = 0;
      BOOL bra;
      char *argp, *v;
      char idb[2];
      char *id = idb;

      if (bra = (c == BRACE)) {
        c = readc();
      }
      if (letter(c)) {
        argp = relstak();
        while (alphanum(c)) {
          pushstak(c);
          c = readc();
        }
        zerostak();
        n = lookup(absstak(argp));
        setstak(argp);
        v = n->namval;
        id = n->namid;
        peekc = c | MARK;
      } else if (digchar(c)) {
        *id = c;
        idb[1] = 0;
        if (astchar(c)) {
          dolg = 1;
          c = '1';
        }
        c -= '0';
        v = ((c == 0) ? cmdadr : (c <= dolc) ? dolv[c] : (dolg = 0));
      } else if (c == '$') {
        v = pidadr;
      } else if (c == '!') {
        v = pcsadr;
      } else if (c == '#') {
        v = dolladr;
      } else if (c == '?') {
        v = exitadr;
      } else if (c == '-') {
        v = flagadr;
      } else if (bra) {
        error(badsub);
      } else {
        goto retry;
      }
      c = readc();
      if (!defchar(c) && bra) {
        error(badsub);
      }
      argp = 0;
      if (bra) {
        if (c != '}') {
          argp = relstak();
          if ((v == 0) ^ (setchar(c))) {
            copyto('}');
          } else {
            skipto('}');
          }
          argp = absstak(argp);
        }
      } else {
        peekc = c | MARK;
        c = 0;
      }
      if (v) {
        if (c != '+') {
          for (;;) {
            while (c = *v++) {
              pushstak(c | quote);
            }
            if (dolg == 0 || (++dolg > dolc)) {
              break;
            } else {
              v = dolv[dolg];
              pushstak(SP | (*id == '*' ? quote : 0));
            }
          };
        }
      } else if (argp) {
        if (c == '?') {
          failed(id, *argp ? argp : badparam);
        } else if (c == '=') {
          if (n) {
            assign(n, argp);
          } else {
            error(badsub);
          };
        }
      } else if (flags & setflg) {
        failed(id, badparam);
      }
      goto retry;
    } else {
      peekc = c | MARK;
    }
  } else if (d == endch) {
    return (d);
  } else if (d == SQUOTE) {
    comsubst();
    goto retry;
  } else if (d == DQUOTE) {
    quoted++;
    quote ^= QUOTE;
    goto retry;
  }
  return (d);
}

char *macro(as) char *as;
{
  /* Strip "" and do $ substitution
   * Leaves result on top of stack
   */
  BOOL savqu = quoted;
  char savq = quote;
  FILEHDR fb;

  push(&fb);
  estabf(as);
  usestak();
  quote = 0;
  quoted = 0;
  copyto(0);
  pop();
  if (quoted && (stakbot == staktop)) {
    pushstak(QUOTE);
  }
  quote = savq;
  quoted = savqu;
  return (fixstak());
}

static comsubst()
{
  /* command substn */
  FILEBLK cb;
  char d;
  STKPTR savptr = fixstak();

  usestak();
  while ((d = readc()) != SQUOTE && d) {
    pushstak(d);
  }

  {
    char *argc;
    trim(argc = fixstak());
    push(&cb);
    estabf(argc);
  }
  {
    TREPTR t = makefork(FPOU, cmd(EOFSYM, MTFLG | NLFLG));
    int pv[2];

    /* this is done like this so that the pipe
     * is open only when needed
     */
    chkpipe(pv);
    initf(pv[INPIPE]);
    execute(t, 0, 0, pv);
    close(pv[OTPIPE]);
  }
  tdystak(savptr);
  staktop = movstr(savptr, stakbot);
  while (d = readc()) {
    pushstak(d | quote);
  }
  await(0);
  while (stakbot != staktop) {
    if ((*--staktop & STRIP) != NL) {
      ++staktop;
      break;
    };
  }
  pop();
}

#define CPYSIZ 512

subst(in, ot) int in, ot;
{
  char c;
  FILEBLK fb;
  int count = CPYSIZ;

  push(&fb);
  initf(in);
  /* DQUOTE used to stop it from quoting */
  while (c = (getch(DQUOTE) & STRIP)) {
    pushstak(c);
    if (--count == 0) {
      flush(ot);
      count = CPYSIZ;
    };
  }
  flush(ot);
  pop();
}

static flush(ot)
{
  write(ot, stakbot, staktop - stakbot);
  if (flags & execpr) {
    write(output, stakbot, staktop - stakbot);
  }
  staktop = stakbot;
}
