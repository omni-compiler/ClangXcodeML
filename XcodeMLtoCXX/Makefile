.PHONY: clean

all:
	$(MAKE) -C src
	mv src/XcodeMLtoCXX .

clean :
	rm XcodeMLtoCXX
	for dir in $(cleandirs); do $(MAKE) clean -C $$dir; done

cleandirs = src
