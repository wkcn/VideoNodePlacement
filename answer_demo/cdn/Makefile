TARGET := test

CXX := g++
CFLAGS := -Wreturn-type -Ilib 

LDFLAGS := 

SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %cpp,%o,$(SRCS))  

all: $(OBJS) 
	$(CXX) $^ $(CFLAGS) $(LDFLAGS) -o $(TARGET)
clean:  
	rm $(TARGET) src/*.o
clean_all:
	rm $(TARGET) $(OBJS) 
%.o:%.cpp  
	$(CXX) $< $(CFLAGS) $(LDFLAGS) -c  -o $@ 
