import struct

from modules.protocol import net


# Enumerates of the reply mode.
class ReplyMode(object):
    GOOD = 0
    NORMAL = 1
    WOO = 2


NUM_REPLY_MODES = 3


class ReplyMessage(object):
    '''A structure for a reply message.

    Attributes:
        mode: The reply mode.
        user: The user who replied this message.
        message: The message.
    '''
    def __init__(self, mode, user, message):
        self.mode = mode
        self.user = user
        self.message = message

    def dump(self):
        return b''.join([net.ReplyModeStruct.pack(self.mode),
                         net.StringStruct.pack(self.user),
                         net.StringStruct.pack(self.message)])


class DocMetaData(object):
    '''A structure for a document's meta data.

    Attributes:
        idid: Identity of this document.
        prev_idid: Identity of the document this document re for.
        title: Title of this document.
        author: Author of this document.
        post_time: Post time of this document.
        board: The board this document in.
        num_reply_rows: An array of number of different type of reply message.
    '''
    def __init__(self,
                 idid, prev_id,
                 title, author, post_time, board,
                 num_reply_rows):
        assert len(num_reply_rows) == NUM_REPLY_MODES
        self.idid = idid
        self.prev_id = prev_id
        self.title = title
        self.author = author
        self.post_time = post_time
        self.board = board
        self.num_reply_rows = [a for a in num_reply_rows]

    def dump(self):
        f = lambda a: struct.Struct('<L').pack(a)
        return b''.join([net.IdentityStruct.pack(self.idid),
                         net.IdentityStruct.pack(self.prev_id),
                         net.StringStruct.pack(self.title),
                         net.StringStruct.pack(self.author),
                         net.TimeStruct.pack(self.post_time),
                         net.StringStruct.pack(self.board)] +
                        [f(a) for a in self.num_reply_rows])


class DocIdentity(object):
    '''Structure for a identity of a document.

    Attributes:
        board: Board which this document in.
        idid: Identity of the document.
    '''
    def __init__(self, board, idid):
        self.board = board
        self.idid = idid

    def dump(self):
        x = net.StringStruct.pack(self.board)
        y = net.IdentityStruct.pack(self.idid)
        return  x + y

    @staticmethod
    def load(buf, offs=0):
        (board, offs) = net.StringStruct.unpack(buf, offs)
        (idid, offs) = net.IdentityStruct.unpack(buf, offs)
        return (DocIdentity(board, idid), offs)


class DocRealData(object):
    '''Structures for a document's real data.

    Attributes:
        content: Content of the document.
        reply_message: An array of reply message.
    '''
    def __init__(self, content, reply_messages):
        self.content = content
        self.reply_messages = [r for r in reply_messages]

    def dump(self):
        return (net.StringStruct.pack(self.content) +
                net.ArrayStruct.pack(self.reply_messages, net.CustomStruct))
