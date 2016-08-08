.PHONY: clean test

all:
	$(MAKE) -C src
	mv src/XcodeMLtoCXX .

test:
	$(MAKE) -C test

clean :
	rm XcodeMLtoCXX
	for dir in $(cleandirs); do $(MAKE) clean -C $$dir; done

cleandirs = src test
