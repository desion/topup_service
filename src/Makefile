CXX = g++

ROOT_DIR = ./
BOOST_DIR=/data/boost-install/

INTERFACE_DIR = $(ROOT_DIR)/interface

SOURCES = BaseBusiness.cpp \
		  ConnectionManager.cpp \
		  GlobalConfig.cpp \
		  TopupService.cpp \
		  HttpClient.cpp \
		  TopupUtils.cpp \
		  ChargeBusiness.cpp \
		  RedisClient.cpp 

OBJS = $(SOURCES:.cpp=.o)
THRIFT_OBJS = $(INTERFACE_DIR)/Topup.o \
			  $(INTERFACE_DIR)/TopupInterface_types.o  \
			  $(INTERFACE_DIR)/TopupInterface_constants.o

HTTP_OBJS = HttpClient.o

UTILS_OBJS = TopupUtils.o

HANDLE_OBJS = OrderHandler.o TopupHandleMain.o GlobalConfig.o \
			  TopupUtils.o ChargeBusiness.o RedisClient.o \
			  BaseBusiness.o ChannelSLS.o ConnectionManager.o \
			  HttpClient.o ChannelImpl.o TopupImpl.o \
			  $(INTERFACE_DIR)/TopupInterface_types.o \
			  TopupCustomer.o ChannelLLWW.o 


TOPUP_OBJS = TopupImpl.o \
			 TopupUtils.o \
			 HttpClient.o \
			 $(INTERFACE_DIR)/TopupInterface_types.o \
			 ChargeBusiness.o \
			 BaseBusiness.o \
			 RedisClient.o 

CUSTOMER_OBJS = TopupCustomer.o \
			 TopupUtils.o \
			 HttpClient.o \
			 $(INTERFACE_DIR)/TopupInterface_types.o \
			 ChargeBusiness.o \
			 BaseBusiness.o \
			 RedisClient.o 

CHANNEL_OBJS = ChannelImpl.o \
			   ChannelSLS.o \
			   TopupUtils.o \
			   HttpClient.o

TOPUP_SO = libtopup.so 
CUSTOMER_SO = libcustomer.so
CHANNEL_SO = libchannel.so
HANDLE_EXE = topup_handle



CXXFLAG = -g -Wall 
SHARE_FLAG = -fPIC

INCS = -I../include \
	   -I../include/occi \
	   -I../include/thrift \
	   -I$(BOOST_DIR)/include \
	   -I./interface \
	   -I../include/fastcgi \
	   -I../lib64/common/include

LIBS = -L../lib \
	   -L../lib64/common/lib

TEST = test_httpclient

all: oracle_client topup_server topup.fcg $(TEST) $(TOPUP_SO) $(CUSTOMER_SO) $(CHANNEL_SO) $(HANDLE_EXE)


.cpp.o:
	$(CXX) -c $(CXXFLAG) $(INCS) $(LIBS) $(SHARE_FLAG)  $< -o $@ 


oracle_client: oracle.o
	$(CXX) $(CPPFLAG) $(INCS) $(LIBS) -Wl,--rpath -Wl,/data/occi/lib  -locci  -lclntsh -lnnz11 -lglog -lpthread -o $@  $<

$(TOPUP_SO): $(TOPUP_OBJS)
	$(CXX) -g -fPIC -shared -o $@ $^
	cp $(TOPUP_SO) /data/occi/lib/

$(CUSTOMER_SO): $(CUSTOMER_OBJS)
	$(CXX) -g -fPIC -shared  -o $@ $^
	cp $(CUSTOMER_SO) /data/occi/lib/

$(CHANNEL_SO): $(CHANNEL_OBJS)
	$(CXX) -g -fPIC -shared -o $@ $^

topup_server: $(OBJS) TopupServer.o
	$(CXX) $(CPPFLAG) $(INCS) $(LIBS)  TopupServer.o $(OBJS) $(THRIFT_OBJS) -Wl,--rpath -Wl,/data/occi/lib \
		-locci -lclntsh -lnnz11 -lglog -liniparser -lcurl -lthrift -lthriftnb -lpthread \
	   	-levent -lcommon -ltinyxml -ljson_linux-gcc-4.1.2_libmt -lhiredis -o $@

$(HANDLE_EXE): $(HANDLE_OBJS)
	$(CXX) $(CPPFLAG) $(INCS) $(LIBS)  $(HANDLE_OBJS)  -Wl,--rpath -Wl,/data/occi/lib \
		-locci -lclntsh -lnnz11 -lglog -liniparser -lcurl -lthrift -lthriftnb -lpthread \
	   	-levent -lcommon -ltinyxml -ljson_linux-gcc-4.1.2_libmt -lhiredis -o $@

topup.fcg: $(HTTP_OBJS) TopupFcgi.o
	$(CXX) $(CPPFLAG) $(INCS) $(LIBS)  TopupFcgi.o $(HTTP_OBJS) $(THRIFT_OBJS) \
		-lfcgi -Wl,--rpath -Wl,/data/occi/lib -lcurl -lthrift -lthriftnb -lpthread -levent -ltinyxml -ljson_linux-gcc-4.1.2_libmt -o $@


test_httpclient: HttpClient.o Test_HttpClient.o
	$(CXX) $(CPPFLAG) $(INCS) $(LIBS)  Test_HttpClient.o HttpClient.o ConnectionManager.o GlobalConfig.o RedisClient.o -Wl,--rpath -Wl,/data/occi/lib \
	   	-lcurl -liconv -liniparser -lcommon -lglog -locci -lclntsh -lnnz11 -ltinyxml -ljson_linux-gcc-4.1.2_libmt -lhiredis -o $@

clean:
	rm *.o -f
	rm *.so
	rm oracle_client topup_server topup.fcg $(TEST) $(TOPUP_SO) $(CUSTOMER_SO)
