
all:
	i686-elf-as ./base/boot.asm -o boot.o
	x86_64-elf-g++ -c ./base/kernel.cpp -o kernel.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -m32 -I"/home/alex/Документы/osdev/LuneOS/"
	i686-elf-gcc  -T ./base/LINK.ld -o kernel -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc
	cp ./kernel ./iso/boot/kernel
	rm ./kernel.o ./boot.o ./kernel
	grub-mkrescue --install-modules="multiboot normal" -o myos.iso ./iso --compress=gz --fonts= --locales= --themes=
