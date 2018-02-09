.PHONY: clean check

TESTDIRS = compile run CCTest

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
