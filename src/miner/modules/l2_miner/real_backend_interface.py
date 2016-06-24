class RealBackendInterface(object):
    @property
    def max_id(self):
        pass

    def yield_doc_meta_data_reversed_from_id(self, idid):
        pass

    def get_doc_real_data(self, idid):
        pass

    def get_id_by_url(self, idid):
        pass

    def get_url_by_id(self, url):
        pass
