struct REG {
		unsigned bit0:1,
				 bit1:1,
				 bit2:1,
				 bit3:1,
				 bit4:1,
				 bit5:1,
				 bit6:1,
				 bit7:1 ;};
#define PORA (*(volatile struct REG*) 0x3B)
#define PINNA (*(volatile struct REG*) 0x39)
#define DDDRA (*(volatile struct REG*) 0x3B)

#define PORD (*(volatile struct REG*) 0x32)
#define PINND (*(volatile struct REG*) 0x30)
#define DDDRD (*(volatile struct REG*) 0x31)

#define PORC (*(volatile struct REG*) 0x35)