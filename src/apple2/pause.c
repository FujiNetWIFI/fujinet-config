#ifdef BUILD_APPLE2

#pragma warn (unused-param, push, off)
void pause(unsigned char delay)
{
    int a,b;

    for (a=0;a<delay;a++)
        for (b=0;b<40;b++);

}

#pragma warn (unused-param, pop)
#endif