#include <nal.h>

volatile int b = 0;

void sighandler(enum signal_type type);

int main(int argc, char **argv)
{
    sigsethandler(sighandler);
    signal(SIG_IRQ, 1); // listen for the keyboard interrupt

    while (1) {
        b++;

        b %= 0x10000000;
    }

    return 0x12345678;
}

void sighandler(enum signal_type type)
{
    signal(3, b); // this should give an error as there is no signal with type 3
}
