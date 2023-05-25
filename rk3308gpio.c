#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>

#define INPUT 1
#define OUTPUT 2


void *CRU_base = NULL;
void *GRU_base = NULL;
void *GPIO0_base = NULL;
void *GPIO1_base = NULL;
void *GPIO2_base = NULL;
void *GPIO3_base = NULL;
void *GPIO4_base = NULL;

void *CRU_base_PC = NULL;
void *GRU_base_PC = NULL;
void *GPIO0_base_PC = NULL;
void *GPIO1_base_PC = NULL;
void *GPIO2_base_PC = NULL;
void *GPIO3_base_PC = NULL;
void *GPIO4_base_PC = NULL;

static inline unsigned int raw_read(const volatile void *addr) {
	return *(const volatile unsigned int *) addr;
}

static inline void raw_write(unsigned int data, volatile void *addr) {
	*(volatile unsigned int *) addr = data;
}

#define GPIO_SWPORT_DR 0x00
#define GPIO_SWPORT_DDR 0x04
#define GPIO_EXT_PORTA 0x50

int GPIO_Init(void);
void GPIO_Free(void);

void *getControlReg(int base, int pin) {
	switch (base) {
		case 0:
			return GPIO0_base;
		case 1:
			return GPIO1_base;
		case 2:
			return GPIO2_base;
		case 3:
			return GPIO3_base;
		case 4:
			return GPIO4_base;
		default:
			return 0;
	}
}

unsigned int getCRUEnableOffset(int base, int pin)
{
	switch (base) {
		case 0:
			return 0x318;
		case 1:
			return 0x318;
		case 2:
			return 0x318;
		case 3:
			return 0x318;
		default:
			return 0;
	}
}

unsigned int getCRUEnableMusk(int base, int pin)
{
	switch (base) {
		case 0:
			return 0x10000000;
		case 1:
			return 0x20000000;
		case 2:
			return 0x40000000;
		case 3:
			return 0x80000000;
		default:
			return 0;
	}
}


unsigned int getGRUMuxOffset(int base, int pin) {
	switch (base) {
		case 0: {
			switch(pin / 8) {
				case 0:
					return 0x0;
				case 1:
					return 0x8;
				case 2:
					return 0x10;
			}
			break;
		}
		case 1:{
           
			switch(pin / 8) {
				case 0:
					return 0x20;
				case 1:  // special for 1B,1C, each take 2 word to control
                {
                    if((pin%8)<=6)
                    {
					    return 0x28;
                    }
                    else{
                        return 0x2c;
                    }
                }
				case 2: // special for 1B,1C, each take 2 word to control
                {
                    if((pin%8)<=4)
                    {
                        return 0x30;
                    }
                    else
                    {
                        return 0x34;
                    }
                }
				case 3:
					return 0x38;
			}
			break;
		}
		case 2:{
			switch(pin / 8) {
				case 0:
					return 0x40;
				case 1:
					return 0x48;
				case 2:
					return 0x50;
			}
			break;
		}
		case 3:{
			switch(pin / 8) {
				case 0:
					return 0x60;
				case 1:
					return 0x68;
			}
			break;
		}
	}

	// Unmapped for now.
	return 0;
}

unsigned int getGRUMuxMask(int base, int pin) {
	switch (base) {
		case 0: {
			switch (pin % 8) {
				case 0:
					return 0x00030000;
				case 1:
					return 0x000C0000;
				case 2:
					return 0x00300000;
				case 3:
					return 0x00C00000;
				case 4:
					return 0x03000000;
				case 5:
					return 0x0C000000;
				case 6:
					return 0x30000000;
				case 7:
					return 0xC0000000;
			}
			break;
		}
		case 1: {
            switch(pin/8)
            {
                case 0:
                {
                    switch (pin % 8) {
                        case 0:
                            return 0x00030000;
                        case 1:
                            return 0x000C0000;
                        case 2:
                            return 0x00300000;
                        case 3:
                            return 0x00C00000;
                        case 4:
                            return 0x03000000;
                        case 5:
                            return 0x0C000000;
                        case 6:
                            return 0x30000000;
                        case 7:
                            return 0xC0000000;
                    }
                }   
                case 1:
                {
                    switch (pin % 8) {
                        case 0:
                            return 0x00030000;
                        case 1:
                            return 0x000C0000;
                        case 2:
                            return 0x00300000;
                        case 3:
                            return 0x00C00000;
                        case 4:
                            return 0x03000000;
                        case 5:
                            return 0x0C000000;
                        case 6:
                            return 0xF0000000;
                        case 7:
                            return 0x00030000;
                    }
                }
                case 2:
                {
                    switch (pin % 8) {
                        case 0:
                            return 0x00030000;
                        case 1:
                            return 0x000C0000;
                        case 2:
                            return 0x00F00000;
                        case 3:
                            return 0x0F000000;
                        case 4:
                            return 0xF0000000;
                        case 5:
                            return 0x000F0000;
                        case 6:
                            return 0x00F00000;
                        case 7:
                            return 0x0F000000;
                    }
                }
                case 3:
                {
                    switch (pin % 8) {
                        case 0:
                            return 0x00030000;
                        case 1:
                            return 0x000C0000;
                    }
                }
            }
			break;
		}
		case 2: {
			switch (pin % 8) {
				case 0:
					return 0x00030000;
				case 1:
					return 0x000C0000;
				case 2:
					return 0x00300000;
				case 3:
					return 0x00C00000;
				case 4:
					return 0x03000000;
				case 5:
					return 0x0C000000;
				case 6:
					return 0x30000000;
				case 7:
					return 0xC0000000;
			}
			break;
		}
	}

	// Unmapped for now.
	return 0;
}

