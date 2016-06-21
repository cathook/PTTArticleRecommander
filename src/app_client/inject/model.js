/**
 * The model part of the MVC structure.
 *
 * When the controller want the model to update the recommand document list by
 * current url, it will connect to the app server to fetch data, after finished,
 * it will emit `common.EVENTS.UPDATE` event.
 */
(function(exports) {
  var ADocument = function(url, title, content) {
    this.url = url;
    this.title = title;
    this.content = content;
  };


  var Model = function() {
    eventObject.EventObject.call(this);

    this._updateCounter = 0;
    this._parser = new window.DOMParser();

    this._clean();
  };

  Model.prototype = Object.setPrototypeOf({
    update: function(url) {
      this._updateCounter += 1;
      var idx = this._updateCounter;

      this._clean();

      var request = new XMLHttpRequest();

      request.open('POST', common.SERVER_URL, true);
      request.setRequestHeader('Content-type', 'application/json');
      request.send(JSON.stringify({'url': url}));

      request.onreadystatechange = (function(evt) {
        if (request.readyState == 4 && request.status == 200) {
          var response = JSON.parse(request.responseText);
          ['positive', 'negative', 'neutral'].forEach((function(name) {
            response[name].forEach((function(url) {
              this._fetchDocument.call(
                  this, idx, this[name + '_docs'], url);
            }).bind(this));
          }).bind(this));
        }
      }).bind(this);
    },

    _clean: function() {
      this.positive_docs = [];
      this.negative_docs = [];
      this.neutral_docs = [];
    },

    _fetchDocument: function(idx, arr, url) {
      var request = new XMLHttpRequest();

      request.open('GET', url, true);
      request.send();

      request.onreadystatechange = (function(evt) {
        if (idx != this._updateCounter) {
          return;
        }
        if (request.readyState == 4 && request.status == 200) {
          var dom = this._parser.parseFromString(request.responseText,
                                                 'text/html');
          var title = dom.head.getElementsByTagName('title')[0].innerHTML;
          // TODO(cathook): Fetch the document content and get its brief
          //     preview text.
          arr.push(new ADocument(url, title, ''));
          this.emit(common.EVENTS.UPDATED, idx);
        }
      }).bind(this);
    }
  }, eventObject.EventObject.prototype);


  exports.ADocument = ADocument;

  exports.Model = Model;
})(window.model = {});
