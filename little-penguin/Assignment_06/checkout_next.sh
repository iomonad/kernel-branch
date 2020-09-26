#!/bin/sh

# Clone source
git clone https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git
cd linux

# Add feature candidate branches
# in remote (linux-next)
git remote add linux-next https://git.kernel.org/pub/scm/linux/kernel/git/next/linux-next.git

# Fetch latests tags
git fetch linux-next
git fetch --tags linux-next

# Tracking
git remote update
git tag -l "next-*" | tail

git checkout -b random_derived_branch $(git tag -l "next-*" | tail -n1)
