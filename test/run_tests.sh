#!/bin/bash

let FAILURE=0

echo " STARTING TESTS"
echo "--------------------"
echo
echo "Valgrind version: $(valgrind --version)"
echo
for t in $*; do
    valgrind --quiet --error-exitcode=1 \
        --leak-check=full --show-possibly-lost=yes --show-reachable=yes ./test/bin/${t}_test
    let FAILURE+=$?
    echo
done

echo "--------------------"
echo " FINISHED TESTS"

if [ "$FAILURE" != "0" ]; then
    echo -e "Result:\e[31m FAILED: $FAILURE\e[0m"
else
    echo -e "Result:\e[32m PASSED\e[0m"
fi

exit $FAILURE

