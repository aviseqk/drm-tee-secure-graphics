.PHONY: tf-a optee uboot linux run clean

optee:
	./scripts/build-optee.sh

uboot:
	./scripts/build-uboot.sh

linux:
	./scripts/build-linux.sh

tf-a: optee uboot linux
	./scripts/build-tfa.sh

run: tf-a
	./scripts/run-qemu.sh

clean:
	rm -rf out/*
