# FIXME: compile error
DIR_BUILD := ./build
CXXSRCS := $(wildcard src/**/*.cpp) $(wildcard src/*.cpp)
# print CXXSRCS
$(info $(CXXSRCS))

OBJS = $(CXXSRCS:%.cpp=$(DIR_BUILD)/objs/%.o)
CXX = g++ --std=c++17 -O2 
CXXFLAGS = -L/extlibs -I/extlibs -L/usr/local/lib/ -I/usr/local/include/antlr4-runtime/ -lm -lantlr4-runtime

$(DIR_BUILD)/objs/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $< 
compiler: $(OBJS) 
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -lm -o $@ $^ 
