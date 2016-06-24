import sys


def get_exception_msg(e):
    exc_type, exc_obj, exc_tb = sys.exc_info()
    while exc_tb.tb_next:
        exc_tb = exc_tb.tb_next
    return 'Got an exception: %s:%d >> %r' % (
            exc_tb.tb_frame.f_code.co_filename, exc_tb.tb_lineno, e)
