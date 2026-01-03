release:
	docker build -f .devcontainer/Dockerfile --build-arg BUILD_TYPE=Release --force-rm -t event-bus:release .

debug:
	docker build -f .devcontainer/Dockerfile --build-arg BUILD_TYPE=debug --force-rm -t event-bus:debug .

# usage make run-release ARGS="SET mykey myval"
run-release: release
	docker run --rm -it event-bus:release ./event-bus $(ARGS)

# what is the bash is used for?
run-debug: debug
	docker run --rm -it event-bus:debug bash

clean-ddimage:
	docker image prune -f

clean-dall:
	docker system prune -a -f