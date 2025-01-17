#!/bin/bash

# dump_flag set to just print out all environments
dump_flag=false

# Function to parse options
parse_options() {
    while [[ $# -gt 0 ]]; do
        case $1 in
        -d)
            dump_flag=true
            shift # Remove the current option from the arguments list
            ;;
        *)
            echo "Usage: source ${BASH_SOURCE[0]} [-d]"
            return 1
            ;;
        esac
    done
}

# declare a associative array for env
declare -A INSTALL_ENVS

get_path() {
    local list_name="$1"
    shift

    # Retrieve the current value of the list
    local current_value
    current_value=$(eval "echo \$$list_name")

    # Loop through each value to be checked
    for new_entry in "$@"; do
        case ":$current_value:" in
        *":$new_entry:"*) : ;;        # already exist
        *) current_value="${new_entry}${current_value:+:${current_value}}" ;; # add new entry. work correct even if current_value is empty
        esac
    done

    # Update the list with the new values
    echo "$current_value"
}

set_envs() {
    INSTALL_ENVS["CPATH"]=$(get_path "CPATH" "$INSTALL_PREFIX/include")
    INSTALL_ENVS["LD_LIBRARY_PATH"]=$(get_path "LD_LIBRARY_PATH" "$INSTALL_PREFIX/lib")
    INSTALL_ENVS["PATH"]=$(get_path "PATH" "$INSTALL_PREFIX/bin")
    INSTALL_ENVS["PKG_CONFIG_PATH"]=$(get_path "PKG_CONFIG_PATH" "$INSTALL_PREFIX/lib/pkgconfig" "$INSTALL_PREFIX/lib64/pkgconfig" "$INSTALL_PREFIX/share/pkgconfig")
    INSTALL_ENVS["THIRD_PARTY_LINK_DIR"]=$(get_path "THIRD_PARTY_LINK_DIR" "$INSTALL_PREFIX/lib" "$INSTALL_PREFIX/lib64" | tr ':' ';')
    INSTALL_ENVS["THIRD_PARTY_INCLUDE_DIR"]=$(get_path "THIRD_PARTY_INCLUDE_DIR" "$INSTALL_PREFIX/include" | tr ':' ';')
    INSTALL_ENVS["THIRD_PARTY_INATALL_DIR"]="$INSTALL_PREFIX"
}

main() {
    parse_options "$@"

    if [[ $? -ne 0 ]]; then
        echo -e "\e[31msetup_env.sh failed!.\e[0m"
        return 1
    fi

    # Check if INSTALL_PREFIX environment variable exists and not empty
    if [ -z "${INSTALL_PREFIX+x}" ] || [ -z "$INSTALL_PREFIX" ]; then
        # Get the directory where this script is located
        SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

        # check if PROJECT_ROOT is end up with 'trace-agent'
        if [[ $SCRIPT_DIR != *"/trace-agent" ]]; then
            echo -e "\e[31mThis script should be located in the trace-agent directory.\e[0m"
        fi
        export INSTALL_PREFIX="$SCRIPT_DIR/.build/installed"
    fi

    set_envs

    # if dump flag set, just dump all the environment variables
    if [[ $dump_flag == true ]]; then
        for key in "${!INSTALL_ENVS[@]}"; do
            echo "$key=${INSTALL_ENVS[$key]}"
        done
    else
        for key in "${!INSTALL_ENVS[@]}"; do
            export $key=${INSTALL_ENVS[$key]}
        done
    fi
}

main "$@"
