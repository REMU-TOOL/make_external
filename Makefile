.PHONY: all
all: make_external.so

make_external.so: make_external.cc
	yosys-config --build $@ $^

.PHONY: test
test: out.v 

out.v : test.v | make_external.so
	yosys -m make_external.so -p "hierarchy -top top; proc; opt; make_external" -o $@ $^

.PHONY: clean
clean:
	rm -f out.v make_external.so
