.PHONY: release debug run-release run-debug callgrind-image run-callgrind callgrind-viz-image callgrind-png gprof-image run-gprof run-memcheck run-helgrind clean-ddimage clean-dall

release:
	docker build -f .devcontainer/Dockerfile --build-arg BUILD_TYPE=Release --force-rm -t event-bus:release .

debug:
	docker build -f .devcontainer/Dockerfile --build-arg BUILD_TYPE=Debug --force-rm -t event-bus:debug .

# usage: make run-release ARGS="--your --args"
run-release: release
	docker run --rm -it event-bus:release ./event-bus $(ARGS)

run-debug: debug
	docker run --rm -it event-bus:debug bash

# Build an image that has valgrind installed
callgrind-image:
	docker build -f .devcontainer/Dockerfile --build-arg BUILD_TYPE=RelWithDebInfo --build-arg WITH_VALGRIND=1 --force-rm -t event-bus:callgrind .

# Build an image that can convert callgrind output to PNG (gprof2dot + graphviz)
callgrind-viz-image:
	docker build -f .devcontainer/Dockerfile --build-arg BUILD_TYPE=RelWithDebInfo --build-arg WITH_GRAPHVIZ=1 --force-rm -t event-bus:callgrind-viz .

# Run callgrind and write output to ./callgrind on the host
run-callgrind: callgrind-image
	mkdir -p callgrind
	MSYS2_ARG_CONV_EXCL="*" docker run --rm -it \
		-v "$(CURDIR)/callgrind:/app/callgrind" \
		-w /app/callgrind \
		event-bus:callgrind \
		valgrind --tool=callgrind --callgrind-out-file=callgrind.out /app/event-bus $(ARGS)

# Generate callgrind/callgrind.png from callgrind/callgrind.out
callgrind-png: callgrind-viz-image
	MSYS2_ARG_CONV_EXCL="*" docker run --rm -it \
		-v "$(CURDIR)/callgrind:/app/callgrind" \
		-w /app/callgrind \
		event-bus:callgrind-viz \
		sh -lc "gprof2dot -f callgrind callgrind.out | dot -Tpng -o callgrind.png"

# Build an image that has a gprof-instrumented binary (-pg) and gprof tooling
gprof-image:
	docker build -f .devcontainer/Dockerfile --build-arg BUILD_TYPE=RelWithDebInfo --build-arg ENABLE_GPROF=ON --build-arg WITH_GPROF=1 --force-rm -t event-bus:gprof .

# Run the binary to produce gprof/gmon.out, then write gprof/gprof.txt
run-gprof: gprof-image
	mkdir -p gprof
	MSYS2_ARG_CONV_EXCL="*" docker run --rm -it \
		-v "$(CURDIR)/gprof:/app/gprof" \
		-w /app/gprof \
		event-bus:gprof \
		sh -lc "/app/event-bus $(ARGS); gprof /app/event-bus gmon.out > gprof.txt"

# Run Valgrind Memcheck and write memcheck/memcheck.log
run-memcheck: callgrind-image
	mkdir -p memcheck
	MSYS2_ARG_CONV_EXCL="*" docker run --rm -it \
		-v "$(CURDIR)/memcheck:/app/memcheck" \
		-w /app/memcheck \
		event-bus:callgrind \
		valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes \
			--log-file=/app/memcheck/memcheck.log /app/event-bus $(ARGS)

# Run Valgrind Helgrind and write helgrind/helgrind.log
run-helgrind: callgrind-image
	mkdir -p helgrind
	MSYS2_ARG_CONV_EXCL="*" docker run --rm -it \
		-v "$(CURDIR)/helgrind:/app/helgrind" \
		-w /app/helgrind \
		event-bus:callgrind \
		valgrind --tool=helgrind --log-file=/app/helgrind/helgrind.log /app/event-bus $(ARGS)

clean-ddimage:
	docker image prune -f

clean-dall:
	docker system prune -a -f
