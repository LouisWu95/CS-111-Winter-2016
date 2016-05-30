function should_fail() {
    result=$?;

    echo -n "==> $1 ";

    if [ $result -lt 1 ]; then
	echo "FAILURE";
	exit 1;
    else
	echo;
    fi
}

function should_succeed() {
    result=$?;

    echo -n "==> $1 ";

    if [ $result -gt 0 ]; then
	echo "FAILURE";
	exit 1;
    else
	echo;
    fi
}


./simpsh --rdonly cantpossiblyexist 2>&1 | grep "No such file" > /dev/null
should_succeed "reports missing file";


./simpsh --rdonly Makefile | grep "No such file" > /dev/null;
should_fail "does not report file that exists"


./simpsh --verbose --command 1 2 3 echo foo 2>&1 | grep "Bad file descriptor" > /dev/null
should_succeed "using a non existent file descriptor should report the error"


(sort < verbose.h | cat Makefile - | tr A-Z a-z > output1.txt) 2>> error.txt

./simpsh --rdonly verbose.h --pipe --pipe --trunc --wronly output2.txt --append --wronly error.txt --command 3 5 6 tr 'A-Z' 'a-z' --command 0 2 6 sort --command 1 4 6 cat Makefile - --wait

diff output1.txt output2.txt
should_succeed "Test success"
