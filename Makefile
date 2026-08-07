.PHONY: tf-a optee uboot clean

optee:
	./scripts/build-optee.sh

uboot:
	./scripts/build-uboot.sh

tf-a: optee uboot
	./scripts/build-tfa.sh

clean:
	rm -rf out/*
