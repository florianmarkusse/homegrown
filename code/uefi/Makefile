CFLAGS += \
        -target x86_64-unknown-windows \
        -ffreestanding \
        -fshort-wchar \
        -mno-red-zone \
        -Iinclude

LDFLAGS += \
		-target x86_64-unknown-windows \
		-nostdlib \
        -Wl,-entry:efi_main \
        -Wl,-subsystem:efi_application \
        -fuse-ld=lld-link

OBJECT := hello.o
BINARY := efi_application.efi

all: $(BINARY)

$(BINARY): $(OBJECT)
	clang $(LDFLAGS) -o $@ $^

$(OBJECT): hello.c
	clang $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJECT) $(BINARY)

