.PHONY: check update

subdirs = $(wildcard */)

update :
	for dir in $(subdirs); do $(MAKE) TARGET=output.xml -C $$dir; done

check :
	for dir in $(subdirs); do $(MAKE) check -C $$dir; done

clean :
	for dir in $(subdirs); \
	do if [ -e $${dir}tmp.xml ]; then \
		rm $${dir}tmp.xml; \
	fi \
	done
