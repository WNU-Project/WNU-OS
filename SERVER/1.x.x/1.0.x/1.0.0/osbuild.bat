cd %~dp0
docker run -it --rm --privileged -v "%cd%":/root/env wnu-os-server