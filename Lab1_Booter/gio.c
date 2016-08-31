#define MAXLEN 128

char *ctable = "0123456789ABCDEF";
u16 BASE;

void prints(u8 *s)
{
    while (*s)
    {
        putc(*s++);
    }
}

void gets(char s[])
{
	*s = getc(); // grab first char
    while (*s != '\r')
    {
    	putc(*s++); // add to the char array
    	*s = getc(); // grab next char
    }
    *s = 0; // reset back to null
}

// From KCW
//char *gets(char s[])
//{
//    char c, *t = s; int len=0;
//    while ( (c=getc() != '\r' && len < MAXLEN-1))
//    {
//        *t++ = c; putc(c); len++;
//    }
//    *t = 0; return s;
//}

// From KCW
void rpu (u16 x)
{
    char c;
    if (x)
    {
        c = ctable[x % BASE];
        rpu(x / BASE);
        putc(c);
    }
}

// From KCW
//void printu(u16 x)
//{
//    //(x==0) ? putc('0') : rpu(x);
//    putc(' ');
//}

void printu(u32 x)
{
  if (x == 0)
    putc('0');
  else
  {
    BASE = 10;
    rpu(x);
  }
  putc(' ');
}

void printd(int x)
{
  if (x == 0)
    putc('0');
  else
  {
    BASE = 10;
    if (x < 0)
    {
      x *= -1;
      putc('-');
    }
    rpu(x);
  }
  putc(' ');
}

void printx(u32 x)
{
  if (x == 0)
    putc('0');
  else
  {
    BASE = 16;
    putc('0');
    putc('x');
    rpu(x);
  }
  putc(' ');
}

// From KCW
void printf(char *fmt, ...)
{
    char *cp = fmt;
    u16 *ip = (u16 *)&fmt + 1;  // first argument
    u32 *up;                    // for accessing long parameters on stack
    while (*cp)
    {
        if (*cp != '%')
        {
            putc (*cp);
            if (*cp == '\n')
            {
                putc('\r');
            }
            cp++; continue;
        }
        cp++;
        switch(*cp)
        {
            case 'c': putc(*ip); break;
            case 's': prints(*ip); break;
            case 'u': printu(*ip); break;
            case 'd': printd(*ip); break;
            case 'x': printx(*ip); break;
            //case 'l': printl(*(u32 *)ip++); break;
            //case 'X': printX(*(u32 *)ip++); break;
        }
        cp++; ip++;
    }
}

