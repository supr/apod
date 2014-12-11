TARG=pod
LDFLAGS=-lcurl
CFLAGS=-std=c++11 -Wall -Werror -g
CXX=clang++
CC=clang

SRC_DIR=src
BUILD=build

OBJ=main.o

BUILD_OBJ=$(OBJ:%.o=$(BUILD)/%.o)

.PHONY: all build clean

all: $(TARG)

$(BUILD)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(@D)
	@echo "(CXX) $@"
	$(CXX) $(CFLAGS) -c -o $@ $<

clean:
	@rm -rf $(BUILD)

$(TARG): $(BUILD_OBJ)
	$(CXX) $(CLFAGS) $(LDFLAGS) -o $(BUILD)/$@ $<

run: $(TARG)
	$(BUILD)/$(TARG)
