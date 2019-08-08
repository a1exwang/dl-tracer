all: dl_tracer_test

tracer.o: tracer.S
	as -o tracer.o tracer.S

dl_tracer_test: tracer.o dl_tracer_test.cpp target.cpp stack_dump.cpp
	g++ -g -o dl_tracer_test dl_tracer_test.cpp tracer.o target.cpp stack_dump.cpp -I3rdparty/backward-cpp -ldl

clean:
	rm -f tracer.o dl_tracer_test
