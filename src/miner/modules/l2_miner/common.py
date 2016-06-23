class ADocument(object):
    def __init__(self):
        self.url = None
        self.idid = None
        self.meta_data = None
        self.real_data = None


HEADERS = {
    'Cookie': str('over18=1; __utma=156441338.1052450315.1398943535.1398943535.1398943535.1; __utmb=156441338.2.10.1398943535; __utmc=156441338; __utmz=156441338.1398943535.1.1.utmcsr=(direct)|utmccn=(direct)|utmcmd=(none)')
}


SERVER_ADDR = 'https://www.ptt.cc'
URL_PREFIX = SERVER_ADDR + '/bbs/'
