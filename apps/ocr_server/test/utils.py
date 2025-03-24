import os
import logging


class TestLogger:
    def __init__(self):
        self.logger = logging.Logger("test_logger")
        self.logger.setLevel(logging.DEBUG)
        console_handler = logging.StreamHandler()
        console_handler.setLevel(logging.DEBUG)
        formatter = logging.Formatter('%(asctime)s | %(levelname)s : %(message)s')
        console_handler.setFormatter(formatter)
        self.logger.addHandler(console_handler)
        self.hr_line = "-" * 80

    def sub_title(self, title):
        self._out(f"<<< {title} >>>", 2)

    def hr(self, message):
        self._out(self.hr_line, 2)
        msg_len = len(message)
        left_space_len = (80 - 2 - msg_len) / 2
        right_space_len = 80 - msg_len - left_space_len - 2
        out = "|" + " " * int(left_space_len) + message + " " * int(right_space_len) + "|"
        self._out(out, 2)
        self._out(self.hr_line, 2)

    def debug(self, msg):
        self._out(msg, 1)

    def info(self, msg):
        self._out(msg, 2)

    def warning(self, msg):
        self._out(msg, 3)

    def error(self, msg):
        self._out(msg, 4)

    def critical(self, msg):
        self._out(msg, 5)

    def _out(self, msg, level=1):
        if level == 1:
            self.logger.debug(msg)
        elif level == 2:
            self.logger.info(msg)
        elif level == 3:
            self.logger.warning(msg)
        elif level == 4:
            self.logger.error(msg)
        elif level == 5:
            self.logger.critical(msg)


def count_files(directory):
    return len([f for f in os.listdir(directory) if os.path.isfile(os.path.join(directory, f))])


logger = TestLogger()
