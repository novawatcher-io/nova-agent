FROM ubuntu:24.04

LABEL githubrepo="https://github.com/LeiZhang-Hunter/trace-agent"

#COPY ./libnvidia-ml.so.1 /lib/x86_64-linux-gnu/libnvidia-ml.so.1
COPY ./start.sh /start.sh
COPY ./config.json /trace-agent/config.json
COPY ./trace-agent /usr/bin/trace-agent

CMD ["/start.sh"]
