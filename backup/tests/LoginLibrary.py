import os.path
import subprocess
import sys


class LoginLibrary(object):

    def __init__(self):
        self._sut_path = os.path.join(os.path.dirname(__file__), '..', 'sut', 'server.exe')
        self._status = ''
        self._retcode = 0

    def execute_this(self, id):
        self._run_command('', id)

    def status_should_be(self, expected_status):
        if int(expected_status) != self._retcode:
            raise AssertionError("Expected status to be '%s' but was '%s'."
                                 % (expected_status, self._retcode))

    def _run_command(self, command, *args):
        command = [self._sut_path] + list(args)
        process = subprocess.Popen(command, universal_newlines=True, stdout=subprocess.PIPE,
                                   stderr=subprocess.STDOUT)
        self._status = process.communicate()[0].strip()
        self._retcode = process.returncode

    def _run_mycommand(self, *args):
        command = [sys.executable, self._sut_path] + list(args)
        process = subprocess.Popen(command, universal_newlines=True, stdout=subprocess.PIPE,
                                   stderr=subprocess.STDOUT)
        process.wait()
        self._retcode = process.returncode