unsigned int getGRUEnableOffset(int base, int pin) {
	switch (base) {
		case 0: {
			switch(pin / 8) {
				case 0:
					return 0xa0;
				case 1:
					return 0xa4;
				case 2:
					return 0xa8;
			}
			break;
		}
		case 1:{
			switch(pin / 8) {
				case 0:
					return 0xb0;
				case 1:
					return 0xb4;
				case 2:
					return 0xb8;
				case 3:
					return 0xbc;
			}
			break;
		}
		case 2:{
			switch(pin / 8) {
				case 0:
					return 0xc0;
				case 1:
					return 0xc4;
				case 2:
					return 0xc8;
			}
			break;
		}
		case 3:{
			switch(pin / 8) {
				case 0:
					return 0xd0;
				case 1:
					return 0xd4;
			}
			break;
		}
	}

	// Unmapped for now.
	return 0;
}

unsigned int getGRUEnableMask(int base, int pin) {
	switch (base) {
		case 0: {
			switch (pin % 8) {
				case 0:
					return 0x00030000;
				case 1:
					return 0x000C0000;
				case 2:
					return 0x00300000;
				case 3:
					return 0x00C00000;
				case 4:
					return 0x03000000;
				case 5:
					return 0x0C000000;
				case 6:
					return 0x30000000;
				case 7:
					return 0xC0000000;
			}
			break;
		}
		case 1: {
			switch (pin % 8) {
				case 0:
					return 0x00030000;
				case 1:
					return 0x000C0000;
				case 2:
					return 0x00300000;
				case 3:
					return 0x00C00000;
				case 4:
					return 0x03000000;
				case 5:
					return 0x0C000000;
				case 6:
					return 0x30000000;
				case 7:
					return 0xC0000000;
			}
			break;
		}
		case 2: {
			switch (pin % 8) {
				case 0:
					return 0x00030000;
				case 1:
					return 0x000C0000;
				case 2:
					return 0x00300000;
				case 3:
					return 0x00C00000;
				case 4:
					return 0x03000000;
				case 5:
					return 0x0C000000;
				case 6:
					return 0x30000000;
				case 7:
					return 0xC0000000;
			}
			break;
		}
	}

	// Unmapped for now.
	return 0;
}

