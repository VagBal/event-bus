.PHONY: release debug coverage run-release run-debug callgrind-image run-callgrind callgrind-viz-image callgrind-png gprof-image run-gprof run-memcheck run-helgrind clean-ddimage clean-dall all-tools

all-tools: coverage run-callgrind run-gprof run-memcheck run-helgrind
	@echo "=========================================="
	@echo "All analysis tools completed!"
	@echo "=========================================="
	@echo "Results available in .tool_result/:"
	@echo "  - Coverage:  .tool_result/coverage/coverage_html/index.html"
	@echo "  - Callgrind: .tool_result/callgrind/callgrind.out"
	@echo "  - Gprof:     .tool_result/gprof/gprof.txt"
	@echo "  - Memcheck:  .tool_result/memcheck/memcheck.log"
	@echo "  - Helgrind:  .tool_result/helgrind/helgrind.log"
	@echo "=========================================="

release:
	docker build -f .devcontainer/Dockerfile --build-arg BUILD_TYPE=Release --force-rm -t event-bus:release .

debug:
	docker build -f .devcontainer/Dockerfile --build-arg BUILD_TYPE=Debug --force-rm -t event-bus:debug .

coverage:
	docker build -f .devcontainer/Dockerfile --build-arg BUILD_TYPE=Debug --force-rm -t event-bus:coverage .
	@echo "Extracting coverage report from container..."
	@mkdir -p .tool_result/coverage
	@docker create --name temp-coverage event-bus:coverage
	@docker cp temp-coverage:/app/coverage_filtered.info .tool_result/coverage/coverage.info || true
	@docker cp temp-coverage:/app/coverage_html .tool_result/coverage/ || true
	@docker rm temp-coverage
	@echo "Coverage report extracted to ./.tool_result/coverage/"
	@echo "Open ./.tool_result/coverage/coverage_html/index.html in your browser to view the HTML report"

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

# Run callgrind and write output to ./.tool_result/callgrind on the host
run-callgrind: callgrind-image
	mkdir -p .tool_result/callgrind
	MSYS2_ARG_CONV_EXCL="*" docker run --rm -it \
		-v "$(CURDIR)/.tool_result/callgrind:/app/callgrind" \
		-w /app/callgrind \
		event-bus:callgrind \
		valgrind --tool=callgrind --callgrind-out-file=callgrind.out /app/event-bus $(ARGS)

# Generate .tool_result/callgrind/callgrind.png from .tool_result/callgrind/callgrind.out
callgrind-png: callgrind-viz-image
	MSYS2_ARG_CONV_EXCL="*" docker run --rm -it \
		-v "$(CURDIR)/.tool_result/callgrind:/app/callgrind" \
		-w /app/callgrind \
		event-bus:callgrind-viz \
		sh -lc "gprof2dot -f callgrind callgrind.out | dot -Tpng -o callgrind.png"

# Build an image that has a gprof-instrumented binary (-pg) and gprof tooling
gprof-image:
	docker build -f .devcontainer/Dockerfile --build-arg BUILD_TYPE=RelWithDebInfo --build-arg ENABLE_GPROF=ON --build-arg WITH_GPROF=1 --force-rm -t event-bus:gprof .

# Run the binary to produce .tool_result/gprof/gmon.out, then write .tool_result/gprof/gprof.txt
run-gprof: gprof-image
	mkdir -p .tool_result/gprof
	MSYS2_ARG_CONV_EXCL="*" docker run --rm -it \
		-v "$(CURDIR)/.tool_result/gprof:/app/gprof" \
		-w /app/gprof \
		event-bus:gprof \
		sh -lc "/app/event-bus $(ARGS); gprof /app/event-bus gmon.out > gprof.txt"

# Run Valgrind Memcheck and write .tool_result/memcheck/memcheck.log
run-memcheck: callgrind-image
	mkdir -p .tool_result/memcheck
	MSYS2_ARG_CONV_EXCL="*" docker run --rm -it \
		-v "$(CURDIR)/.tool_result/memcheck:/app/memcheck" \
		-w /app/memcheck \
		event-bus:callgrind \
		valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes \
			--log-file=/app/memcheck/memcheck.log /app/event-bus $(ARGS)

# Run Valgrind Helgrind and write .tool_result/helgrind/helgrind.log
run-helgrind: callgrind-image
	mkdir -p .tool_result/helgrind
	MSYS2_ARG_CONV_EXCL="*" docker run --rm -it \
		-v "$(CURDIR)/.tool_result/helgrind:/app/helgrind" \
		-w /app/helgrind \
		event-bus:callgrind \
		valgrind --tool=helgrind --log-file=/app/helgrind/helgrind.log /app/event-bus $(ARGS)

clean-ddimage:
	docker image prune -f

clean-dall:
	docker system prune -a -f
