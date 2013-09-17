TARGET := libtinyoal.so
SRCDIR := TinyOAL
BUILDDIR := bin
OBJDIR := bin/obj
C_SRCS := $(wildcard $(SRCDIR)/*.c)
CXX_SRCS := $(wildcard $(SRCDIR)/*.cpp)
INCLUDE_DIRS := include
LIBRARY_DIRS := 
LIBRARIES := rt soft_oal

CPPFLAGS += -fPIC -D BSS_UTIL_EXPORTS -Wall -Wno-attributes -Wno-unknown-pragmas -Wno-reorder -Wno-missing-braces -std=gnu++0x
LDFLAGS += -shared

include base.mk

distclean:
	@- $(RM) $(OBJS)
	@- $(RM) -r $(OBJDIR)
