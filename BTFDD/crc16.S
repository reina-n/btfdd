; https://embdev.net/articles/CRC-16-CCITT_in_AVR_Assembler
; Cの関数と使えるようにレジスタ修正
; 元の関数より2倍くらい速くなっている気がする

; r23 = new data byte to add to the CRC-16 checksum.
; The 16-bit variable crc16_accumulator (see below) holds the checksum,
; you should initialise its value to 0xFFFF before the first call to this routine.
; r18, r19, r24 and r25 are overwritten.


/* Algorithm: CRC-16-CCITT, x16 + x12 + x5 + 1, 0x1021 / 0x8408 / 0x8810

   Steps in C to update a running CRC with a new byte in ser_data:

   unsigned char ser_data;
   unsigned int crc;   -> accumulated CRC to update, initial value 0xFFFF

   crc  = (unsigned char)(crc >> 8) | (crc << 8);
   crc ^= ser_data;
   crc ^= (unsigned char)(crc & 0xff) >> 4;
   crc ^= (crc << 8) << 4;

   crc ^= ((crc & 0xff) << 4) << 1;

   Note that there are faster table-based implementations, but they
   consume more program space.
*/

.global accumulate_crc16
.func accumulate_crc16

accumulate_crc16:

    ; input
    ; r24: CRC low
    ; r25: CRC high
    ; r22: data

    ; work
    ; r18 r19

    ; output
    ; r24: CRC low
    ; r25: CRC high



    ; Load the last CRC value, but swapped (low <-> high)
    ;lds r18, crc16_accumulator+1
    ;lds r19, crc16_accumulator
    mov r18, r24
    mov r24, r25
    mov r25, r18


    ; First XOR (代入)
    eor r24, r22


    ; Second XOR
    mov r19, r24


    ; These 2 instructions are faster than executing 4 times "lsr 4".
    swap r19
    andi r19, 0x0F
    eor  r24, r19


    ; Third XOR
    mov r19, r24


    ; These 2 instructions are faster than executing 4 times "lsr 4".
    swap r19
    andi r19, 0xF0
    eor  r25, r19


    ; Fourth XOR
    mov  r19, r24
    swap r19
    mov  r18, r19
    andi r19, 0xF0
    andi r18, 0x0F
    lsl r19
    rol r18

    eor r24, r19
    eor r25, r18

    ;sts crc16_accumulator  , r24 //low
    ;sts crc16_accumulator+1, r25 //high

    ret

.endfunc
