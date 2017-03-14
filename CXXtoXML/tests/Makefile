.PHONY: clean check

TESTDIRS = CppTest

check:
	set -e; \
	for dir in $(TESTDIRS); do \
		$(MAKE) -C $$dir check; \
	done

clean:
	set -e ; \
	for dir in $(TESTDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
