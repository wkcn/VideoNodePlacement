TARGET := test

CXX := g++
CFLAGS := -Wreturn-type -Ilib 

LDFLAGS := 

SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %cpp,%o,$(SRCS))  

all: $(OBJS) 
	$(CXX) $^ $(CFLAGS) $(LDFLAGS) -o $(TARGET)
clean:  
	rm $(TARGET) *.o
clean_all:
	rm $(TARGET) $(OBJS) 
test:
	./$(TARGET) ../../case_example/case0.txt out1.txt
%.o:%.cpp  
	$(CXX) $< $(CFLAGS) $(LDFLAGS) -c  -o $@ 
