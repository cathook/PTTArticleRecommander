import sys
import datetime
import time


def get_exception_msg(e):
    exc_type, exc_obj, exc_tb = sys.exc_info()
    while exc_tb.tb_next:
        exc_tb = exc_tb.tb_next
    return 'Got an exception: %s:%d >> %r' % (
            exc_tb.tb_frame.f_code.co_filename, exc_tb.tb_lineno, e)


def sleep_if(total_time, func, delta=1):
    t0 = datetime.datetime.now().timestamp()
    while datetime.datetime.now().timestamp() - t0 < total_time:
        if not func():
            return False
        time.sleep(delta)
    return True


def UnimplementedMethodCalled(Exception):
    pass
