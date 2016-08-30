.PHONY: clean check

TESTDIRS = UnitTest Compilability

all:
	for dir in $(TESTDIRS); do $(MAKE) -C $$dir check; done

clean:
	for dir in $(TESTDIRS); do $(MAKE) -C $$dir clean; done
