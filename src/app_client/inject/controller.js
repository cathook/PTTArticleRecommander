/**
 * Controller part of the MVC structure.
 */
(function(exports) {
  var Controller = function(model) {
    this._model = model;

    this._model.update(window.location.href);
  };

  Controller.prototype = {
  };


  exports.Controller = Controller;
})(window.controller = {});
