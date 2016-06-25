/**
 * View part of the MVC structure.
 */
(function(exports) {
  var View = function(model, control) {
    this._model = model;
    this._control = control;
    this._posListElement = null;
    this._negListElement = null;
    this._neuListElement = null;
    this._prevUpdateIdx = null;

    this._initDOM();
  };

  View.prototype = {
    _initDOM: function() {
      var request = new XMLHttpRequest();

      request.open('GET', chrome.extension.getURL('/inject/view.html'), true);

      request.send();

      request.onreadystatechange = (function(evt) {
        if (request.readyState == 4 && request.status == 200) {
          var parser = new window.DOMParser();
          var dom = parser.parseFromString(request.responseText, 'text/html');

          while (dom.head.firstElementChild !== null) {
            document.head.appendChild(dom.head.firstElementChild);
          }

          while (dom.body.firstElementChild !== null) {
            var element = dom.body.firstElementChild;

            var placeSelector = element.getAttribute('data-parent-selector');
            if (placeSelector === null || placeSelector === '') {
              placeSelector = 'body';
            }

            p = document.querySelector(placeSelector);

            nextSelector = element.getAttribute('data-next-node-selector');
            if (nextSelector === null || nextSelector === '') {
              p.appendChild(element);
            } else {
              p.insertBefore(element, p.querySelector(nextSelector));
            }
          }

          this._posListElement = document.getElementById('pttr-positive-list');
          this._negListElement = document.getElementById('pttr-negative-list');
          this._neuListElement = document.getElementById('pttr-neutral-list');

          this._setupEventHandlers();
          this._setupEventListeners();
        }
      }).bind(this);
    },

    _setupEventHandlers: function() {
      document.addEventListener('click', function() {
        var list = document.getElementById('pttr-list');
        list.classList.remove('pttr-list-show');
        list.classList.add('pttr-list-hide');
      });

      document.getElementById('pttr-list').addEventListener(
          'click', function(evt) { evt.stopPropagation(); });

      document.getElementById('pttr-show-list').addEventListener(
          'click', function(evt) {
            var list = document.getElementById('pttr-list');
            list.classList.remove('pttr-list-hide');
            list.classList.add('pttr-list-show');
            evt.stopPropagation();
          });
    },

    _setupEventListeners: function() {
      this._model.on(common.EVENTS.UPDATED, this._update.bind(this));
    },

    _update: function(idx) {
      if (idx != this._prevUpdateIdx) {
        this._posListElement.innerHTML = '';
        this._negListElement.innerHTML = '';
        this._neuListElement.innerHTML = '';
        this._prevUpdateIdx = idx;
      }
      ['positive', 'negative', 'neutral'].forEach((function(name) {
        var listElement = this['_' + name.substr(0, 3) + 'ListElement'];
        var docs = this._model[name + '_docs'];
        for (var i = listElement.children.length; i < docs.length; ++i) {
          listElement.appendChild(this._createDocLink(docs[i]));
        }
      }).bind(this));
    },

    _createDocLink: function(doc) {
      var a = document.createElement('a');
      a.classList.add('pttr-list-item');
      a.setAttribute('href', doc.url);
      a.innerHTML = doc.title;
      return a;
    }
  };


  exports.View = View;
})(window.view = {});
