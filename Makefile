# Makefile for NetPower Messenger

#--------------------------------------------------------------------------
ifeq ("$(wildcard Makefile.option)", "")
TARGET = Makefile.option
ifneq ("$(MAKECMDGOALS)", "")
TARGET = $(MAKECMDGOALS)
endif
$(TARGET):
	@echo Run ./configure first to create Makefile.option
else # Makefile.option

include Makefile.option

PLATFORM = $(shell uname)

#--------------------------------------------------------------------------

CC = g++

#--------------------------------------------------------------------------

INC_DIRS = $(OPENSSL_DIR)/include/
LIB_DIRS = $(OPENSSL_DIR)/lib/

OUT_DIR = Output
TMP_DIR = Temporary

OUT_PATH = $(OUT_DIR)/$(PLATFORM)/$(CONFIG)
TMP_PATH = $(TMP_DIR)/$(PLATFORM)/$(CONFIG)

#--------------------------------------------------------------------------

CFLAGS = -c -Wall -Werror
LFLAGS = -Wall

ifeq ("$(CONFIG)", "Debug")
CFLAGS += -D_DEBUG -g
LFLAGS += -D_DEBUG -g
else
CFLAGS += -DNDEBUG -O4
LFLAGS += -DNDEBUG -O4
endif

#--------------------------------------------------------------------------

CMS = $(OUT_PATH)/tap-cms-messenger
IDS = $(OUT_PATH)/tap-ids-messenger
TEST = $(TMP_PATH)/UnitTests/unittests

CMS_MODULES = \
	Source/CmsMessenger/Main \
	Source/CmsMessenger/SafeIPList \
	Source/CmsMessenger/MessengerServer \
	Source/CmsMessenger/ControlServer

IDS_MODULES = \
	Source/IdsMessenger/Main

TEST_MODULES = \
	Source/CmsMessenger/SafeIPList \
	UnitTests/SafeIPTest \
	UnitTests/SafeIPListTest \
	UnitTests/Main

#--------------------------------------------------------------------------

TARGETS = $(TEST) $(CMS) $(IDS)
MODULES = $(TEST_MODULES) $(CMS_MODULES) $(IDS_MODULES)

#--------------------------------------------------------------------------

.PHONY: all
.PHONY: clean

all: $(TARGETS)

clean:
	@rm -vf $(TARGETS) \
		$(MODULES:%=$(TMP_PATH)/%.d) \
		$(MODULES:%=$(TMP_PATH)/%.o)
	@[ ! -d $(OUT_DIR) ] || rmdir -v `find $(OUT_DIR) -type d | sort -r` --ignore-fail-on-non-empty
	@[ ! -d $(TMP_DIR) ] || rmdir -v `find $(TMP_DIR) -type d | sort -r` --ignore-fail-on-non-empty

$(CMS): $(CMS_MODULES:%=$(TMP_PATH)/%.o)
	@[ -d $(@D) ] || mkdir -p $(@D)
	$(CC) $(LFLAGS) $(LIB_DIRS:%=-L%) $^ -o $@ -lpthread -lssl -lcrypto -ldl

$(IDS): $(IDS_MODULES:%=$(TMP_PATH)/%.o)
	@[ -d $(@D) ] || mkdir -p $(@D)
	$(CC) $(LFLAGS) $(LIB_DIRS:%=-L%) $^ -o $@ -lpthread -lssl -lcrypto -ldl

$(TEST): $(TEST_MODULES:%=$(TMP_PATH)/%.o)
	@[ -d $(@D) ] || mkdir -p $(@D)
	$(CC) $(LFLAGS) $^ -o $@
	$@

#--------------------------------------------------------------------------
# Dependencies

ifneq ("$(MAKECMDGOALS)", "clean") 

sinclude $(MODULES:%=$(TMP_PATH)/%.d)

$(TMP_PATH)/%.d: %.cpp
	@echo "Parsing dependency for $< ($(CONFIG))"
	@[ -d $(@D) ] || mkdir -p $(@D)
	@$(CC) -MM $(INC_DIRS:%=-I%) $< -MT $(@:%.d=%.o) > $@.tmp
	@sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.tmp > $@
	@rm -f $@.tmp
endif

$(TMP_PATH)/%.o: %.cpp
	@[ -d $(@D) ] || mkdir -pv $(@D)
	$(CC) $(CFLAGS) $(INC_DIRS:%=-I%) $< -o $@

#--------------------------------------------------------------------------
endif # Makefile.option

