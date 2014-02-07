### CONFIG

SOURCE_DIR = ./

LLVM_HOME_DIR = /home/legup/legup-3.0/llvm/Release+Asserts/bin/
FRONT_END = clang
LEGUP_LIB_DIR = /home/legup/legup-3.0/examples/lib/
# fix for some Ubuntu distros
CFLAGS = -I/usr/include/i386-linux-gnu/
LDFLAGS = 
OPT_FLAGS = -load=$(LLVM_HOME_DIR)../lib/LLVMLegUp.so -legup-config=$(SOURCE_DIR)legup.tcl

OBJS = oil.prelto.2.bc

### RULES

all: oil.v

# first compilation
%.prelto.1.bc : $(SOURCE_DIR)%.c
	$(FRONT_END) -emit-llvm -c -fno-builtin -m32 -O0 -mllvm -inline-threshold=-100 $(CFLAGS) -o $@ $<

# optimization stage
%.prelto.2.bc : %.prelto.1.bc
ifdef UNROLL
	$(LLVM_HOME_DIR)opt -mem2reg -loops -loop-simplify -loop-unroll $(UNROLL) < $< > $@
else
	cp $< $@
endif

# linking may produce llvm mem-family intrinsics
%.prelto.linked.bc : $(OBJS)
	$(LLVM_HOME_DIR)llvm-ld -disable-inlining -disable-opt $(LDFLAG) $^ -b=$@

# performs intrinsic lowering so that the linker may be optimized
%.prelto.bc : %.prelto.linked.bc
	$(LLVM_HOME_DIR)opt $(OPT_FLAGS) -legup-prelto < $< > $@

%.postlto.bc : %.prelto.bc
	$(LLVM_HOME_DIR)llvm-ld $(LDFLAG) $< $(LEGUP_LIB_DIR)llvm/liblegup.a $(LEGUP_LIB_DIR)llvm/libm.a -b=$@
	
%.bc : %.postlto.bc
	$(LLVM_HOME_DIR)opt $(OPT_FLAGS) -basicaa -loop-simplify -modulo-schedule < $< > $@

# produces textual representation of bitcode
%.ll : %.bc
	$(LLVM_HOME_DIR)llvm-dis $<

# produces verilog: .v
%.v : %.bc
	$(LLVM_HOME_DIR)llc $(LLC_FLAGS) -march=v $< -o $@

.PHONY: clean
clean:
	rm -f oil.v 
	rm -f *.prelto.{1,2,linked}.bc
	rm -f *.postlto.bc 
	rm -f *.bc 
	rm -f *.ll
	rm -f *.o

.PHONY: all

