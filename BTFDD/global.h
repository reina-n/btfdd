#define RETURN_NEWLINE "\r\n"

//ƒOƒ[ƒoƒ‹•Ï”
extern volatile unsigned char fdc_mr;
extern volatile unsigned char fdc_command;
extern volatile unsigned char fdc_st0[4];

extern volatile char uart_shell_buffer[80];
extern volatile uint8_t uart_shell_enable;


extern volatile uint8_t timer_count;
extern volatile uint8_t timer_timeout;

extern volatile uint32_t spi_sram_addr;
extern volatile uint16_t spi_sram_crc;

void data_in();
void data_out();

int htoi (const char *s);
unsigned int crcByte(unsigned int crc, unsigned char b);
