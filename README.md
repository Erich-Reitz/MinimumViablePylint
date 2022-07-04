# Minimum Viable Pylint
Python script to move pylint errors to `disable=` section of `.pylintrc` file. Useful when incrementally updating code to conform to linter.
<pre>
usage: main.py [-h] --command COMMAND [COMMAND ...] [--file FILE]

optional arguments:
  -h, --help            show this help message and exit
  --command COMMAND [COMMAND ...], -c COMMAND [COMMAND ...]
                        command to run
  --file FILE, -f FILE  configuration file
  </pre>

