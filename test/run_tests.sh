#!/bin/bash

let FAILURE=0

echo " STARTING TESTS"
echo "--------------------"
echo
for t in $*; do
    valgrind --quiet --error-exitcode=1 \
        --leak-check=full --show-leak-kinds=all ./test/${t}_test
    let FAILURE+=$?
    echo
done

echo "--------------------"
echo " FINISHED TESTS"

if [ "$FAILURE" != "0" ]; then
    echo -e "Result: \e[31m FAILED: $FAILURE\e[0m"
else
    echo -e "Result: \e[32m PASSED\e[0m"
fi

exit $FAILURE