int mapReg(void *reg, void **reg_mapped, void **reg_mapped_PC) {
	int fd;
	unsigned int addr_start, addr_offset;
	unsigned int pagesize, pagemask;
	

	if((fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
		return -1;
	}

	pagesize = sysconf(_SC_PAGESIZE);
	pagemask = (~(pagesize - 1));

	addr_start = (uintptr_t)reg & pagemask;
	addr_offset = (uintptr_t)reg & ~pagemask;
	// printf("addr_start:%d,addr_offset:%d\n",addr_start,addr_offset);
	// printf("reg:%ld\n",(uintptr_t)reg);
	*reg_mapped_PC = (void *)mmap(0, pagesize * 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr_start);

	if(*reg_mapped_PC == MAP_FAILED) {
		return -2;
	}

	close(fd);

	*(uintptr_t *)reg_mapped = (*(uintptr_t *)reg_mapped_PC + addr_offset);

	return 0;
}

int GPIO_Init() {
	// Map GPIO0-4 for bit twiddling.
	if (mapReg((void *)0xff220000, &GPIO0_base, &GPIO0_base_PC) != 0) {
		return -1;
	}
	if (mapReg((void *)0xff230000, &GPIO1_base, &GPIO1_base_PC) != 0) {
		return -1;
	}
	if (mapReg((void *)0xff240000, &GPIO2_base, &GPIO2_base_PC) != 0) {
		return -1;
	}
	if (mapReg((void *)0xff250000, &GPIO3_base, &GPIO3_base_PC) != 0) {
		return -1;
	}
	if (mapReg((void *)0xff260000, &GPIO4_base, &GPIO4_base_PC) != 0) {
		return -1;
	}

	// Map GRU for MUX settings.
	if (mapReg((void *)0xff000000, &GRU_base, &GRU_base_PC) != 0) {
		return -2;
	}

	// Map CRU for gpio pclk settings.
	if (mapReg((void *)0xff500000, &CRU_base, &CRU_base_PC) != 0) {
		return -2;
	}

	printf("GPIO_INIT success\n");
	return 0;
}

void GPIO_Free()
{
    unsigned int pagesize;
    pagesize = sysconf(_SC_PAGESIZE);
    munmap(GPIO0_base_PC,pagesize*2);
    munmap(GPIO1_base_PC,pagesize*2);
    munmap(GPIO2_base_PC,pagesize*2);
    munmap(GPIO3_base_PC,pagesize*2);
    munmap(GPIO4_base_PC,pagesize*2);
    munmap(GRU_base_PC,pagesize*2);
	munmap(CRU_base_PC,pagesize*2);
}

int GPIO_ConfigPin(unsigned int gpio, unsigned int direction)
{
    unsigned int base = gpio / 32;
	unsigned int pin = gpio % 32;
	unsigned int offset = 1UL << pin;

    // Mux settings for raw GPIO.
	unsigned int gru_off = getGRUMuxOffset(base, pin);
	unsigned int gru_mask = getGRUMuxMask(base, pin);
	raw_write(gru_mask, (uint8_t*)GRU_base + gru_off);

	// Enable settinsg to make sure its not a weak pull-up/pull-down.
	gru_off = getGRUEnableOffset(base, pin);
	gru_mask = getGRUEnableMask(base, pin);
	raw_write(gru_mask, (uint8_t*)GRU_base + gru_off);

	// Enable gpio pclk
	unsigned int cru_off = getCRUEnableOffset(base,pin);
	unsigned int cru_mask = getCRUEnableMusk(base,pin);
	raw_write(cru_mask,(uint8_t*)CRU_base + cru_off);

	void *reg = getControlReg(base, pin);
	if (reg == 0) {
		return -1;
	}


	unsigned int val = raw_read((uint8_t*)reg + GPIO_SWPORT_DDR);
	if (direction == INPUT) {
		val &= ~offset;
	} else {
		val |= offset;
	}

	raw_write(val, (uint8_t*)reg + GPIO_SWPORT_DDR);

	return 0;
}


void GPIO_SetPin(unsigned int gpio, unsigned int val)
{
    unsigned int base = gpio / 32;
	unsigned int pin = gpio % 32;
	unsigned int offset = 1UL << pin;
	void *reg = getControlReg(base, pin);
	unsigned int newVal = raw_read((uint8_t*)reg + GPIO_SWPORT_DR);
	raw_write((newVal&~offset) | (val!=0 ? offset: 0x0), (uint8_t*)reg + GPIO_SWPORT_DR);
}

unsigned int GPIO_GetPin(unsigned int gpio)
{
 	unsigned int base = gpio / 32;
	unsigned int pin = gpio % 32;
	unsigned int offset = 1UL << pin;
	void *reg = getControlReg(base, pin);
	unsigned int newVal = raw_read((uint8_t*)reg+GPIO_EXT_PORTA) ;
	return newVal & offset;
}

int main()
{
    GPIO_Init();
    // RS : GPIO2_B6, 32*2 + 1*8 + 6 = 78
    // RST: GPIO1_C5, 32*1 + 2*8 + 5 = 53
	GPIO_ConfigPin(78,OUTPUT);
	GPIO_ConfigPin(53,OUTPUT);
	int count = 100;
	while(count--)
	{
		printf("count:%d\n",count);
		GPIO_SetPin(78,count%2);
		GPIO_SetPin(53,count%2);
		usleep(100000);
	}
    GPIO_Free();
    return 0;
}