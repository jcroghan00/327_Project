#include <ncurses.h>

int main(int argc, char *argv[])
{

int test;
for(int i=1;i<250;i++)
{
    test = getch();
    if ( test == 0 || test == 224 )
        test = 256 + getch();
    printf("%d", test);
}


}