.PHONY: clean check

TESTDIRS = Compilability CCTest

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
