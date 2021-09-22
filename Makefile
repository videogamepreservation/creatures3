
CFLAGS := -DC2E_SDL -DC2E_NO_INLINE_ASM -DC2E_OLD_CPP_LIB \
	-ftemplate-depth-32

LIBS := -lz -lSDL -lpthread


# (-ftemplate-depth-32 is required for Scriptorium.cpp)


# modules will add to this
SRC :=


# include all the module.mk files...
#include $(patsubst %,%/module.mk,$(MODULES))

include common/module.mk
include engine/module.mk



OBJ := \
	$(patsubst %.cpp,%.o,$(filter %.cpp,$(SRC))) 


# rule to compile .cpp files
%.o : %.cpp
	g++ -c $(CFLAGS) $< -o $@



all: lc2e


lc2e: depend $(OBJ)
	g++ -o lc2e $(OBJ) $(LIBS)


.PHONY: clean
clean:
	rm depend $(OBJ)

depend:
	makedepend -f- -- $(CFLAGS) -- $(SRC) >depend

include depend


