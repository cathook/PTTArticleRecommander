window.addEventListener('load', function() {
  var m = new model.Model();
  var c = new controller.Controller(m);
  var v = new view.View(m, c);

  window.pttrObjs = [m, v, c];
});
