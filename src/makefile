SUBDIRS = common common_server client server

subdirs:	
	for dir in $(SUBDIRS); do \
	    $(MAKE) -C $$dir; \
        done; \
        for dir in $(SUBDIRS); do \
            rm -rf *.o; \
        done

clean:
	for dir in $(SUBDIRS); do \
	    $(MAKE) -C $$dir clean; \
	done