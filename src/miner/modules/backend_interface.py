class UnimplementQueryError(Exception):
    def __init__(self, *args, **kwargs):
        super(UnimplementQueryError, self).__init__(*args, **kwargs)


class BackendInterface(object):
    '''An interface for a miner backend.

    It simply defines what methods should be implemented in the real backend
    server.
    '''
    def get_max_id(self, board):
        '''Gets the maximum id of a board.

        Args:
            board: The board name.

        Returns: A integer indicates the maximum document id in the gived board.
        '''
        raise UnimplementQueryError()

    def get_doc_meta_data_after_id(self, board, idid):
        '''Gets an array of document's meta data by given minimum id.

        Args:
            board: The board name.
            idid: The minimum id.

        Returns: An array of `protocol.types.DocMetaData`, which contains all
                the meta data of the documents in the specific board which id
                is larger or equal to the gived minimum id.
        '''
        raise UnimplementQueryError()

    def get_doc_meta_data_after_time(self, board, post_time):
        '''Gets an array of document's meta data by given minimum post time.

        Args:
            board: The board name.
            post_time: The minimum post time.

        Returns: An array of `protocol.types.DocMetaData`, which contains all
                the meta data of the documents posted later than the specific
                post time in the specific board.
        '''
        raise UnimplementQueryError()

    def get_doc_meta_data_of_author(self, board, author):
        '''Gets an array of meta data of the documents posted by an author.

        Args:
            board: The board name.
            author: The author.

        Returns: An array of `protocol.types.DocMetaData`, which contains all
                the meta data of the documents posted by the specific author
                in the specific board.
        '''
        raise UnimplementQueryError()

    def get_doc_meta_data_series(self, board, idid):
        '''Gets an array of meta data of the documents with the same theme.

        Args:
            board: The board name.
            idid: One of the document's id.

        Returns: An array of `protocol.types.DocMetaData`, which contains all
                the meta daa of the documents for the same theme.
        '''
        raise UnimplementQueryError()

    def get_doc_real_data(self, board, idid):
        '''Gets a document's real data.

        Args:
            board: The board name.
            idid: The document's id.

        Returns: An instance of `protocol.types.DocRealData`, which contains
                the document's context and reply messages.
        '''
        raise UnimplementQueryError()

    def register_new_doc_listener(self, board, listener):
        '''A alsome method, but I don't know how to explain now.'''
        raise UnimplementQueryError()

    def unregister_new_doc_listener(self, idid):
        '''A alsome method, but I don't know how to explain now.'''
        raise UnimplementQueryError()

    def register_doc_meta_data_changed_listener(self, board, listener):
        '''A alsome method, but I don't know how to explain now.'''
        raise UnimplementQueryError()

    def unregister_doc_meta_data_changed_listener(self, idid):
        '''A alsome method, but I don't know how to explain now.'''
        raise UnimplementQueryError()

    def get_id_by_url(self, url):
        '''Gets the document's identity by url.

        Args:
            url: Url of the document.

        Returns: An instance of `protocol.types.DocIdentity`
        '''
        raise UnimplementQueryError()

    def get_url_by_id(self, board, idid):
        '''Gets the url of the document in the web by its id.

        Args:
            board: The board name.
            idid: The id of the document.

        Returns: A string of url.
        '''
        raise UnimplementQueryError()
