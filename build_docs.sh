#!/bin/sh

cat pushmac.pushmac.1 | pushmac > pushmac.1
groff -mandoc -Thtml pushmac.1 > README.md
