ABS_DIR = $(abs_top_srcdir)/server/Utilities

# GLIBCPP = /usr/include/c++/7/
# GLIBCPP32 = /usr/include/x86_64-linux-gnu/c++/7/x32/
# GLIBC = /usr/src/linux-headers-4.13.0-32-generic/include/
# STANDARD = -std=c99 -m32

# $(ABS_DIR)/StringMultiMap.cpp
# $(ABS_DIR)/StringMap.cpp
# $(ABS_DIR)/StringVector.cpp

AUTOMAKE_OPTIONS = subdir-objects
AM_CPPFLAGS = $(INCLUDE_DIRS_STANDARD)
lib_LIBRARIES = libutilities.a
libutilities_a_SOURCES = \
$(ABS_DIR)/strtools.cpp \
$(ABS_DIR)/syntax2.cpp \
$(ABS_DIR)/regexpr2.cpp \
$(ABS_DIR)/StringMap.cpp \
$(ABS_DIR)/StringVector.cpp \
$(ABS_DIR)/StringMultiMap.cpp
