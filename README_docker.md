## Docker Instructions


```bash
docker build -f ./.devcontainer/Dockerfile --tag=rtmptest:latest .
docker run rtmptest:latest
docker run -it --network="host" rtmptest:latest test.mp4 rtmp://127.0.0.1:1935/live/test
```
