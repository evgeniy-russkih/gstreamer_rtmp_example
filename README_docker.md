## Docker Instructions

If you have [Docker](https://www.docker.com/) installed, you can run this
in your terminal, when the Dockerfile is inside the .devconatiner directory:

```bash
docker build -f ./.devcontainer/Dockerfile --tag=rtmptest:latest .
docker run -it \
	-v absolute_path_on_host_machine:absolute_path_in_guest_container \
	rtmptest:latest
```

You can configure and build [as directed above](#build) using these commands:

```bash
/starter_project# mkdir build
/starter_project# cmake -S . -B ./build
/starter_project# cmake --build ./build
```

