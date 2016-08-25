.PHONY: clean check

all:
	$(MAKE) -C src
	mv src/XcodeMLtoCXX .

check:
	$(MAKE) -C testcases

clean :
	rm XcodeMLtoCXX
	for dir in $(cleandirs); do $(MAKE) clean -C $$dir; done

cleandirs = src testcases
