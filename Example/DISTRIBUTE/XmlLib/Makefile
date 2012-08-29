CPP = g++
AR = ar
RANLIB = ranlib
FLAGS = -Wall -Werror -O4
#FLAGS += -g
INCS = -I.
OBJS = tinystr.o tinyxml.o tinyxmlerror.o tinyxmlparser.o
TARGET = libxml.a

all: $(TARGET)

$(TARGET): $(OBJS)
	-rm -f $(TARGET)
	$(AR) cru $(TARGET) $(OBJS)
	$(RANLIB) $(TARGET)

lib: $(OBJS)
	$(CPP) -shared -o libxml.so $(OBJS)

test: $(OBJS) test.o
	$(CPP) -o test test.o $(OBJS) $(INCS)

%.o: %.cpp
	$(CPP) -c $< -o $@ $(FLAGS) $(INCS)

.PHONY: clean

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)
	rm -f libxml.so
	@rm -f *~
	@rm -f test test.o
