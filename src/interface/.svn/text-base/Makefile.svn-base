CXX = g++

INTERFACE_SOURCES = TopupInterface_types.cpp Topup.cpp TopupInterface_constants.cpp

INTERFACE_OBJS = $(INTERFACE_SOURCES:.cpp=.o)





INC_THRIFT = -I../../include -I/data/boost-install/include

CXXFLAG = -g -Wall 

.cpp.o:
	$(CXX) -c $(CXXFLAG) $(INC_THRIFT) -fPIC   $< -o $@ 

all: libinterface.so test 

libinterface.so: $(INTERFACE_OBJS)
	$(CXX) -shared -o $@ $<

test: Topup_server.skeleton.o
	$(CXX) $(CXXFLAG) $(INC_THRIFT) -L../../lib -lthrift -o $@ $< $(INTERFACE_OBJS)
clean:
	rm -f $(INTERFACE_OBJS)
	
