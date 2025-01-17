#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 ubuntu-22.04 | ubuntu-24.04 | incremental"
    exit 1
fi

if [ "$1" != "ubuntu-22.04" ] && [ "$1" != "ubuntu-24.04" ] && [ "$1" != "incremental" ]; then
    echo "Usage: $0 ubuntu-22.04 | ubuntu-24.04 | incremental"
    exit 1
fi
base=$1

# get location of current script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_DIR="$(cd "$SCRIPT_DIR"/../../ && pwd)"

cp $base/Dockerfile $PROJECT_DIR/third_party
cp $PROJECT_DIR/build_third_party.sh $PROJECT_DIR/third_party
cp $PROJECT_DIR/setup_env.sh $PROJECT_DIR/third_party
cd $PROJECT_DIR/third_party

docker build . -t  ${BUILD_TAG:-trace-agent-builder/${base}:$(date +%Y%m%d)}
rm Dockerfile build_third_party.sh setup_env.sh
