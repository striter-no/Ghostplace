#include "../src/debug.h"

void __print_buffer(
    ubyte *buffer,
    u64 size
){
    u64 line_size = 10;
    u64 lines = ceil((float)size / line_size);

    for (u64 i = 0, rm = 0; i < lines; i++){
        printf("%.10x ", i * line_size);
        
        for (u64 k = 0; k < line_size; k++){
            if (rm < size){
                printf("%.2x ", (int)buffer[rm]);
                rm++;
            } else {
                printf("00");
            }
        }
        
        printf("\n");
    }
}