declare -r YMPVER="@VERSION@"
export NOCONFIGURE=1
export NO_COLOR=1
export VERBOSE=1
export FORCE_UNSAFE_CONFIGURE=1
export PYTHONDONTWRITEBYTECODE=1
export SHELL="/bin/bash"
export EDITOR="/bin/false"
export PAGER="/bin/cat"
export CONFIG_SHELL="/bin/bash"
export DESTDIR="@buildpath@/output"

declare -r TARGET="@BUILD_TARGET@"
declare -r DISTRO="@DISTRO@"
export V=1

export CFLAGS="-s -DTURKMEN -L@DISTRODIR@ @CFLAGS@"
export CXXFLAGS="-s -DTURKMEN -L@DISTRODIR@ @CXXFLAGS@"
export CC="@CC@"
export LDFLAGS="@LDFLAGS@"

export ARCH="@ARCH@"
export DEBARCH="@DEBARCH@"

export GOCACHE="@buildpath@/go-cache/"
export GOMODCACHE="@buildpath@/go/"
export GOTMPDIR="@buildpath@"

export LANG="C.UTF-8"

declare -r API_KEY='@APIKEY@'

exec 0< /dev/null
set -o pipefail
shopt -s expand_aliases

function meson(){
    if [[ "$1" == "setup" ]] ; then
        command meson "$@" \
            -Ddefault_library=both \
            -Dwrap_mode=nodownload \
            -Db_lto=true \
            -Db_pie=true
    else
        command meson "$@"
    fi
}

function target(){
    if [[ "$1" == "@BUILD_TARGET@" ]] ; then
        return 0
    fi
    return 1
}
readonly -f target

function buildtype (){
    if [[ "$1" == "$BUILDTYPE" ]] ; then
        return 0
    fi
    return 1
}
readonly -f buildtype

function use(){
    if ! echo ${uses[@]} ${uses_extra[@]} ${arch[@]} all extra | grep "$1" >/dev/null; then
        echo "Use flag \"$1\" is unknown!"
        return 1
    fi
    if [[ "${use_all}" == "31" ]] ; then
        if echo ${uses[@]} | grep "$1" >/dev/null; then
            return 0
        fi
    fi
    if [[ "${use_extra}" == "31" ]] ; then
        if echo ${uses_extra[@]} | grep "$1" >/dev/null; then
            return 0
        fi
    fi
    for use in ${uses[@]} ${uses_extra[@]}; do
        if [[ "${use}" == "$1" ]] ; then
            flag="use_$1"
            [[ "${!flag}" == "31" ]]
            return $?
        fi
    done
}
readonly -f use

function use_opt(){
    if use "$1" ; then
        echo $2
    else
        echo $3
    fi
}
readonly -f use_opt
alias use_arg=use_opt

function eapply(){
    for aa in $* ; do
        patch -Np1 "$aa"
    done
}
readonly -f eapply
