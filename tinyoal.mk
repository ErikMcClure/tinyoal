TARGET := libtinyoal.so
SRCDIR := TinyOAL
BUILDDIR := bin
OBJDIR := bin/obj
C_SRCS := $(wildcard $(SRCDIR)/*.c)
CXX_SRCS := $(wildcard $(SRCDIR)/*.cpp)
INCLUDE_DIRS := include bss-util/include
LIBRARY_DIRS := 
LIBRARIES := rt

CPPFLAGS += -fPIC -D TINYOAL_EXPORTS -Wall -Wno-attributes -Wno-unknown-pragmas -Wno-reorder -Wno-missing-braces -Wno-unused-function -std=c++17 
LDFLAGS += -shared

include base.mk

distclean:
	@- $(RM) $(OBJS)
	@- $(RM) -r $(OBJDIR)
