.PHONY: test clean

check :
	$(MAKE) check -C testcases

clean :
	for dir in $(cleandirs); do $(MAKE) clean -C $$dir; done

cleandirs = src testcases
