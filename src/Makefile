CXX=g++
DBG_CXXFLAGS= -D_DEBUG -g
CXXFLAGS= -w $(DBG_CXXFLAGS) 
p1_SOURCES=dip_p1.cpp
p2_SOURCES=dip_p2.cpp
p1_OBJS=$(p1_SOURCES:.cpp=.o)
p2_OBJS=$(p2_SOURCES:.cpp=.o)

all: dip_p1 dip_p2

dip_p1: $(p1_OBJS)
	$(CXX) $(CXXFLAGS) -o dip_p1 $(p1_OBJS)

dip_p2: $(p2_OBJS)
	$(CXX) $(CXXFLAGS) -o dip_p2 $(p2_OBJS)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< 

clean:
	rm -fr dip_p1 dip_p2 *.o
