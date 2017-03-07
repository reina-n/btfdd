#define FDC_LDCR    1
#define FDC_LDOR    2
#define FDC_RD      4
#define FDC_WR      8
#define FDC_CS     16
#define FDC_A0     32
#define FDC_DACK   64
#define FDC_TC    128

void fdc_hardreset();
void fdc_pcatreset();

unsigned char fdc_read_mr();
void fdc_write_do(unsigned char c);
void fdc_write_cr(unsigned char c);


void fdc_exec_03(unsigned char c1, unsigned char c2);
unsigned char fdc_exec_04(unsigned char c);
void fdc_exec_07(unsigned char c);
void fdc_exec_0F(unsigned char c1, unsigned char c2);
void fdc_exec_0A(unsigned char c);
unsigned char fdc_exec_08();

