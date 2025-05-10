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

declare -r TARGET="@BUILD_TARGET@"
declare -r DISTRO="@DISTRO@"
export V=1

export CFLAGS="-s -DTURKMAN -L@DISTRODIR@ @CFLAGS@"
export CXXFLAGS="-s -DTURKMAN -L@DISTRODIR@ @CXXFLAGS@"
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

