#//////////////////////////////////////////////////////////////////////////
CPP = g++
MAKE = make
FLAGS = -Wall -Werror 
FLAGS += -g
#FLAGS += -DDEBUG

LIB_XML_PATH = ./XmlLib
LIB_XML = $(LIB_XML_PATH)/libxml.a

LIB_DB_PATH = ./DBLib

INCS = -I./
INCS += -I$(LIB_XML_PATH)
INCS += -I$(LIB_DB_PATH)

ORACLE_H = /home/oracle/app/oracle/product/10g
CDBFLAGS = -I$(ORACLE_H)/plsql/public 
CDBFLAGS += -I$(ORACLE_H)/rdbms/public 
LDDBFLAGS = -L$(ORACLE_H)/lib -lclntsh

OBJS = distribute.o Communication.o CDBOption.o CXmlConfig.o Common.o \
    CRunLog.o CProfile.o CMachine.o CDisFile.o CDistribute.o

#//////////////////////////////////////////////////////////////////////////
TARGET = distribute

all: $(TARGET)

$(TARGET): $(OBJS) $(LIB_XML)
	$(CPP) -o $(TARGET) $(OBJS) $(FLAGS) $(LIB_XML) $(LDDBFLAGS) $(CDBFLAGS)

%.o: %.cpp
	$(CPP) -o $@ -c $< $(INCS) $(FLAGS) $(CDBFLAGS)

$(LIB_XML):
	$(MAKE) -C $(LIB_XML_PATH)

clean: 
	rm -f $(TARGET)
	rm -f $(OBJS)
	$(MAKE) -C $(LIB_XML_PATH) clean

#//////////////////////////////////////////////////////////////////////////
