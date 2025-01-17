FROM aronic/cppdev:latest

USER root

COPY . /tmp/third_party

# Waring: we have openssl in the third party package, if we install openssl to `/usr` that will break everything!
RUN cd /tmp && INSTALL_PREFIX=/opt/trace-agent /tmp/third_party/build_third_party.sh

RUN rm -rf /tmp/third_party && rm -rf /tmp/third_party_build

# Switch back to dialog for any ad-hoc use of apt-get
ENV DEBIAN_FRONTEND=dialog

# switch to user `devops`
USER devops
