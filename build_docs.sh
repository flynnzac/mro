#!/bin/sh

cat pushmac.pshm.1 | pushmac > pushmac.1
groff -mandoc -Thtml pushmac.1 > README.md
