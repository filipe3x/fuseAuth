# source files.
SRC = passthrough.c

OBJ = $(SRC:.cpp=.o)

OUT = passthrough

# include directories
INCLUDES = -I. -I/usr/local/include/
 
# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -O2 -Wall `pkg-config fuse3 --cflags --libs`

# compiler
CCC = gcc

# library paths (libfuse3.so.3)
LIBS = -L/usr/local/lib/x86_64-linux-gnu

# compile flags
LDFLAGS = -g

.SUFFIXES: .cpp .c 

default: $(OUT)

.cpp.o:
	$(CCC) $(DEFINEMACRO) $(CCFLAGS) $(INCLUDES)  -c $< -o $@

.c.o:
	$(CCC) $(DEFINEMACRO) $(CCFLAGS) $(INCLUDES) -c $< -o $@

$(OUT): $(OBJ)
	$(CCC) -o $(OUT) $(CCFLAGS) $(OBJ) $(LIBS) && mkdir -p /tmp/pt_dir && ./$(OUT) -o allow_other -o default_permissions /tmp/pt_dir 

depend:  dep

clean:
	rm -f *.o $(OUT) .a *~ Makefile.bak 
	bash -c '[ -d /tmp/pt_dir ] && fusermount -u /tmp/pt_dir && rm -r /tmp/pt_dir'
