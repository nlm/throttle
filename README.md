throttle
========

Easily throttle command spawing in scripts

usage
-----

    throttle <times> <period> <statefile> [<command> [args]]

throttle will execute a command at most TIMES times in a period of PERIOD seconds
and store its state in STATEFILE.

return values
-------------

    <100: actual return value of the program
    100: command throttled
    101: syntax error
    102: system error
    103: error reading/writing state file

