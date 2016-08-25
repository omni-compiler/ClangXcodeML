.PHONY: check update

subdirs = $(shell echo */)

update :
	for dir in $(subdirs); do $(MAKE) TARGET=output.xml -C $$dir; done

check :
	for dir in $(subdirs); do echo -n "test $${dir%/}: " ; $(MAKE) --no-print-directory check -C $$dir || exit 1; done

clean :
	for dir in $(subdirs); \
	do if [ -e $${dir}tmp.xml ]; then \
		rm $${dir}tmp.xml; \
	fi \
	done
