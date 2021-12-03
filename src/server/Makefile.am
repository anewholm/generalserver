## Process this file with automake to produce Makefile.in
ABS_DIR = $(abs_top_srcdir)/server
AUTOMAKE_OPTIONS = subdir-objects
SUBDIRS = QueryEnvironment Utilities Xml

AM_CPPFLAGS = $(INCLUDE_DIRS_STANDARD)
lib_LIBRARIES = libserver.a
libserver_a_SOURCES = \
$(ABS_DIR)/Main.cpp \
$(ABS_DIR)/Tests.cpp \
$(ABS_DIR)/Debug.cpp \
$(ABS_DIR)/Thread.cpp \
$(ABS_DIR)/RegularX.cpp \
$(ABS_DIR)/Database.cpp \
$(ABS_DIR)/DatabaseNode.cpp \
$(ABS_DIR)/DatabaseNodeServerObject.cpp \
$(ABS_DIR)/DatabaseClass.cpp \
$(ABS_DIR)/TXml.cpp \
$(ABS_DIR)/TXmlProcessor.cpp \
$(ABS_DIR)/Iterators.cpp \
$(ABS_DIR)/XSLFunctions.cpp \
$(ABS_DIR)/XJavaScript.cpp \
$(ABS_DIR)/Repository.cpp \
$(ABS_DIR)/FileSystem.cpp \
$(ABS_DIR)/RelationalDatabases.cpp \
$(ABS_DIR)/Conversation.cpp \
$(ABS_DIR)/MessageInterpretation.cpp \
$(ABS_DIR)/Request.cpp \
$(ABS_DIR)/Response.cpp \
$(ABS_DIR)/Server.cpp \
$(ABS_DIR)/Service.cpp \
$(ABS_DIR)/User.cpp \
$(ABS_DIR)/Session.cpp \
$(ABS_DIR)/MemoryLifetimeOwner.cpp \
$(ABS_DIR)/ProfilerThread.cpp \
$(ABS_DIR)/Exceptions.cpp
