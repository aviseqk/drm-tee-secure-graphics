.PHONY: tf-a optee uboot linux clean

optee:
	./scripts/build-optee.sh

uboot:
	./scripts/build-uboot.sh

linux:
	./scripts/build-linux.sh

tf-a: optee uboot linux
	./scripts/build-tfa.sh

clean:
	rm -rf out/*
