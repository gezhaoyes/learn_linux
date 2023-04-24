TARGET          ?=bsp
ATEMP         := bsp/clk \
				 bsp/led \
				 bsp/delay \
				 bsp/beep \
				 bsp/key \
				 bsp/gpio \
				 bsp/int \
				 bsp/epittimer \
				 bsp/uart \
				 bsp/lcd \
				 bsp/rtc \
				 bsp/i2c \
				 bsp/spi

LIBPATH	:= -lgcc -L /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/lib/gcc/arm-linux-gnueabihf/4.9.4
INCDIRS       := imx6ul stdio/include $(ATEMP)
SRCDIRS       := project stdio/lib $(ATEMP)
			 
INCLUDE       :=$(patsubst %,-I %,$(INCDIRS)) #所有的单词前面都加上-I

#每次将第二个量中的单词放到逐一放到第一个量中，然后执行第三个表达式，返回执行结果构成的字符串
#wildcard函数将通配符展开
SFILES        :=$(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.S))
CFILES        :=$(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))

SFILENDIR     :=$(notdir $(SFILES))  #去掉路径
CFILENDIR     :=$(notdir $(CFILES))

SOBJS         :=$(patsubst %.S,obj/%.o,$(SFILENDIR))#所有.S后缀的文件都加上obj/,且后缀变为.o
COBJS         :=$(patsubst %.c,obj/%.o,$(CFILENDIR))
export OBJS          :=$(SOBJS) $(COBJS)

VPATH         :=$(SRCDIRS)

CROSS_COMPILE ?= arm-linux-gnueabihf-
GCC           :=$(CROSS_COMPILE)gcc -Wall -march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard -Wa,-mimplicit-it=thumb -nostdlib -fno-builtin -c -O2 $(INCLUDE) -o
LD            :=$(CROSS_COMPILE)ld -Timx6ul.lds -o
OBJCOPY       :=$(CROSS_COMPILE)objcopy -O binary -S
OBJDUMP       :=$(CROSS_COMPILE)objdump -D -m arm

.PHONY:clean

$(TARGET).bin:$(OBJS)
	$(LD) $(TARGET).elf $^ $(LIBPATH)
	$(OBJCOPY) $(TARGET).elf $@
	$(OBJDUMP) $(TARGET).elf > $(TARGET).dis
		
obj/%.o:%.S
	$(GCC) $@ $<
	
obj/%.o:%.c
	$(GCC) $@ $<
	
clean:
	rm -rf *.o $(TARGET).bin $(TARGET).elf $(TARGET).dis $(OBJS)

print:
	make -f makefile1
