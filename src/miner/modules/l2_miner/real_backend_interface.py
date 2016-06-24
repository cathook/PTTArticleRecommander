from modules.utils import UnimplementedMethodCalled


class RealBackendInterface(object):
    @property
    def max_id(self):
        raise UnimplementedMethodCalled()

    @property
    def updated(self):
        raise UnimplementedMethodCalled()

    def stop_auto_update(self):
        raise UnimplementedMethodCalled()

    def yield_doc_meta_data_reversed_from_id(self, idid):
        raise UnimplementedMethodCalled()

    def get_doc_real_data(self, idid):
        raise UnimplementedMethodCalled()

    def get_id_by_url(self, idid):
        raise UnimplementedMethodCalled()

    def get_url_by_id(self, url):
        raise UnimplementedMethodCalled()
