/**
 * An simple event object which supplies register event handler and emit event.
 */
(function(exports) {
  var EventObject = function() {
    this._eventObjectCallbacks = {};
  };

  EventObject.prototype = {
    on: function(eventName, eventListener, thisObj) {
      if (!this._eventObjectCallbacks.hasOwnProperty(eventName)) {
        this._eventObjectCallbacks[eventName] = [];
      }
      this._eventObjectCallbacks[eventName].push(eventListener);
    },

    off: function(eventName, eventListener) {
      var idx = this._eventObjectCallbacks[eventName].indexOf(eventListener);
      this._eventObjectCallbacks[eventName].splice(idx, 1);
    },

    emit: function(eventName) {
      var args = [], i;
      for (i = 1; i < arguments.length; ++i) {
        args.push(arguments[i]);
      }
      this._eventObjectCallbacks[eventName].forEach(function(callback) {
        callback.apply(window, args);
      });
    }
  };


  exports.EventObject = EventObject;
})(window.eventObject = {});
