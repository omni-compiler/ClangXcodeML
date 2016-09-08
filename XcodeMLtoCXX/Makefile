.PHONY: clean check

all:
	$(MAKE) -C src
	mv src/XcodeMLtoCXX .

check:
	$(MAKE) -C tests

clean :
	rm -f XcodeMLtoCXX
	for dir in $(cleandirs); do $(MAKE) clean -C $$dir; done

cleandirs = src tests
