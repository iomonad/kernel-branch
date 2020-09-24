#!/bin/sh
#
# (c) iomonad <iomonad@riseup.net>
# https://github.com/iomonad/kernel-branch
#

set -o errexit
set -o pipefail

function success () {
    echo -e "\e[92mOK"
    tput sgr0;
}

function failure () {
    echo -e "\e[91mERR"
    tput sgr0;
    echo "[!!] $1"
}

function ensure_running_vm () {
    printf '%s' "$FUNCNAME: "
    if [ -z "$(grep hypervisor /proc/cpuinfo)"]; then
	failure "Not running in VM."
    fi
    success
}

function check_kernel_version () {
    printf '%s' "$FUNCNAME: "
    if [ "$(uname -r | head -c 1)" -le 4 ]; then
	failure "Kernel used is outdated !"
    fi
    success
}

function check_kernel_sources () {
    printf '%s' "$FUNCNAME: "
    if [ ! -d "/usr/src/kernel-$(uname -r)" ]; then
	failure "No kernel sources found"
    fi
    success
}

function check_correct_partition_number () {
    printf '%s' "$FUNCNAME: "
    if [ "$(lsblk | grep "sda[0-9]" | wc -l)" -le 2 ]; then
	failure "No enought partion required! $(lsblk)"
    fi
    success
}

function ensure_udev_installed () {
    printf '%s' "$FUNCNAME: "
    if [ -z "$(pgrep udev)" ]; then
	failure "Udev not running !"
    fi
    success
}

function check_kernel_version_string () {
    local username=ctrouill
    printf '%s' "$FUNCNAME: "
    if [ "$(uname -r | cut -d '-' -f 2)" != "$username" ]; then
	failure "Wrong username"
    fi
    success
}

function ensure_hostname_correct_value () {
    printf '%s' "$FUNCNAME: "
    if [ "$(hostname)" != "$username" ]; then
	failure "Wrong username"
    fi
    success
}

function ensure_init_manager_installed () {
    printf '%s' "$FUNCNAME: "
    if [ ! -z "$(init --version | grep SysV)" ]; then
	success
	return;
    fi
    if [ ! -z "$(init --version | grep systemd)" ]; then
	success
	return;
    fi
    failure "No INIT found"
}

function ensure_vmlinuz_correct () {
    # TODO: Impl needed
    printf '%s' "$FUNCNAME: "
    success
}

function ensure_packages_present () {
    # TODO: Impl needed
    printf '%s' "$FUNCNAME: "
    success
}

function main () {
    ensure_running_vm
    check_kernel_version
    check_kernel_sources
    check_correct_partition_number
    ensure_udev_installed
    check_kernel_version_string
    ensure_hostname_correct_value
    ensure_init_manager_installed
    ensure_vmlinuz_correct
    ensure_packages_present
}

main
