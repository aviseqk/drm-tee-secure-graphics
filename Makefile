.PHONY: tf-a optee uboot linux buildroot run clean

optee:
	./scripts/build-optee.sh

uboot:
	./scripts/build-uboot.sh

linux:
	./scripts/build-linux.sh

buildroot:
	./scripts/build-buildroot.sh

tf-a: optee uboot linux buildroot
	./scripts/build-tfa.sh

run: tf-a
	./scripts/run-qemu.sh

clean:
	rm -rf out/*
