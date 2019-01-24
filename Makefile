SRCS := doclink_y.cpp \
        doclink_l.cpp \
        act.cpp \
        correlation.cpp \
        error.cpp \
        flow.cpp \
        gen.cpp \
        main.cpp \
        warning.cpp \


OBJS := $(SRCS:.cpp=.o)
DOCLINK = doclink.exe

DEBUG_FLAG = -g
CXXFLAGS = $(DEBUG_FLAG)

all:$(DOCLINK)

$(DOCLINK):$(OBJS)
	$(CXX) $(DEBUG_FLAG) -o $(DOCLINK) $(OBJS)

doclink_y.cpp:doclink.y
	bison -pdoclink_ -o $@ $< --defines=doclink_y.h

doclink_y.h:doclink.y

$(OBJS):doclink_y.h doclink.h

doclink_l.cpp:doclink.l
	flex -Pdoclink_ -t $< > $@

RM = rm -r -f

clean:
	$(RM) $(DOCLINK) $(OBJS) *.stackdump *~
	$(RM) doclink_y.cpp doclink_y.h doclink_l.cpp
	$(RM) x64 Debug Release .vs
	$(RM) doc.aux doc.log doc.dvi doc.pdf

