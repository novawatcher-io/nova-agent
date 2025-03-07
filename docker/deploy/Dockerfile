FROM ubuntu:24.04

LABEL githubrepo="https://github.com/LeiZhang-Hunter/trace-agent"

#COPY ./libnvidia-ml.so.1 /lib/x86_64-linux-gnu/libnvidia-ml.so.1
COPY ./start.sh /start.sh
COPY ./config.json /nova-agent/config.json
COPY ./nova-agent /usr/bin/nova-agent

CMD ["/start.sh"]
