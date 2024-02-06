#include <stdio.h>

int main()
{
    FILE *f = fopen("inpFile.mbin", "wb");
    unsigned char program[] = {
        // This program will print "Hello, World!\n"
        // As one address means 8 bytes i.e each of these above byte are going to be in their respective memory locations
        // That is really inefficient but we can mitigate this by compressing 8 characters into 1 address but this will require
        // a lot of logical shifts to get the needed data which is not fast and efficient
        // Thus to solve this issue, a new solution will be needed
        0x4d, 0x49, 0x4e, 0x00, 0x00, 0x00, 0x00, 0x01, /*We are going big endian mode here for easiness*/
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, /*The instruction's size*/
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, /*The data size: We have 13 but len must be a multiple of 8*/
        // 0x1A, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, /*move_imm Mc, 14*/
        0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*move_imm Ma, 0 ;; The address to the message*/
        0x1A, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, /*move_imm Mb, 14 ;; The length of the message including '\0'*/
        0x5D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA2, /*int 162<WRITE STR>*/
        // 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*dec Ma*/
        // 0x4E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, /*jnz [2]*/
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*Halt*/
        'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l',
        'd', '!', '\n', 0, 0, 0};
    fwrite(program, 1, sizeof(program), f);
    fclose(f);
}