
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

void printf(char *fmt, ...)
{

}
