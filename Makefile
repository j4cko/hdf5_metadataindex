CXXFLAGS:= -g -std=c++11 -Wall
HDRS_INDEXER:= attributes.h indexHdf5.h h5helpers.h sqliteHelpers.h
OBJS_INDEXER:= attributes.o indexHdf5.o sqliteHelpers.cc
DEPS_INDEXER:= $(HDRS_INDEXER) $(OBJS_INDEXER)

PROGS:= indexHdf5 

all: $(PROGS)

%.o: %.cc $(HDRS_INDEXER)
	$(CXX) -c -o $@ $(CXXFLAGS) $<

indexHdf5: indexer.o $(DEPS_INDEXER)
	$(CXX) -o $@ $(CXXFLAGS) -lhdf5 -lsqlite3 $< $(OBJS_INDEXER)

clean:
	rm -vf *~ *.o $(PROGS)
