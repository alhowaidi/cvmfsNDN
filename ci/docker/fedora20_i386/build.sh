#!/bin/bash

set -e

SCRIPT_LOCATION=$(cd "$(dirname "$0")"; pwd)
. ${SCRIPT_LOCATION}/../common.sh

SYSTEM_NAME="fedora20"
BASE_ARCH="i386"

yum_repo="$(get_yum_repo_mirror "https://mirrors.fedoraproject.org/mirrorlist?repo=fedora-20&arch=$BASE_ARCH")"
REPO_BASE_URL="$yum_repo"
GPG_KEY_PATHS="file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora-20"
BASE_PACKAGES="fedora-release coreutils tar iputils rpm yum"

. ${SCRIPT_LOCATION}/../rhel_common/build.sh
