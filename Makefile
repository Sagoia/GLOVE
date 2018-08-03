.PHONY: default install debug_install cross_install cross_debug_install

NPROCS := $(shell grep -c ^processor /proc/cpuinfo)

default:
	@cd build && make -j $(NPROCS)

install:
	@cd build && make install

uninstall:
	@cd build && cat install_manifest.txt | grep .so | xargs rm

debug:
	@cd build_debug && make -j $(NPROCS)

debug_install:
	@cd build_debug && make install

cross:
	@cd cross_build && make -j $(NPROCS)

cross_install:
	@cd cross_build && make install

cross_debug:
	@cd cross_build_debug && make -j $(NPROCS)

cross_debug_install:
	@cd cross_build_debug && make install

clean:
	@rm -rf build build_debug cross_build cross_build_debug
