#  DynamicPath ...  Makefile puts  our *so libraries!
#  smart trick I define  $w
#
rs   := $(shell echo $$ROOTSYS)
date := $(shell date +%Y%m%d_%H%M%S)
mpath2 := $(shell cat ~/.rootrc | grep -v -e "^\#" | grep Unix | grep DynamicPath | awk '{print $$2}' | sed  's/(BFARCH)/x64/g' | sed  's/(ROOTSYS)/w/g' | sed  's/:/ /g'  )
#mpath2 := $(shell echo  $(mpath) | sed  's/:/ /g')

all:  nrfit.C 
	$(rs)/bin/root -n -b -q  compile.C  

clean:
	rm nrfit_C.so

install: nrfit_C.so 
	@echo I TRY TO FIND PATH for dynamic .SO load;\
	w=$(rs); x64=x64;\
	echo dynamic $(ROOTSYS) paths is $(mpath2);\
	TARG=""; \
	for i in $(mpath2); do \
	echo "testing ... " $$i;\
	if [ -d "$$i" ]; then \
	 	if [ $$i = "./" ]; then echo "       current dir ";else \
		if [ "$$TARG" = "" ]; then \
			TARG=$$i;\
			cp nrfit_C.so $$TARG/ ;\
			if [ $$? != 0 ]; then TARG="";fi; \
		fi; \
	 	echo "       OK " $$i "  targ =" $$TARG; \
		fi; \
	 else \
		 echo "       no such dir"; \
	fi; \
	done;\
	echo "Selected= <$$TARG>";\
	if [ "$$TARG" != "" ];then \
		ls -l $$TARG/*.so;\
	else\
		echo ERROR; echo "  " NO PATH FOR MACRO. CREATE SOME inside ~/.rootrc;\
	fi;


