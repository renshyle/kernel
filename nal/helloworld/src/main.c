#include <nal.h>

int main(int argc, char **argv)
{
    syscall(1, 0xdeadc0de, 0x1234567890abcdef, 0xdeaddeaddeaddaed, 0xfacefeed12983476, 0xfeedc0de);
    return 0x12345678;
}
